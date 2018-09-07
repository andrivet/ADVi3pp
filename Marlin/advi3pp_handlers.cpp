/**
 * Marlin 3D Printer Firmware For Wanhao Duplicator i3 Plus (ADVi3++)
 *
 * Copyright (C) 2017 Sebastien Andrivet [https://github.com/andrivet/]
 *
 * Copyright (C) 2016 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Based on Sprinter and grbl.
 * Copyright (C) 2011 Camiel Gubbels / Erik van der Zalm
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include "configuration_store.h"
#include "temperature.h"
#include "cardreader.h"
#include "planner.h"
#include "parser.h"
#include "printcounter.h"
#include "duration_t.h"

#include "advi3pp.h"
#include "advi3pp_log.h"
#include "advi3pp_dgus.h"
#include "advi3pp_stack.h"
#include "advi3pp_.h"

#include <HardwareSerial.h>

// Used directly by Marlin
uint8_t progress_bar_percent;
int16_t lcd_contrast;

namespace
{
    const uint16_t advi3_pp_version = 0x400;
    const uint16_t advi3_pp_oldest_lcd_compatible_version = 0x400;
    const uint16_t advi3_pp_newest_lcd_compatible_version = 0x400;

    const unsigned long advi3_pp_baudrate = 115200; // Between the LCD panel and the mainboard
    const uint16_t nb_visible_sd_files = 5;
	const uint8_t  nb_visible_sd_file_chars = 48;
    const uint16_t tuning_extruder_filament = 100; // 10 cm
	const uint16_t tuning_extruder_delta = 20; // 2 cm

    const advi3pp::Feature DEFAULT_FEATURES =
        advi3pp::Feature::ThermalProtection |
        advi3pp::Feature::HeadParking |
        advi3pp::Feature::Dimming |
        advi3pp::Feature::Buzzer;

    const uint32_t DEFAULT_USB_BAUDRATE = BAUDRATE;

    const uint32_t usb_baudrates[] = {9600, 19200, 38400, 57600, 115200, 230400, 250000};

    const int8_t BRIGHTNESS_MIN = 0x01;
    const int8_t BRIGHTNESS_MAX = 0x40;
    const uint8_t DIMMING_RATIO = 25; // in percent
    const uint16_t DIMMING_DELAY = 1 * 60;

    const uint16_t DEFAULT_PREHEAT_PRESET[5][3] = {
        {180, 50, 0},
        {200, 60, 0},
        {220, 70, 0},
        {180, 00, 0},
        {200, 00, 0}
    };

    const uint8_t BUZZ_ON_PRESS_DURATION = 10; // x 1 ms

    //! Transform a value from a scale to another one.
    //! @param value        Value to be transformed
    //! @param valueScale   Current scale of the value (maximal)
    //! @param targetScale  Target scale
    //! @return             The scaled value
    int16_t scale(int16_t value, int16_t valueScale, int16_t targetScale) { return value * targetScale / valueScale; }

    template <typename T, size_t N>
    constexpr size_t countof(T const (&)[N]) noexcept { return N; }
}

#ifdef ADVi3PP_BLTOUCH
bool set_probe_deployed(bool);
float run_z_probe();
extern float zprobe_zoffset;
#endif

namespace advi3pp {

inline namespace singletons
{
    extern ADVi3pp_ advi3pp;
    extern Pages pages;
    extern Task task;
    extern Feature features;
    extern Dimming dimming;
    extern Graphs graphs;

    Wait wait;
    LoadUnload load_unload;
    Preheat preheat;
    Move move;
    SdCard sd_card;
    FactoryReset factory_reset;
    ManualLeveling manual_leveling;
    ExtruderTuning extruder_tuning;
    PidTuning pid_tuning;
    SensorSettings sensor_settings;
    FirmwareSettings firmware_settings;
    NoSensor no_sensor;
    LcdSettings lcd_settings;
    Statistics statistics;
    Versions versions;
    PrintSettings print_settings;
    PidSettings pid_settings;
    StepSettings steps_settings;
    FeedrateSettings feedrates_settings;
    AccelerationSettings accelerations_settings;
    JerkSettings jerks_settings;
    Copyrights copyrights;
    SensorTuning sensor_tuning;
    SensorGrid sensor_grid;
    SensorZHeight sensor_z_height;
    ChangeFilament change_filament;
    EepromMismatch eeprom_mismatch;
    Sponsors sponsors;
    LinearAdvanceTuning linear_advance_tuning;
    LinearAdvanceSettings linear_advance_settings;
    Diagnosis diagnosis;
    SdPrint sd_print;
    UsbPrint usb_print;
    AdvancedPause pause;
};


// --------------------------------------------------------------------
// Pages management
// --------------------------------------------------------------------

//! Show the given page on the LCD screen
//! @param [in] page The page to be displayed on the LCD screen
void Pages::show_page(Page page, bool save_back)
{
    Log::log() << F("Show page ") << static_cast<uint8_t>(page) << Log::endl();

    if(save_back)
        back_pages_.push(get_current_page());

    WriteRegisterDataRequest frame{Register::PictureID};
    frame << 00_u8 << page;
    frame.send(true);
}

//! Retrieve the current page on the LCD screen
Page Pages::get_current_page()
{
    ReadRegister frame{Register::PictureID, 2};
    if(!frame.send_and_receive())
    {
        Log::error() << F("Reading PictureID") << Log::endl();
        return Page::None;
    }

    Uint16 page; frame >> page;
    Log::log() << F("Current page index = ") << page.word << Log::endl();
    return static_cast<Page>(page.word);
}

//! Set page to display after the completion of an operation.
void Pages::save_forward_page()
{
    forward_page_ = get_current_page();
}

//! Show the "Back" page on the LCD display.
void Pages::show_back_page()
{
    if(back_pages_.is_empty())
    {
        Log::error() << F("No Back page defined" )<< Log::endl();
        return;
    }

    show_page(back_pages_.pop(), false);
}

//! Show the "Next" page on the LCD display.
void Pages::show_forward_page()
{
    if(forward_page_ == Page::None)
    {
        Log::error() << F("No Forward page defined") << Log::endl();
        return;
    }

    if(!back_pages_.contains(forward_page_))
    {
        show_back_page();
        return;
    }

    while(!back_pages_.is_empty())
    {
        Page page = back_pages_.pop();
        if(page == forward_page_)
        {
            forward_page_ = Page::None;
            show_page(page, false);
            return;
        }
    }
}

// --------------------------------------------------------------------
// Handler
// --------------------------------------------------------------------

void Handler::handle(KeyValue value)
{
    if(!dispatch(value))
        invalid(value);
}

bool Handler::dispatch(KeyValue value)
{
    switch(value)
    {
        case KeyValue::Show: show(); break;
        case KeyValue::Save: save(); break;
        case KeyValue::Back: back(); break;
        default:                    return false;
    }

    return true;
}

void Handler::invalid(KeyValue value)
{
    Log::error()
            << F("Invalid key value ")
            << static_cast<uint16_t>(value) << Log::endl();
}

void Handler::show(bool save_forward, bool save_back)
{
    do_backup();
    if(save_forward)
        pages.save_forward_page();

    Page page = do_show();
    if(page != Page::None)
        pages.show_page(page, save_back);
}

void Handler::do_backup()
{
}

void Handler::do_rollback()
{
}

Page Handler::do_show()
{
    return Page::None;
}

void Handler::save()
{
    do_save();
    enqueue_and_echo_commands_P(PSTR("M500"));
    pages.show_forward_page();
}

void Handler::do_save()
{
}

void Handler::back()
{
    do_rollback();
    pages.show_back_page();
}

void Handler::store_eeprom_data(EepromWrite& eeprom)
{
    // Nothing to do
}

void Handler::restore_eeprom_data(EepromRead& eeprom)
{
    // Nothing to do
}

void Handler::reset_eeprom_data()
{
    // Nothing to do
}

uint16_t Handler::size_of_eeprom_data() const
{
    return 0;
}

// --------------------------------------------------------------------
// Wait
// --------------------------------------------------------------------

void Wait::show(const __FlashStringHelper* message, bool save_back)
{
    advi3pp.set_status_PGM(reinterpret_cast<const char*>(message));
    back_ = nullptr;
    continue_ = nullptr;
    pages.show_page(Page::Waiting, save_back);
}

void Wait::show(const __FlashStringHelper* message, WaitCalllback back, bool save_back)
{
    advi3pp.set_status_PGM(reinterpret_cast<const char*>(message));
    back_ = back;
    continue_ = nullptr;
    pages.show_page(Page::WaitBack, save_back);
}

void Wait::show(const __FlashStringHelper* message, WaitCalllback back, WaitCalllback cont, bool save_back)
{
    advi3pp.set_status_PGM(reinterpret_cast<const char*>(message));
    back_ = back;
    continue_ = cont;
    pages.show_page(Page::WaitBackContinue, save_back);
}

void Wait::show_continue(const __FlashStringHelper* message, WaitCalllback cont, bool save_back)
{
    advi3pp.set_status_PGM(reinterpret_cast<const char*>(message));
    back_ = nullptr;
    continue_ = cont;
    pages.show_page(Page::WaitContinue, save_back);
}

void Wait::do_rollback()
{
    if(!back_)
    {
        Log::error() << F("No Back action defined") << Log::endl();
        return;
    }

    back_();
    back_ = nullptr;
}

void Wait::do_save()
{
    if(!continue_)
    {
        Log::error() << F("No Continue action defined") << Log::endl();
        return;
    }

    continue_();
    continue_ = nullptr;
}

// --------------------------------------------------------------------
// SD Card
// --------------------------------------------------------------------

bool SdCard::dispatch(KeyValue key_value)
{
    if(Handler::dispatch(key_value))
        return true;

	switch(key_value)
	{
		case KeyValue::SDUp:	up(); break;
		case KeyValue::SDDown:	down(); break;
		case KeyValue::SDLine1:
		case KeyValue::SDLine2:
		case KeyValue::SDLine3:
		case KeyValue::SDLine4:
		case KeyValue::SDLine5:	select_file(static_cast<uint16_t>(key_value) - 1); break;
		default:                return false;
	}

	return true;
}

Page SdCard::do_show()
{
    return Page::SdCard;
}

void SdCard::show_first_page()
{
	if(!card.cardOK)
		return;

	nb_files_ = card.getnrfilenames();
	last_file_index_ = nb_files_ - 1;

    show_current_page();
}

void SdCard::down()
{
	if(!card.cardOK)
		return;

	if(last_file_index_ >= nb_visible_sd_files)
		last_file_index_ -= nb_visible_sd_files;

    show_current_page();
}

void SdCard::up()
{
	if(!card.cardOK)
		return;

	if(last_file_index_ + nb_visible_sd_files < nb_files_)
		last_file_index_ += nb_visible_sd_files;

    show_current_page();
}

//! Show the list of files on SD.
void SdCard::show_current_page()
{
    WriteRamDataRequest frame{Variable::FileName1};

    String name;
    for(uint8_t index = 0; index < nb_visible_sd_files; ++index)
    {
        get_file_name(index, name);
        frame << FixedSizeString(name, nb_visible_sd_file_chars); // Important to truncate, there is only limited space
    }
    frame.send(true);
}

//! Get a filename with a given index.
//! @param index    Index of the filename
//! @param name     Copy the filename into this Chars
void SdCard::get_file_name(uint8_t index_in_page, String& name)
{
    name = "";
	if(last_file_index_ >= index_in_page)
	{
		card.getfilename(last_file_index_ - index_in_page);
        if(card.filenameIsDir) name = "[";
		name += (card.longFilename[0] == 0) ? card.filename : card.longFilename;
		if(card.filenameIsDir) name += "]";
	}
};

//! Select a filename as sent by the LCD screen.
//! @param file_index    The index of the filename to select
void SdCard::select_file(uint16_t file_index)
{
    if(!card.cardOK)
        return;

    if(file_index > last_file_index_)
        return;

    card.getfilename(last_file_index_ - file_index);
    if(card.filenameIsDir)
        return;

    String longName{(card.longFilename[0] == 0) ? card.filename : card.longFilename};
    if(longName.length() <= 0) // If the SD card is not readable
        return;

    advi3pp.set_progress_name(longName);

    WriteRamDataRequest frame{Variable::CurrentFileName};
    frame << FixedSizeString{longName, 26};
    frame.send(true);

    card.openFile(card.filename, true); // use always short filename so it will work even if the filename is long
    card.startFileprint();
    PrintCounter::start();

    pages.show_page(Page::Print);
}

// --------------------------------------------------------------------
// Printing
// --------------------------------------------------------------------

//! Handle print commands.
//! @param key_value    The sub-action to handle
bool Print::dispatch(KeyValue key_value)
{
    if(Handler::dispatch(key_value))
        return true;

    switch(key_value)
    {
        case KeyValue::PrintStop:           stop(); break;
        case KeyValue::PrintPauseResume:    pause_resume(); break;
        case KeyValue::PrintAdvancedPause:  advanced_pause(); break;
        default:                            return false;
    }

    return true;
}

Page Print::do_show()
{
    return Page::Print;
}

//! Stop printing
void Print::stop()
{
    Log::log() << F("Stop Print") << Log::endl();

    do_stop();
    clear_command_queue();
    quickstop_stepper();
    PrintCounter::stop();
    Temperature::disable_all_heaters();
    fanSpeeds[0] = 0;

    pages.show_back_page();
    task.set_background_task(BackgroundTask(this, &Print::reset_messages_task), 500);
}

void Print::reset_messages_task()
{
    task.clear_background_task();
    advi3pp.reset_progress();
    advi3pp.reset_message();
}

//! Pause printing
void Print::pause_resume()
{
    // FIX
    Log::log() << F("Pause or Resume Print") << Log::endl();

    if(is_printing())
    {
        advi3pp.queue_message(F("Pause printing..."));
        do_pause();
        PrintCounter::pause();
#if ENABLED(PARK_HEAD_ON_PAUSE)
        enqueue_and_echo_commands_P(PSTR("M125"));
#endif
    }
    else
    {
        Log::log() << F("Resume Print") << Log::endl();

        advi3pp.queue_message(F("Resume printing"));
#if ENABLED(PARK_HEAD_ON_PAUSE)
        enqueue_and_echo_commands_P(PSTR("M24"));
#endif
        do_resume();
        PrintCounter::start(); // TODO: Check this is right
    }
}

//! Resume the current SD printing
void Print::advanced_pause()
{
    enqueue_and_echo_commands_P(PSTR("M600"));
}

// --------------------------------------------------------------------
// SD Printing
// --------------------------------------------------------------------

void SdPrint::do_stop()
{
    card.stopSDPrint();
}

void SdPrint::do_pause()
{
    card.pauseSDPrint();
}

void SdPrint::do_resume()
{
    card.startFileprint();
}

bool SdPrint::is_printing() const
{
    return card.sdprinting;
}

// --------------------------------------------------------------------
// USB Printing
// --------------------------------------------------------------------

void UsbPrint::do_stop()
{
    SERIAL_ECHOLNPGM("//action:cancel");
}

void UsbPrint::do_pause()
{
    SERIAL_ECHOLNPGM("//action:pause");
}

void UsbPrint::do_resume()
{
    SERIAL_ECHOLNPGM("//action:resume");
}

bool UsbPrint::is_printing() const
{
    return PrintCounter::isRunning();
}

// --------------------------------------------------------------------
// Load and Unload Filament
// --------------------------------------------------------------------

//! Handle Load & Unload actions.
//! @param key_value    The sub-action to handle
bool LoadUnload::dispatch(KeyValue key_value)
{
    if(Handler::dispatch(key_value))
        return true;

    switch(key_value)
    {
        case KeyValue::Load:    load(); break;
        case KeyValue::Unload:  unload(); break;
        default:                return false;
    }

    return true;
}

//! Show the Load & Unload screen on the LCD display.
Page LoadUnload::do_show()
{
    WriteRamDataRequest frame{Variable::Value0};
    frame << Uint16(advi3pp.last_used_hotend_temperature());
    frame.send();

    return Page::LoadUnload;
}

void LoadUnload::prepare()
{
    ReadRamData frame{Variable::Value0, 1};
    if(!frame.send_and_receive())
    {
        Log::error() << F("Receiving Frame (Target Temperature)") << Log::endl();
        return;
    }

    Uint16 hotend; frame >> hotend;

    Temperature::setTargetHotend(hotend.word, 0);
    enqueue_and_echo_commands_P(PSTR("M83"));       // relative E mode
    enqueue_and_echo_commands_P(PSTR("G92 E0"));    // reset E axis
}

//! Start Load action.
void LoadUnload::load()
{
    prepare();

    task.set_background_task(BackgroundTask(this, &LoadUnload::load_start_task));
    wait.show(F("Wait until the target temp is reached..."),
              WaitCalllback(this, &LoadUnload::stop));
}

//! Start Load action.
void LoadUnload::unload()
{
    prepare();

    task.set_background_task(BackgroundTask(this, &LoadUnload::unload_start_task));
    wait.show(F("Wait until the target temp is reached..."),
              WaitCalllback(this, &LoadUnload::stop));
}


//! Handle back from the Load on Unload LCD screen.
void LoadUnload::stop()
{
    Log::log() << F("Load/Unload Stop");

    advi3pp.reset_message();
    task.set_background_task(BackgroundTask(this, &LoadUnload::stop_task));
    clear_command_queue();
    Temperature::setTargetHotend(0, 0);

    pages.show_back_page();
}

void LoadUnload::stop_task()
{
    if(advi3pp.is_busy() || !task.has_background_task())
        return;

    task.clear_background_task();
    advi3pp.reset_message();
    // Do this asynchronously to avoid race conditions
    enqueue_and_echo_commands_P(PSTR("M82"));       // absolute E mode
    enqueue_and_echo_commands_P(PSTR("G92 E0"));    // reset E axis

}

//! Load the filament if the temperature is high enough.
void LoadUnload::load_start_task()
{
    if(Temperature::current_temperature[0] >= Temperature::target_temperature[0] - 10)
    {
        Log::log() << F("Load Filament") << Log::endl();
        advi3pp.buzz(100); // Inform the user that the extrusion starts
        enqueue_and_echo_commands_P(PSTR("G1 E1 F120"));
        task.set_background_task(BackgroundTask(this, &LoadUnload::load_task));
        advi3pp.set_status(F("Wait until the filament comes out..."));
    }
}

//! Load the filament if the temperature is high enough.
void LoadUnload::load_task()
{
    if(Temperature::current_temperature[0] >= Temperature::target_temperature[0] - 10)
        enqueue_and_echo_commands_P(PSTR("G1 E1 F120"));
}


//! Unload the filament if the temperature is high enough.
void LoadUnload::unload_start_task()
{
    if(Temperature::current_temperature[0] >= Temperature::target_temperature[0] - 10)
    {
        Log::log() << F("Unload Filament") << Log::endl();
        advi3pp.buzz(100); // Inform the user that the un-extrusion starts
        enqueue_and_echo_commands_P(PSTR("G1 E-1 F120"));
        task.set_background_task(BackgroundTask(this, &LoadUnload::unload_task));
        advi3pp.set_status(F("Wait until the filament comes out..."));
    }
}

//! Unload the filament if the temperature is high enough.
void LoadUnload::unload_task()
{
    if(Temperature::current_temperature[0] >= Temperature::target_temperature[0] - 10)
        enqueue_and_echo_commands_P(PSTR("G1 E-1 F120"));
}

// --------------------------------------------------------------------
// Preheat & Cooldown
// --------------------------------------------------------------------

//! Handle Preheat actions.
//! @param key_value    Sub-action to handle
bool Preheat::dispatch(KeyValue key_value)
{
    if(Handler::dispatch(key_value))
        return true;

    switch(key_value)
    {
        case KeyValue::PresetPrevious:  previous(); break;
        case KeyValue::PresetNext:      next(); break;
        case KeyValue::Cooldown:        cooldown(); break;
        default:                        return false;
    }

    return true;
}

//! Store presets in permanent memory.
void Preheat::store_eeprom_data(EepromWrite& eeprom)
{
    for(auto& preset: presets_)
    {
        eeprom.write(preset.hotend);
        eeprom.write(preset.bed);
    }
}

//! Restore presets from permanent memory.
//! @param read Function to use for the actual reading
//! @param eeprom_index
//! @param working_crc
void Preheat::restore_eeprom_data(EepromRead& eeprom)
{
    for(auto& preset: presets_)
    {
        eeprom.read(preset.hotend);
        eeprom.read(preset.bed);
    }
}

//! Reset presets.
void Preheat::reset_eeprom_data()
{
    for(size_t i = 0; i < NB_PRESETS; ++i)
    {
        presets_[i].hotend  = DEFAULT_PREHEAT_PRESET[i][0];
        presets_[i].bed     = DEFAULT_PREHEAT_PRESET[i][1];
        presets_[i].fan     = DEFAULT_PREHEAT_PRESET[i][2];
    }
}

uint16_t Preheat::size_of_eeprom_data() const
{
    return NB_PRESETS * (sizeof(Preset::hotend) + sizeof(Preset::bed));
}

void Preheat::send_presets()
{
    Log::log() << F("Preheat page") << Log::endl();
    WriteRamDataRequest frame{Variable::Value0};
    frame << Uint16(presets_[index_].bed)
          << Uint16(presets_[index_].hotend)
          << Uint16(presets_[index_].fan);
    frame.send();
}

//! Show the preheat screen
Page Preheat::do_show()
{
    send_presets();
    return Page::Preheat;
}

void Preheat::previous()
{
    if(index_ <= 0)
        return;
    --index_;
    send_presets();
}

void Preheat::next()
{
    if(index_ >= NB_PRESETS - 1)
        return;
    ++index_;
    send_presets();
}

//! Cooldown the bed and the nozzle
void Preheat::cooldown()
{
    Log::log() << F("Cooldown") << Log::endl();
    advi3pp.reset_message();
    Temperature::disable_all_heaters();
}

// --------------------------------------------------------------------
// Move & Home
// --------------------------------------------------------------------

//! Execute a move command
bool Move::dispatch(KeyValue key_value)
{
    if(Handler::dispatch(key_value))
        return true;

    switch(key_value)
    {
        case KeyValue::MoveXHome:           x_home(); break;
        case KeyValue::MoveYHome:           y_home(); break;
        case KeyValue::MoveZHome:           z_home(); break;
        case KeyValue::MoveAllHome:         all_home(); break;
        case KeyValue::DisableMotors:       disable_motors(); break;
        default:                            return false;
    }

    return true;
}

Page Move::do_show()
{
    Planner::finish_and_disable(); // To circumvent homing problems
    return Page::Move;
}

//! Move the nozzle.
void Move::move(const char* command, millis_t delay)
{
    if(!ELAPSED(millis(), last_move_time_ + delay))
        return;
    enqueue_and_echo_commands_P(PSTR("G91"));
    enqueue_and_echo_commands_P(command);
    enqueue_and_echo_commands_P(PSTR("G90"));
    last_move_time_ = millis();
}


//! Move the nozzle.
void Move::x_plus()
{
    move(PSTR("G1 X4 F1000"), 150);
}

//! Move the nozzle.
void Move::x_minus()
{
    move(PSTR("G1 X-4 F1000"), 150);
}

//! Move the nozzle.
void Move::y_plus()
{
    move(PSTR("G1 Y4 F1000"), 150);
}

//! Move the nozzle.
void Move::y_minus()
{
    move(PSTR("G1 Y-4 F1000"), 150);
}

//! Move the nozzle.
void Move::z_plus()
{
    move(PSTR("G1 Z0.5 F1000"), 10);
}

//! Move the nozzle.
void Move::z_minus()
{
    move(PSTR("G1 Z-0.5 F1000"), 10);
}

//! Extrude some filament.
void Move::e_plus()
{
    if(Temperature::degHotend(0) < 180)
        return;

    clear_command_queue();
    enqueue_and_echo_commands_P(PSTR("G91"));
    enqueue_and_echo_commands_P(PSTR("G1 E1 F120"));
    enqueue_and_echo_commands_P(PSTR("G90"));
}

//! Unextrude.
void Move::e_minus()
{
    if(Temperature::degHotend(0) < 180)
        return;

    clear_command_queue();
    enqueue_and_echo_commands_P(PSTR("G91"));
    enqueue_and_echo_commands_P(PSTR("G1 E-1 F120"));
    enqueue_and_echo_commands_P(PSTR("G90"));
}

//! Disable the motors.
void Move::disable_motors()
{
    enqueue_and_echo_commands_P(PSTR("M84"));
    axis_homed = 0;
    axis_known_position = 0;
}

//! Go to home on the X axis.
void Move::x_home()
{
    enqueue_and_echo_commands_P(PSTR("G28 X0"));
}

//! Go to home on the Y axis.
void Move::y_home()
{
    enqueue_and_echo_commands_P(PSTR("G28 Y0"));
}

//! Go to home on the Z axis.
void Move::z_home()
{
    enqueue_and_echo_commands_P(PSTR("G28 Z0"));
}

//! Go to home on all axis.
void Move::all_home()
{
    enqueue_and_echo_commands_P(PSTR("G28"));
}

// --------------------------------------------------------------------
// Factory Reset
// --------------------------------------------------------------------

Page FactoryReset::do_show()
{
    return Page::FactoryReset;
}

void FactoryReset::do_save()
{
    enqueue_and_echo_commands_P(PSTR("M502"));
    advi3pp.save_settings();
}

// --------------------------------------------------------------------
// Statistics
// --------------------------------------------------------------------

Page Statistics::do_show()
{
    send_stats();
    return Page::Statistics;
}

void Statistics::send_stats()
{
    printStatistics stats = print_job_timer.getStats();

    WriteRamDataRequest frame{Variable::TotalPrints};
    frame << Uint16(stats.totalPrints) << Uint16(stats.finishedPrints);
    frame.send();

    duration_t duration = stats.printTime;
    frame.reset(Variable::TotalPrintTime);
    frame << FixedSizeString{duration, 16};
    frame.send();

    duration = stats.longestPrint;
    frame.reset(Variable::LongestPrintTime);
    frame << FixedSizeString{duration, 16};
    frame.send();

    String filament_used;
    filament_used << static_cast<unsigned int>(stats.filamentUsed / 1000)
                  << "."
                  << static_cast<unsigned int>(stats.filamentUsed / 100) % 10
                  << "m";
    frame.reset(Variable::TotalFilament);
    frame << FixedSizeString{filament_used, 16};
    frame.send();
}

// --------------------------------------------------------------------
// Versions
// --------------------------------------------------------------------

void Versions::get_advi3pp_lcd_version()
{
    ReadRamData response{Variable::ADVi3ppLCDversion, 1};
    if(!response.send_and_receive())
    {
        Log::error() << F("Receiving Frame (Measures)") << Log::endl();
        return;
    }

    Uint16 version; response >> version;
    Log::log() << F("ADVi3++ LCD version = ") <<  version.word << Log::endl();
    lcd_version_ = version.word;
}

//! Get the current LCD firmware version.
//! @return     The version as a string.
String Versions::get_lcd_firmware_version()
{
    ReadRegister response{Register::Version, 1};
    if(!response.send_and_receive())
    {
        Log::error() << F("Receiving Frame (Version)") << Log::endl();
        return F("Unknown");
    }

    Uint8 version; response >> version;
    String lcd_version; lcd_version << (version.byte / 0x10) << "." << (version.byte % 0x10);
    Log::log() << F("LCD Firmware raw version = ") << version.byte << Log::endl();
    return lcd_version;
}

//! Convert a version from its hexadecimal representation.
//! @param hex_version  Hexadecimal representation of the version
//! @return             Version as a string
String convert_version(uint16_t hex_version)
{
    String version;
    version << hex_version / 0x0100 << "." << (hex_version % 0x100) / 0x10 << "." << hex_version % 0x10;
    return version;
}

//! Send the different versions to the LCD screen.
void Versions::send_versions()
{
    String marlin_version{SHORT_BUILD_VERSION};
    String motherboard_version = convert_version(advi3_pp_version);
    String advi3pp_lcd_version = convert_version(lcd_version_);
    String lcd_firmware_version = get_lcd_firmware_version();

    WriteRamDataRequest frame{Variable::MotherboardVersion};
    frame << FixedSizeString(motherboard_version, 16)
          << FixedSizeString(advi3pp_lcd_version, 16)
          << FixedSizeString(lcd_firmware_version, 16)
		  << FixedSizeString(marlin_version, 16);
    frame.send();
}

bool Versions::is_lcd_version_valid() const
{
    return lcd_version_ >= advi3_pp_oldest_lcd_compatible_version && lcd_version_ <= advi3_pp_newest_lcd_compatible_version;
}

bool Versions::dispatch(KeyValue key_value)
{
    if(Handler::dispatch(key_value))
        return true;

    switch(key_value)
    {
        case KeyValue::MismatchForward:         versions_mismatch_forward(); break;
        default:								return false;
    }

    return true;
}

void Versions::versions_mismatch_forward()
{
    pages.show_page(Page::Main, false);
}

Page Versions::do_show()
{
    return Page::Versions;
}

// --------------------------------------------------------------------
// Sponsors
// --------------------------------------------------------------------

Page Sponsors::do_show()
{
    return Page::Sponsors;
}

// --------------------------------------------------------------------
// Copyrights
// --------------------------------------------------------------------

Page Copyrights::do_show()
{
    return Page::Copyrights;
}

// --------------------------------------------------------------------
// PID Tuning
// --------------------------------------------------------------------

//! Handle PID tuning.
//! @param key_value    The step of the PID tuning
bool PidTuning::dispatch(KeyValue key_value)
{
    if(Handler::dispatch(key_value))
        return true;

    switch(key_value)
    {
        case KeyValue::PidTuningStep2:   pid_tuning_step2(); break;
        default:                         return false;
    }

    return true;
}

//! Show step #1 of PID tuning
Page PidTuning::do_show()
{
    WriteRamDataRequest frame{Variable::Value0};
    frame << Uint16(advi3pp.last_used_hotend_temperature());
    frame.send();

    return Page::PidTuning1;
}

//! Show step #2 of PID tuning
void PidTuning::pid_tuning_step2()
{
    ReadRamData frame{Variable::Value0, 1};
    if(!frame.send_and_receive())
    {
        Log::error() << F("Receiving Frame (Target Temperature)") << Log::endl();
        return;
    }

    Uint16 hotend; frame >> hotend;

    enqueue_and_echo_commands_P(PSTR("M106 S255")); // Turn on fan
    String auto_pid_command; auto_pid_command << F("M303 S") << hotend.word << F("E0 C8 U1");
    enqueue_and_echo_command(auto_pid_command.c_str());

    pages.show_page(Page::PidTuning2);
};

//! PID automatic tuning is finished.
void PidTuning::finished()
{
    Log::log() << F("Auto PID finished") << Log::endl();
    enqueue_and_echo_commands_P(PSTR("M106 S0"));
    pid_settings.show(false, false);
}

// --------------------------------------------------------------------
// Manual Leveling
// --------------------------------------------------------------------

bool ManualLeveling::dispatch(KeyValue key_value)
{
    if(Handler::dispatch(key_value))
        return true;

    switch(key_value)
    {
        case KeyValue::LevelingPoint1:  point1(); break;
        case KeyValue::LevelingPoint2:  point2(); break;
        case KeyValue::LevelingPoint3:  point3(); break;
        case KeyValue::LevelingPoint4:  point4(); break;
        case KeyValue::LevelingPoint5:  point5(); break;
        case KeyValue::LevelingPointA:  pointA(); break;
        case KeyValue::LevelingPointB:  pointB(); break;
        case KeyValue::LevelingPointC:  pointC(); break;
        case KeyValue::LevelingPointD:  pointD(); break;
        default:                        return false;
    }

    return true;
}

void ManualLeveling::back()
{
    enqueue_and_echo_commands_P(PSTR("G1 Z30 F2000"));
    Handler::back();
}

//! Home the printer for bed leveling.
Page ManualLeveling::do_show()
{
    wait.show(F("Homing..."));
    axis_homed = 0;
    axis_known_position = 0;
    enqueue_and_echo_commands_P(PSTR("G90")); // absolute mode
    enqueue_and_echo_commands_P((PSTR("G28"))); // homing
    task.set_background_task(BackgroundTask(this, &ManualLeveling::leveling_task), 200);
    return Page::None;
}

//! Leveling Background task.
void ManualLeveling::leveling_task()
{
    if(!TEST(axis_homed, X_AXIS) || !TEST(axis_homed, Y_AXIS) || !TEST(axis_homed, Z_AXIS))
        return;

    Log::log() << F("Leveling Homed, start process") << Log::endl();
    advi3pp.reset_message();
    task.clear_background_task();
    pages.show_page(Page::ManualLeveling, false);
}


//! Handle leveling point #1.
void ManualLeveling::point1()
{
    Log::log() << F("Level point 1") << Log::endl();
    enqueue_and_echo_commands_P(PSTR("G1 Z10 F2000"));
    enqueue_and_echo_commands_P(PSTR("G1 X30 Y30 F6000"));
    enqueue_and_echo_commands_P(PSTR("G1 Z0 F1000"));
}

//! Handle leveling point #2.
void ManualLeveling::point2()
{
    Log::log() << F("Level point 2") << Log::endl();
    enqueue_and_echo_commands_P(PSTR("G1 Z10 F2000"));
    enqueue_and_echo_commands_P(PSTR("G1 X170 Y170 F6000"));
    enqueue_and_echo_commands_P(PSTR("G1 Z0 F1000"));
}

//! Handle leveling point #3.
void ManualLeveling::point3()
{
    Log::log() << F("Level point 3") << Log::endl();
    enqueue_and_echo_commands_P(PSTR("G1 Z10 F2000"));
    enqueue_and_echo_commands_P(PSTR("G1 X170 Y30 F6000"));
    enqueue_and_echo_commands_P(PSTR("G1 Z0 F1000"));
}

//! Handle leveling point #4.
void ManualLeveling::point4()
{
    Log::log() << F("Level point 4") << Log::endl();
    enqueue_and_echo_commands_P(PSTR("G1 Z10 F2000"));
    enqueue_and_echo_commands_P(PSTR("G1 X30 Y170 F6000"));
    enqueue_and_echo_commands_P(PSTR("G1 Z0 F1000"));
}

//! Handle leveling point #5.
void ManualLeveling::point5()
{
    Log::log() << F("Level point 5") << Log::endl();
    enqueue_and_echo_commands_P(PSTR("G1 Z10 F2000"));
    enqueue_and_echo_commands_P(PSTR("G1 X100 Y100 F6000"));
    enqueue_and_echo_commands_P(PSTR("G1 Z0 F1000"));
}

void ManualLeveling::pointA()
{
    Log::log() << F("Level point A") << Log::endl();
    enqueue_and_echo_commands_P(PSTR("G1 Z10 F2000"));
    enqueue_and_echo_commands_P(PSTR("G1 X100 Y30 F6000"));
    enqueue_and_echo_commands_P(PSTR("G1 Z0 F1000"));
}

void ManualLeveling::pointB()
{
    Log::log() << F("Level point B") << Log::endl();
    enqueue_and_echo_commands_P(PSTR("G1 Z10 F2000"));
    enqueue_and_echo_commands_P(PSTR("G1 X30 Y100 F6000"));
    enqueue_and_echo_commands_P(PSTR("G1 Z0 F1000"));
}

void ManualLeveling::pointC()
{
    Log::log() << F("Level point C") << Log::endl();
    enqueue_and_echo_commands_P(PSTR("G1 Z10 F2000"));
    enqueue_and_echo_commands_P(PSTR("G1 X100 Y170 F6000"));
    enqueue_and_echo_commands_P(PSTR("G1 Z0 F1000"));
}

void ManualLeveling::pointD()
{
    Log::log() << F("Level point D") << Log::endl();
    enqueue_and_echo_commands_P(PSTR("G1 Z10 F2000"));
    enqueue_and_echo_commands_P(PSTR("G1 X170 Y100 F6000"));
    enqueue_and_echo_commands_P(PSTR("G1 Z0 F1000"));
}

// --------------------------------------------------------------------
// Extruder tuning
// --------------------------------------------------------------------

bool ExtruderTuning::dispatch(KeyValue key_value)
{
    if(Handler::dispatch(key_value))
        return true;

    switch(key_value)
    {
        case KeyValue::TuningStart:     start(); break;
        case KeyValue::TuningSettings:  settings(); break;
        default:                        return false;
    }

    return true;
}

//! Show the extruder tuning screen.
Page ExtruderTuning::do_show()
{
    WriteRamDataRequest frame{Variable::Value0};
    frame << Uint16(advi3pp.last_used_hotend_temperature());
    frame.send();

    pages.save_forward_page();
    return Page::ExtruderTuningTemp;
}

//! Start extruder tuning.
void ExtruderTuning::start()
{
    ReadRamData frame{Variable::Value0, 1};
    if(!frame.send_and_receive())
    {
        Log::error() << F("Receiving Frame (Target Temperature)") << Log::endl();
        return;
    }

    Uint16 hotend; frame >> hotend;

    wait.show(F("Heating the extruder..."));
    Temperature::setTargetHotend(hotend.word, 0);

    task.set_background_task(BackgroundTask(this, &ExtruderTuning::heating_task));
}

//! Extruder tuning background task.
void ExtruderTuning::heating_task()
{
    if(Temperature::current_temperature[0] < Temperature::target_temperature[0] - 10)
        return;
    task.clear_background_task();

    advi3pp.set_status(F("Wait until the extrusion is finished..."));
    enqueue_and_echo_commands_P(PSTR("G1 Z20 F240"));   // raise head
    enqueue_and_echo_commands_P(PSTR("M83"));           // relative E mode
    enqueue_and_echo_commands_P(PSTR("G92 E0"));        // reset E axis

    String command; command << F("G1 E") << tuning_extruder_filament << " F50"; // Extrude slowly
    enqueue_and_echo_command(command.c_str());

    task.set_background_task(BackgroundTask(this, &ExtruderTuning::extruding_task));
}

//! Extruder tuning background task.
void ExtruderTuning::extruding_task()
{
    if(current_position[E_AXIS] < tuning_extruder_filament || advi3pp.is_busy())
        return;
    task.clear_background_task();

    extruded_ = current_position[E_AXIS];

    Temperature::setTargetHotend(0, 0);
    task.clear_background_task();
    advi3pp.reset_message();
    finished();
}

//! Record the amount of filament extruded.
void ExtruderTuning::finished()
{
    Log::log() << F("Filament extruded ") << extruded_ << Log::endl();
    enqueue_and_echo_commands_P(PSTR("M82"));       // absolute E mode
    enqueue_and_echo_commands_P(PSTR("G92 E0"));    // reset E axis

    task.clear_background_task();
    pages.show_page(Page::ExtruderTuningMeasure, false);
}

//! Cancel the extruder tuning.
void ExtruderTuning::back()
{
    task.clear_background_task();

    Temperature::setTargetHotend(0, 0);

    enqueue_and_echo_commands_P(PSTR("M82"));       // absolute E mode
    enqueue_and_echo_commands_P(PSTR("G92 E0"));    // reset E axis

    Handler::back();
}


//! Compute the extruder (E axis) new value and show the steps settings.
void ExtruderTuning::settings()
{
    ReadRamData response{Variable::Value0, 1};
    if(!response.send_and_receive())
    {
        Log::error() << F("Receiving Frame (Measures)") << Log::endl();
        return;
    }

    Uint16 e; response >> e;
    e.word /= 10;

    // Fill all values because all 4 axis are displayed by show_steps_settings
    // TODO
    /*steps_settings.axis_steps_per_mm[X_AXIS] = Planner::axis_steps_per_mm[X_AXIS];
    steps_settings.axis_steps_per_mm[Y_AXIS] = Planner::axis_steps_per_mm[Y_AXIS];
    steps_settings.axis_steps_per_mm[Z_AXIS] = Planner::axis_steps_per_mm[Z_AXIS];
    steps_settings.axis_steps_per_mm[E_AXIS] = Planner::axis_steps_per_mm[E_AXIS]
                                       * extruded_ / (extruded_ + tuning_extruder_delta - e.word);*/

	/*Log::log() << F("Adjust: old = ")
               << Planner::axis_steps_per_mm[E_AXIS]
               << F(", expected = ") << extruded_
               << F(", measured = ") << (extruded_ + tuning_extruder_delta - e.word)
               << F(", new = ") << steps_settings.axis_steps_per_mm[E_AXIS] << Log::endl();*/

    steps_settings.show(false);
}

#ifdef ADVi3PP_BLTOUCH

// --------------------------------------------------------------------
// Sensor Settings
// --------------------------------------------------------------------

Page SensorSettings::do_show()
{
    send_z_height_to_lcd(zprobe_zoffset);
    pages.save_forward_page();
    return Page::SensorSettings;
}

void SensorSettings::do_save()
{
    save_lcd_z_height();
}

// --------------------------------------------------------------------
// Sensor Tuning
// --------------------------------------------------------------------

bool SensorTuning::dispatch(KeyValue key_value)
{
    if(Handler::dispatch(key_value))
        return true;

    switch(key_value)
    {
        case KeyValue::SensorSelfTest:  self_test(); break;
        case KeyValue::SensorReset:     reset(); break;
        case KeyValue::SensorDeploy:    deploy(); break;
        case KeyValue::SensorStow:      stow(); break;
        //case KeyValue::SensorZHeight:   z_height(); break;
        default:                        return false;
    }

    return true;
}

Page SensorTuning::do_show()
{
    return Page::SensorTuning;
}

void SensorTuning::leveling()
{
    sensor_interactive_leveling_ = true;
    pages.save_forward_page();
    wait.show(F("Homing..."));
    enqueue_and_echo_commands_P(PSTR("G28"));                   // homing
    enqueue_and_echo_commands_P(PSTR("G1 Z10 F240"));           // raise head
    enqueue_and_echo_commands_P(PSTR("G29 E"));                 // leveling
    enqueue_and_echo_commands_P(PSTR("G1 X0 Y0 F3000"));        // go back to corner
}

void SensorTuning::g29_leveling_finished(bool success)
{
    if(!success)
    {
        if(!sensor_interactive_leveling_ && !IS_SD_FILE_OPEN) // i.e. USB print
            SERIAL_ECHOLNPGM("//action:disconnect"); // "disconnect" is the only standard command to stop an USB print

        if(sensor_interactive_leveling_)
            wait.show(F("Leveling failed"), WaitCalllback(this, &SensorTuning::g29_leveling_failed), false);
        else
            advi3pp.set_status(F("Leveling failed"));

        sensor_interactive_leveling_ = false;
        return;
    }

    advi3pp.reset_message();

    if(sensor_interactive_leveling_)
    {
        sensor_interactive_leveling_ = false;
        sensor_grid.show();
    }
    else
    {
        enqueue_and_echo_commands_P(PSTR("M500"));      // Save settings (including mash)
        enqueue_and_echo_commands_P(PSTR("M420 S1"));   // Set bed leveling state (enable)
    }
}

void SensorTuning::g29_leveling_failed()
{
    pages.show_back_page();
}

void SensorTuning::self_test()
{
    enqueue_and_echo_commands_P(PSTR("M280 P0 S120"));
}

void SensorTuning::reset()
{
    enqueue_and_echo_commands_P(PSTR("M280 P0 S160"));
}

void SensorTuning::deploy()
{
    enqueue_and_echo_commands_P(PSTR("M280 P0 S10"));
}

void SensorTuning::stow()
{
    enqueue_and_echo_commands_P(PSTR("M280 P0 S90"));
}

// --------------------------------------------------------------------
// Sensor grid
// --------------------------------------------------------------------

Page SensorGrid::do_show()
{
    WriteRamDataRequest frame{Variable::Value0};
    for(auto y = 0; y < GRID_MAX_POINTS_Y; y++)
        for(auto x = 0; x < GRID_MAX_POINTS_X; x++)
            frame << Uint16(static_cast<int16_t>(z_values[x][y] * 1000));
    frame.send();

    return Page::SensorGrid;
}

void SensorGrid::do_save()
{
    enqueue_and_echo_commands_P(PSTR("M500"));      // Save settings (including mash)
    enqueue_and_echo_commands_P(PSTR("M420 S1"));   // Set bed leveling state (enable)
}

// --------------------------------------------------------------------
// Sensor Z-Height
// --------------------------------------------------------------------

Page SensorZHeight::do_show()
{
    pages.save_forward_page();
    wait.show(F("Homing..."));
    enqueue_and_echo_commands_P((PSTR("G28")));  // homing
    task.set_background_task(BackgroundTask(this, &SensorZHeight::home_task), 200);
    return Page::None;
}

void SensorZHeight::home_task()
{
    if(!TEST(axis_homed, X_AXIS) || !TEST(axis_homed, Y_AXIS) || !TEST(axis_homed, Z_AXIS))
        return;
    if(advi3pp.is_busy())
        return;

    advi3pp.set_status(F("Going to the middle of the bed..."));
    enqueue_and_echo_commands_P(PSTR("G1 Z10 F240"));           // raise head
    enqueue_and_echo_commands_P(PSTR("G1 X100 Y100 F3000"));    // center of the bed
    enqueue_and_echo_commands_P(PSTR("G1 Z0 F240"));            // lower head

    task.set_background_task(BackgroundTask(this, &SensorZHeight::center_task), 200);
}

void SensorZHeight::center_task()
{
    if(current_position[X_AXIS] != 100 || current_position[Y_AXIS] != 100 || current_position[Z_AXIS] != 0)
        return;
    if(advi3pp.is_busy())
        return;
    task.clear_background_task();

    advi3pp.reset_message();
    pages.show_page(Page::ZHeightTuning, false);
}

void SensorZHeight::do_rollback()
{
    enqueue_and_echo_commands_P((PSTR("G28 X0 Y0"))); // homing
}

void SensorZHeight::save()
{
    wait.show(F("Measure Z-height"));
    enqueue_and_echo_commands_P(PSTR("I0")); // measure z-height
}

#else

Page SensorSettings::do_show()
{
    return Page::NoSensor;
}

Page SensorTuning::do_show()
{
    return Page::NoSensor;
}

Page SensorGrid::do_show()
{
    return Page::NoSensor;
}

Page SensorZHeight::do_show()
{
    return Page::NoSensor;
}

#endif

// --------------------------------------------------------------------
// No Sensor
// --------------------------------------------------------------------

Page NoSensor::do_show()
{
    return Page::NoSensor;
}


// --------------------------------------------------------------------
// Change Filament
// --------------------------------------------------------------------

Page ChangeFilament::do_show()
{
    return Page::None;
}

// --------------------------------------------------------------------
// Features Settings
// --------------------------------------------------------------------

void FeaturesSettings::send_features()
{
    WriteRamDataRequest frame{Variable::Value0};
    frame << Uint16(static_cast<uint16_t>(features_));
    frame.send();
}

void FeaturesSettings::do_save()
{
    advi3pp.change_features(features_);
}

// --------------------------------------------------------------------
// Firmware Settings
// --------------------------------------------------------------------

bool FirmwareSettings::dispatch(KeyValue key_value)
{
    if(Handler::dispatch(key_value))
        return true;

    switch(key_value)
    {
        case KeyValue::ThermalProtection:   thermal_protection(); break;
        case KeyValue::USBBaudrateMinus:    baudrate_minus(); break;
        case KeyValue::USBBaudratePlus:     baudrate_plus(); break;
        default:                            return false;
    }

    return true;
}

Page FirmwareSettings::do_show()
{
    usb_baudrate_ = advi3pp.get_current_baudrate();
    features_ = advi3pp.get_current_features();
    send_usb_baudrate();
    send_features();

    pages.save_forward_page();
    return Page::Firmware;
}

void FirmwareSettings::thermal_protection()
{
    flip_bits(features_, Feature::ThermalProtection);
    send_features();
}

void FirmwareSettings::do_save()
{
    advi3pp.change_usb_baudrate(usb_baudrate_);
    FeaturesSettings::do_save();
}

void FirmwareSettings::send_usb_baudrate()
{
    String value; value << usb_baudrate_;

    WriteRamDataRequest frame{Variable::ValueText};
    frame << FixedSizeString{value, 6};
    frame.send();
}

static size_t UsbBaudrateIndex(uint32_t baudrate)
{
    size_t nb = countof(usb_baudrates);
    for(size_t i = 0; i < nb; ++i)
        if(baudrate == usb_baudrates[i])
            return i;
    return 0;
}

void FirmwareSettings::baudrate_minus()
{
    auto index = UsbBaudrateIndex(usb_baudrate_);
    usb_baudrate_ = index > 0 ? usb_baudrates[index - 1] : usb_baudrates[0];
    send_usb_baudrate();
}

void FirmwareSettings::baudrate_plus()
{
    auto index = UsbBaudrateIndex(usb_baudrate_);
    static const auto max = countof(usb_baudrates) - 1;
    usb_baudrate_ = index < max ? usb_baudrates[index + 1] : usb_baudrates[max];
    send_usb_baudrate();
}

// --------------------------------------------------------------------
// LCD Settings
// --------------------------------------------------------------------

bool LcdSettings::dispatch(KeyValue key_value)
{
    if(Handler::dispatch(key_value))
        return true;

    switch(key_value)
    {
        case KeyValue::LCDDimming:          dim(); break;
        case KeyValue::Buzzer:              buzzer(); break;
        case KeyValue::BuzzOnPress:         buzz_on_press(); break;
        default:                            return false;
    }

    return true;
}

Page LcdSettings::do_show()
{
    features_ = advi3pp.get_current_features();
    send_features();
    return Page::LCD;
}

void LcdSettings::dim()
{
    flip_bits(features_, Feature::Dimming);
    dimming.enable(test_one_bit(features_, Feature::Dimming));
    send_features();
    advi3pp.save_settings();
}

void LcdSettings::buzzer()
{
    flip_bits(features_, Feature::Buzzer);
    advi3pp.enable_buzzer(test_one_bit(features_, Feature::Buzzer));
    send_features();
    advi3pp.save_settings();
}

void LcdSettings::buzz_on_press()
{
    flip_bits(features_, Feature::BuzzOnPress);
    advi3pp.enable_buzz_on_press(test_one_bit(features_, Feature::BuzzOnPress));
    send_features();
    advi3pp.save_settings();
}

// --------------------------------------------------------------------
// Linear Advance
// --------------------------------------------------------------------

// --------------------------------------------------------------------
// Print Settings
// --------------------------------------------------------------------

bool PrintSettings::dispatch(KeyValue key_value)
{
    if(Handler::dispatch(key_value))
        return true;

    switch(key_value)
    {
        case KeyValue::Baby001:     [this]{ multiplier_ = 0.01; }; break;
        case KeyValue::Baby005:     [this]{ multiplier_ = 0.05; }; break;
        case KeyValue::Baby010:     [this]{ multiplier_ = 0.10; }; break;
        default:                    return false;
    }

    return true;
}

//! Display on the LCD screen the printing settings.
Page PrintSettings::do_show()
{
    WriteRamDataRequest frame{Variable::Value0};
    frame << Uint16(feedrate_percentage)
          << Uint16(scale(fanSpeeds[0], 255, 100))
          << Uint16(Temperature::degTargetHotend(0))
          << Uint16(Temperature::degTargetBed());
    frame.send();

    return Page::PrintSettings;
}

//! Save the printing settings.
void PrintSettings::do_save()
{
    ReadRamData response{Variable::Value0, 5};
    if(!response.send_and_receive())
    {
        Log::error() << F("Receiving Frame (Print Settings)") << Log::endl();
        return;
    }

    Uint16 speed, fan, hotend, bed;
    response >> speed >> hotend >> bed >> fan;

    feedrate_percentage = speed.word;
    Temperature::setTargetHotend(hotend.word, 0);
    Temperature::setTargetBed(bed.word);
    fanSpeeds[0] = scale(fan.word, 100, 255);
}

void PrintSettings::baby_minus()
{
    String auto_pid_command; auto_pid_command << F("M290 Z-") << multiplier_;
    enqueue_and_echo_command(auto_pid_command.c_str());
}

void PrintSettings::baby_plus()
{
    String auto_pid_command; auto_pid_command << F("M290 Z") << multiplier_;
    enqueue_and_echo_command(auto_pid_command.c_str());
}

// --------------------------------------------------------------------
// PidSettings
// --------------------------------------------------------------------

void PidSettings::do_backup()
{
    backup_.Kp_ = Temperature::Kp;
    backup_.Ki_ = Temperature::Ki;
    backup_.Kd_ = Temperature::Kd;
}

void PidSettings::do_rollback()
{
    Temperature::Kp = backup_.Kp_;
    Temperature::Ki = backup_.Ki_;
    Temperature::Kd = backup_.Kd_;
}

void PidSettings::set(uint16_t temperature, bool bed)
{
    bed_ = bed;
    for(size_t i = 0; i < NB_PIDs; ++i)
    {
        if(temperature == pid_[bed_][i].temperature_)
        {
            index_ = i;
            return;
        }
    }

    // Temperature not found, so assign index 0, move PIDs and forget the last one
    index_ = 0;
    for(size_t i = 1; i < NB_PIDs; ++i)
        pid_[bed_][i] = pid_[bed_][i - 1];
}

void PidSettings::next()
{
 // TODO
}

void PidSettings::previous()
{
    // TODO
}

//! Show the PID settings
Page PidSettings::do_show()
{
    const Pid& pid = pid_[bed_][index_];
    WriteRamDataRequest frame{Variable::Value0};
    frame << Uint16(bed_ ? 0_u16 : 1_u16)
          << Uint16(pid.Kp_ * 100)
          << Uint16(unscalePID_i(pid.Ki_) * 100)
          << Uint16(unscalePID_d(pid.Kd_) * 100);
    frame.send();

    return Page::PidSettings;
}

//! Save the PID settings
void PidSettings::do_save()
{
    Pid& pid = pid_[bed_][index_];

    ReadRamData response{Variable::Value0, 4};
    if(!response.send_and_receive())
    {
        Log::error() << F("Receiving Frame (PID Settings)") << Log::endl();
        return;
    }

    Uint16 temperature, p, i, d;
    response >> temperature >> p >> i >> d;

    Temperature::Kp = static_cast<float>(p.word) / 100;
    Temperature::Ki = scalePID_i(static_cast<float>(i.word) / 100);
    Temperature::Kd = scalePID_d(static_cast<float>(d.word) / 100);
}

// --------------------------------------------------------------------
// Steps Settings
// --------------------------------------------------------------------

//! Initialize temporary Step settings.
void StepSettings::do_backup()
{
    backup_[X_AXIS] = Planner::axis_steps_per_mm[X_AXIS];
    backup_[Y_AXIS] = Planner::axis_steps_per_mm[Y_AXIS];
    backup_[Z_AXIS] = Planner::axis_steps_per_mm[Z_AXIS];
    backup_[E_AXIS] = Planner::axis_steps_per_mm[E_AXIS];
}

//! Save temporary Step settings.
void StepSettings::do_rollback()
{
    Planner::axis_steps_per_mm[X_AXIS] = backup_[X_AXIS];
    Planner::axis_steps_per_mm[Y_AXIS] = backup_[Y_AXIS];
    Planner::axis_steps_per_mm[Z_AXIS] = backup_[Z_AXIS];
    Planner::axis_steps_per_mm[E_AXIS] = backup_[E_AXIS];

    advi3pp.save_settings();
}

//! Show the Steps settings
//! @param init     Initialize the settings are use those already set
Page StepSettings::do_show()
{
    WriteRamDataRequest frame{Variable::Value0};
    frame << Uint16(Planner::axis_steps_per_mm[X_AXIS] * 10)
          << Uint16(Planner::axis_steps_per_mm[Y_AXIS] * 10)
          << Uint16(Planner::axis_steps_per_mm[Z_AXIS] * 10)
          << Uint16(Planner::axis_steps_per_mm[E_AXIS] * 10);
    frame.send();

    return Page::StepsSettings;
}

//! Save the Steps settings
void StepSettings::do_save()
{
    ReadRamData response{Variable::Value0, 4};
    if(!response.send_and_receive())
    {
        Log::error() << F("Receiving Frame (Steps Settings)") << Log::endl();
        return;
    }

    Uint16 x, y, z, e;
    response >> x >> y >> z >> e;

    Planner::axis_steps_per_mm[X_AXIS] = static_cast<float>(x.word) / 10;
    Planner::axis_steps_per_mm[Y_AXIS] = static_cast<float>(y.word) / 10;
    Planner::axis_steps_per_mm[Z_AXIS] = static_cast<float>(z.word) / 10;
    Planner::axis_steps_per_mm[E_AXIS] = static_cast<float>(e.word) / 10;
}

// --------------------------------------------------------------------
// Feedrate Settings
// --------------------------------------------------------------------

//! Initialize temporary Feedrate settings.
void FeedrateSettings::do_backup()
{
    backup_max_feedrate_mm_s_[X_AXIS] = Planner::max_feedrate_mm_s[X_AXIS];
    backup_max_feedrate_mm_s_[Y_AXIS] = Planner::max_feedrate_mm_s[Y_AXIS];
    backup_max_feedrate_mm_s_[Z_AXIS] = Planner::max_feedrate_mm_s[Z_AXIS];
    backup_max_feedrate_mm_s_[E_AXIS] = Planner::max_feedrate_mm_s[E_AXIS];
    backup_min_feedrate_mm_s_ = Planner::min_feedrate_mm_s;
    backup_min_travel_feedrate_mm_s_ = Planner::min_travel_feedrate_mm_s;
}

//! Save temporary Feedrate settings.
void FeedrateSettings::do_rollback()
{
    Planner::max_feedrate_mm_s[X_AXIS] = backup_max_feedrate_mm_s_[X_AXIS];
    Planner::max_feedrate_mm_s[Y_AXIS] = backup_max_feedrate_mm_s_[Y_AXIS];
    Planner::max_feedrate_mm_s[Z_AXIS] = backup_max_feedrate_mm_s_[Z_AXIS];
    Planner::max_feedrate_mm_s[E_AXIS] = backup_max_feedrate_mm_s_[E_AXIS];
    Planner::min_feedrate_mm_s = backup_min_feedrate_mm_s_;
    Planner::min_travel_feedrate_mm_s = backup_min_travel_feedrate_mm_s_;

    advi3pp.save_settings();
}

//! Show the Feedrate settings
Page FeedrateSettings::do_show()
{
    WriteRamDataRequest frame{Variable::Value0};
    frame << Uint16(Planner::max_feedrate_mm_s[X_AXIS])
          << Uint16(Planner::max_feedrate_mm_s[Y_AXIS])
          << Uint16(Planner::max_feedrate_mm_s[Z_AXIS])
          << Uint16(Planner::max_feedrate_mm_s[E_AXIS])
          << Uint16(Planner::min_feedrate_mm_s)
          << Uint16(Planner::min_travel_feedrate_mm_s);
    frame.send();

    return Page::FeedrateSettings;
}

//! Save the Feedrate settings
void FeedrateSettings::do_save()
{
    ReadRamData response{Variable::Value0, 6};
    if(!response.send_and_receive())
    {
        Log::error() << F("Receiving Frame (Feedrate Settings)") << Log::endl();
        return;
    }

    Uint16 x, y, z, e, min, travel;
    response >> x >> y >> z >> e >> min >> travel;

    Planner::max_feedrate_mm_s[X_AXIS] = static_cast<float>(x.word);
    Planner::max_feedrate_mm_s[Y_AXIS] = static_cast<float>(y.word);
    Planner::max_feedrate_mm_s[Z_AXIS] = static_cast<float>(z.word);
    Planner::max_feedrate_mm_s[E_AXIS] = static_cast<float>(e.word);
    Planner::min_feedrate_mm_s         = static_cast<float>(min.word);
    Planner::min_travel_feedrate_mm_s  = static_cast<float>(travel.word);
}

// --------------------------------------------------------------------
// AccelerationSettings
// --------------------------------------------------------------------

//! Initialize temporary Acceleration settings.
void AccelerationSettings::do_backup()
{
    backup_max_acceleration_mm_per_s2_[X_AXIS] = Planner::max_acceleration_mm_per_s2[X_AXIS];
    backup_max_acceleration_mm_per_s2_[Y_AXIS] = Planner::max_acceleration_mm_per_s2[Y_AXIS];
    backup_max_acceleration_mm_per_s2_[Z_AXIS] = Planner::max_acceleration_mm_per_s2[Z_AXIS];
    backup_max_acceleration_mm_per_s2_[E_AXIS] = Planner::max_acceleration_mm_per_s2[E_AXIS];
    backup_acceleration_ = Planner::acceleration;
    backup_retract_acceleration_ = Planner::retract_acceleration;
    backup_travel_acceleration_ = Planner::travel_acceleration;
}

//! Save temporary Acceleration settings.
void AccelerationSettings::do_rollback()
{
    Planner::max_acceleration_mm_per_s2[X_AXIS] = backup_max_acceleration_mm_per_s2_[X_AXIS];
    Planner::max_acceleration_mm_per_s2[Y_AXIS] = backup_max_acceleration_mm_per_s2_[Y_AXIS];
    Planner::max_acceleration_mm_per_s2[Z_AXIS] = backup_max_acceleration_mm_per_s2_[Z_AXIS];
    Planner::max_acceleration_mm_per_s2[E_AXIS] = backup_max_acceleration_mm_per_s2_[E_AXIS];
    Planner::acceleration = backup_acceleration_;
    Planner::retract_acceleration = backup_retract_acceleration_;
    Planner::travel_acceleration =  backup_travel_acceleration_;

    advi3pp.save_settings();
}

//! Show the Acceleration settings
Page AccelerationSettings::do_show()
{
    WriteRamDataRequest frame{Variable::Value0};
    frame << Uint16(static_cast<uint16_t>(Planner::max_acceleration_mm_per_s2[X_AXIS]))
          << Uint16(static_cast<uint16_t>(Planner::max_acceleration_mm_per_s2[Y_AXIS]))
          << Uint16(static_cast<uint16_t>(Planner::max_acceleration_mm_per_s2[Z_AXIS]))
          << Uint16(static_cast<uint16_t>(Planner::max_acceleration_mm_per_s2[E_AXIS]))
          << Uint16(static_cast<uint16_t>(Planner::acceleration))
          << Uint16(static_cast<uint16_t>(Planner::retract_acceleration))
          << Uint16(static_cast<uint16_t>(Planner::travel_acceleration));
    frame.send();

    return Page::AccelerationSettings;
}

//! Save the Acceleration settings
void AccelerationSettings::do_save()
{
    ReadRamData response{Variable::Value0, 7};
    if(!response.send_and_receive())
    {
        Log::error() << F("Receiving Frame (Acceleration Settings)") << Log::endl();
        return;
    }

    Uint16 x, y, z, e, print, retract, travel;
    response >> x >> y >> z >> e >> print >> retract >> travel;

    Planner::max_acceleration_mm_per_s2[X_AXIS] = static_cast<uint32_t>(x.word);
    Planner::max_acceleration_mm_per_s2[Y_AXIS] = static_cast<uint32_t>(y.word);
    Planner::max_acceleration_mm_per_s2[Z_AXIS] = static_cast<uint32_t>(z.word);
    Planner::max_acceleration_mm_per_s2[E_AXIS] = static_cast<uint32_t>(e.word);
    Planner::acceleration                       = static_cast<float>(print.word);
    Planner::retract_acceleration               = static_cast<float>(retract.word);
    Planner::travel_acceleration                = static_cast<float>(travel.word);
}

// --------------------------------------------------------------------
// JerkSettings
// --------------------------------------------------------------------

//! Initialize temporary Jerk settings.
void JerkSettings::do_backup()
{
    backup_max_jerk_[X_AXIS] = Planner::max_jerk[X_AXIS];
    backup_max_jerk_[Y_AXIS] = Planner::max_jerk[Y_AXIS];
    backup_max_jerk_[Z_AXIS] = Planner::max_jerk[Z_AXIS];
    backup_max_jerk_[E_AXIS] = Planner::max_jerk[E_AXIS];
}

//! Save temporary Jerk settings.
void JerkSettings::do_rollback()
{
    Planner::max_jerk[X_AXIS] = backup_max_jerk_[X_AXIS];
    Planner::max_jerk[Y_AXIS] = backup_max_jerk_[Y_AXIS];
    Planner::max_jerk[Z_AXIS] = backup_max_jerk_[Z_AXIS];
    Planner::max_jerk[E_AXIS] = backup_max_jerk_[E_AXIS];

    advi3pp.save_settings();
}

//! Show the Jerk settings
Page JerkSettings::do_show()
{
    WriteRamDataRequest frame{Variable::Value0};
    frame << Uint16(Planner::max_jerk[X_AXIS] * 10)
          << Uint16(Planner::max_jerk[Y_AXIS] * 10)
          << Uint16(Planner::max_jerk[Z_AXIS] * 10)
          << Uint16(Planner::max_jerk[E_AXIS] * 10);
    frame.send();

    return Page::JerkSettings;
}

//! Save the Jerk settings
void JerkSettings::do_save()
{
    ReadRamData response{Variable::Value0, 4};
    if(!response.send_and_receive())
    {
        Log::error() << F("Receiving Frame (Acceleration Settings)") << Log::endl();
        return;
    }

    Uint16 x, y, z, e;
    response >> x >> y >> z >> e;

    Planner::max_jerk[X_AXIS] = x.word / 10.0;
    Planner::max_jerk[Y_AXIS] = y.word / 10.0;
    Planner::max_jerk[Z_AXIS] = z.word / 10.0;
    Planner::max_jerk[E_AXIS] = e.word / 10.0;
}

// --------------------------------------------------------------------
// Dimming
// --------------------------------------------------------------------

void ADVi3pp_::set_brightness(int16_t brightness)
{
    dimming.change_brightness(brightness);
}

Dimming::Dimming()
{
    set_next_checking_time();
    set_next_dimmming_time();
}

void Dimming::enable(bool enable)
{
    enabled_ = enable;
    reset();
}

void Dimming::set_next_checking_time()
{
    next_check_time_ = millis() + 200;
}

void Dimming::set_next_dimmming_time()
{
    next_dimming_time_ = millis() + 1000 * DIMMING_DELAY;
}

uint8_t Dimming::get_adjusted_brightness()
{
    uint16_t brightness = ::lcd_contrast;
    if(dimming_)
        brightness = brightness * DIMMING_RATIO / 100;
    if(brightness < BRIGHTNESS_MIN)
        brightness = BRIGHTNESS_MIN;
    if(brightness > BRIGHTNESS_MAX)
        brightness = BRIGHTNESS_MAX;
    return static_cast<uint8_t>(brightness);
}

void Dimming::check()
{
    if(!enabled_ || !ELAPSED(millis(), next_check_time_))
        return;
    set_next_checking_time();

    ReadRegister read{Register::TouchPanelFlag, 1};
    if(!read.send_and_receive(false))
    {
        Log::error() << F("Reading TouchPanelFlag") << Log::endl();
        return;
    }

    Uint8 value; read >> value;
    if(value.byte == 0x5A)
	{
		WriteRegisterDataRequest request{Register::TouchPanelFlag};
		request << 00_u8;
		request.send();
        reset();
	}
    else if(!dimming_ && ELAPSED(millis(), next_dimming_time_))
    {
        dimming_ = true;
        send_brightness();
    }
}

void Dimming::reset()
{
    dimming_ = false;
    set_next_dimmming_time();
    send_brightness();
}

void Dimming::send_brightness()
{
    auto brightness = get_adjusted_brightness();

    WriteRegisterDataRequest frame{Register::Brightness};
    frame << Uint8{brightness};
    frame.send(true);
}

void Dimming::change_brightness(int16_t brightness)
{
    ::lcd_contrast = brightness;
    reset();
    send_brightness();
}

// --------------------------------------------------------------------
// BLTouch
// --------------------------------------------------------------------

#ifdef ADVi3PP_BLTOUCH

void SensorSettings::send_z_height_to_lcd(double z_height)
{
    WriteRamDataRequest frame{Variable::Value0};
    frame << Uint16(z_height * 100);
    frame.send();
}

void SensorSettings::save_lcd_z_height()
{
    ReadRamData response{Variable::Value0, 1};
    if(!response.send_and_receive())
    {
        Log::error() << F("Receiving Frame (Sensor Settings)") << Log::endl();
        return;
    }

    Uint16 offsetZ; response >> offsetZ;
    save_z_height(static_cast<int16_t>(offsetZ.word) / 100.0);
}

void SensorSettings::save_z_height(double height)
{
    String command; command << F("M851 Z") << height;
    enqueue_and_echo_command(command.c_str());
    advi3pp.save_settings();
}

#endif

// --------------------------------------------------------------------
// Advance pause
// --------------------------------------------------------------------

void ADVi3pp_::advanced_pause_show_message(const AdvancedPauseMessage message)
{
    pause.advanced_pause_show_message(message);
}

void AdvancedPause::advanced_pause_show_message(const AdvancedPauseMessage message)
{
    if(message == last_advanced_pause_message_)
        return;
    last_advanced_pause_message_ = message;

    switch (message)
    {
        case ADVANCED_PAUSE_MESSAGE_INIT:                       init(); break;
        case ADVANCED_PAUSE_MESSAGE_UNLOAD:                     advi3pp.set_status(F("Unloading filament...")); break;
        case ADVANCED_PAUSE_MESSAGE_INSERT:                     insert_filament(); break;
        case ADVANCED_PAUSE_MESSAGE_EXTRUDE:                    advi3pp.set_status(F("Extruding some filament...")); break;
        case ADVANCED_PAUSE_MESSAGE_CLICK_TO_HEAT_NOZZLE:       advi3pp.set_status(F("Press continue to heat")); break;
        case ADVANCED_PAUSE_MESSAGE_RESUME:                     advi3pp.set_status(F("Resuming print...")); break;
        case ADVANCED_PAUSE_MESSAGE_STATUS:                     printing(); break;
        case ADVANCED_PAUSE_MESSAGE_WAIT_FOR_NOZZLES_TO_HEAT:   advi3pp.set_status(F("Waiting for heat...")); break;
        case ADVANCED_PAUSE_MESSAGE_OPTION:                     advanced_pause_menu_response = ADVANCED_PAUSE_RESPONSE_RESUME_PRINT; break;
        default: advi3pp::Log::log() << F("Unknown AdvancedPauseMessage: ") << static_cast<uint16_t>(message) << advi3pp::Log::endl(); break;
    }
}

void AdvancedPause::init()
{
    pages.save_forward_page();
    wait.show(F("Pausing..."));
}

void AdvancedPause::insert_filament()
{
    wait.show_continue(F("Insert filament and press continue..."),
        WaitCalllback(this, &AdvancedPause::filament_inserted), false);
}

void AdvancedPause::filament_inserted()
{
    ::wait_for_user = false;
    wait.show(F("Filament inserted.."), false);
}

void AdvancedPause::printing()
{
    advi3pp.set_status(F("Printing"));
    pages.show_forward_page();
}

// --------------------------------------------------------------------
// EEPROM data mismatch
// --------------------------------------------------------------------

Page EepromMismatch::do_show()
{
    return Page::EEPROMMismatch;
}

void EepromMismatch::do_save()
{
    advi3pp.save_settings();
    pages.show_page(Page::Main);
}

bool EepromMismatch::does_mismatch() const
{
    return mismatch_;
}

void EepromMismatch::set_mismatch()
{
    mismatch_ = true;
}

void EepromMismatch::reset_mismatch()
{
    mismatch_ = false;
}

// --------------------------------------------------------------------
// Linear Advance Tuning
// --------------------------------------------------------------------

Page LinearAdvanceTuning::do_show()
{
    return Page::LinearAdvanceTuning;
}

// --------------------------------------------------------------------
// Linear Advance Settings
// --------------------------------------------------------------------

Page LinearAdvanceSettings::do_show()
{
    return Page::LinearAdvanceSettings;
}

// --------------------------------------------------------------------
// Diagnosis
// --------------------------------------------------------------------

Page Diagnosis::do_show()
{
    // TODO
    return Page::None;
}

// --------------------------------------------------------------------

}
