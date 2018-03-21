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
#include "stepper.h"
#include "gcode.h"

#include "advi3pp.h"
#include "advi3pp_utils.h"
#include "advi3pp_.h"

#ifdef DEBUG
#pragma message "This is a DEBUG build"
#endif

#ifdef ADVi3PP_BLTOUCH
#pragma message "This is a BLTouch build"
#endif

namespace
{
    const uint16_t advi3_pp_version = 0x300;
    const uint16_t advi3_pp_oldest_lcd_compatible_version = 0x210;
    const uint16_t advi3_pp_newest_lcd_compatible_version = 0x300;
    // Modify also DETAILED_BUILD_VERSION in Version.h

    const unsigned long advi3_pp_baudrate = 115200; // Between the LCD panel and the mainboard
    const uint16_t nb_visible_sd_files = 5;
	const uint8_t  nb_visible_sd_file_chars = 48;
    const uint16_t calibration_cube_size = 20; // 20 mm
    const uint16_t calibration_extruder_filament = 100; // 10 cm
	const uint16_t calibration_extruder_delta = 20; // 2 cm

    const uint32_t usb_baudrates[] = {9600, 19200, 38400, 57600, 115200, 230400, 250000};

    const uint8_t BRIGHTNESS_MIN = 0x01;
    const uint8_t BRIGHTNESS_MAX = 0x40;
    const uint8_t DIMMING_RATIO = 25; // in percent
    const uint16_t DIMMING_DELAY = 1 * 60;
}

#ifdef ADVi3PP_BLTOUCH
bool set_probe_deployed(bool);
float run_z_probe();
extern float zprobe_zoffset;
#endif

namespace advi3pp {

inline namespace { Printer_ printer; };

// --------------------------------------------------------------------
// Printer
// --------------------------------------------------------------------

//! Initialize the printer and its LCD.
void Printer::setup()
{
    printer.setup();
}

//! Read data from the LCD and act accordingly.
void Printer::task()
{
    printer.task();
}

//! PID automatic tuning is finished.
void Printer::auto_pid_finished()
{
    printer.auto_pid_finished();
}

//! Store presets in permanent memory.
//! @param write Function to use for the actual writing
//! @param eeprom_index
//! @param working_crc
void Printer::store_presets(eeprom_write write, int& eeprom_index, uint16_t& working_crc)
{
    printer.store_eeprom_data(write, eeprom_index, working_crc);
}

//! Restore presets from permanent memory.
//! @param read Function to use for the actual reading
//! @param eeprom_index
//! @param working_crc
void Printer::restore_presets(eeprom_read read, int& eeprom_index, uint16_t& working_crc)
{
    printer.restore_eeprom_data(read, eeprom_index, working_crc);
}

//! Reset presets.
void Printer::reset_presets()
{
    printer.reset_eeprom_data();
}

//! Called when a temperature error occurred and display the error on the LCD.
void Printer::temperature_error(const char* message)
{
    printer.temperature_error(message);
}

void Printer::send_status_data()
{
    printer.send_status();
}

bool Printer::is_thermal_protection_enabled()
{
    return printer.is_thermal_protection_enabled();
}

void Printer::process_command(const GCodeParser& parser)
{
    printer.process_command(parser);
}

// --------------------------------------------------------------------
// PrinterImpl
// --------------------------------------------------------------------

Printer_::Printer_()
: sd_files_{pages_}, preheat_{pages_}, processor_{pages_, sensor_}, sensor_{pages_}
{
}

//! Initialize the printer and its LCD
void Printer_::setup()
{
#ifdef DEBUG
    Log::log() << F("This is a DEBUG build") << Log::endl();
#endif

#ifdef ADVi3PP_BLTOUCH
	 Log::log() << F("This is a BLTouch build") << Log::endl();
#endif

    if(usb_baudrate_ != BAUDRATE)
        change_usb_baudrate();

    Serial2.begin(advi3_pp_baudrate);
    get_advi3pp_lcd_version();
	pages_.show_page(is_lcd_version_valid() ? Page::Boot : Page::Mismatch, false);
    send_versions();
    clear_graphs();
    dimming_.reset();
}

//! Process command specific to this printer (I)
void Printer_::process_command(const GCodeParser& parser)
{
    processor_.process(parser);
}

//! Store presets in permanent memory.
//! @param write Function to use for the actual writing
//! @param eeprom_index
//! @param working_crc
void Printer_::store_eeprom_data(eeprom_write write, int& eeprom_index, uint16_t& working_crc)
{
    EepromWrite eeprom{write, eeprom_index, working_crc};

    preheat_.store_eeprom_data(eeprom);
    uint16_t dummy = 0; eeprom.write(dummy);
    eeprom.write(features_);
    eeprom.write(usb_baudrate_);

    dimming_.store_eeprom_data(eeprom);
}

//! Restore presets from permanent memory.
//! @param read Function to use for the actual reading
//! @param eeprom_index
//! @param working_crc
void Printer_::restore_eeprom_data(eeprom_read read, int& eeprom_index, uint16_t& working_crc)
{
    EepromRead eeprom{read, eeprom_index, working_crc};

    preheat_.restore_eeprom_data(eeprom);
    uint16_t dummy = 0; eeprom.read(dummy);
    eeprom.read(features_);
    eeprom.read(usb_baudrate_);

    dimming_.restore_eeprom_data(eeprom);
}

//! Reset presets.
void Printer_::reset_eeprom_data()
{
    preheat_.reset_eeprom_data();
    features_ = DEFAULT_FEATURES;
    dimming_.reset_eeprom_data();
}

bool Printer_::is_thermal_protection_enabled() const
{
    return (features_ & Feature::ThermalProtection) == Feature::ThermalProtection;
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Background tasks
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

//! Set the next (minimal) background task time
//! @param delta    Duration to be added to the current time to compute the next (minimal) background task time
void Printer_::set_next_background_task_time(unsigned int delta)
{
    next_op_time_ = millis() + delta;
}

//! Set the next (minimal) update time
//! @param delta    Duration to be added to the current time to compute the next (minimal) update time
void Printer_::set_next_update_time(unsigned int delta)
{
    next_update_time_ = millis() + delta;
}

//! Set the next background task and its delay
//! @param task     The next background task
//! @param delta    Duration to be added to the current time to execute the background tast
void Printer_::set_background_task(BackgroundTask task, unsigned int delta)
{
    background_task_ = task;
    set_next_background_task_time(delta);
}

//! Reset the background task
void Printer_::clear_background_task()
{
    background_task_ = BackgroundTask::None;
}

//! If there is an operating running, execute its next step
void Printer_::execute_background_task()
{
    if(!ELAPSED(millis(), next_op_time_))
        return;

    switch(background_task_)
    {
        case BackgroundTask::None:                  break;
        case BackgroundTask::ManualLeveling:        manual_leveling_task(); break;
        case BackgroundTask::LoadFilament:          load_filament_task(); break;
        case BackgroundTask::UnloadFilament:        unload_filament_task(); break;
        case BackgroundTask::ExtruderCalibration:   extruder_calibration_task(); break;
        default:                                    Log::error() << F("Invalid background task ") << static_cast<uint16_t>(background_task_) << Log::endl(); break;
    }
}


namespace
{
    //! Transform a value from a scale to another one.
    //! @param value        Value to be transformed
    //! @param valueScale   Current scale of the value (maximal)
    //! @param targetScale  Target scale
    //! @return             The scaled value
    int16_t scale(int16_t value, int16_t valueScale, int16_t targetScale) { return value * targetScale / valueScale; }
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Pages management
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

//! Show the given page on the LCD screen
//! @param [in] page The page to be displayed on the LCD screen
void PagesManager::show_page(Page page, bool save_back)
{
    Log::log() << F("Show page ") << static_cast<uint8_t>(page) << Log::endl();

    if(save_back)
        back_pages_.push(get_current_page());

    WriteRegisterDataRequest frame{Register::PictureID};
    frame << 00_u8 << page;
    frame.send(true);
}

void PagesManager::show_waiting_page(const __FlashStringHelper* message)
{
    LCD::set_status_PGM(reinterpret_cast<const char*>(message));
    show_page(Page::Waiting, true);
}

//! Retrieve the current page on the LCD screen
Page PagesManager::get_current_page()
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
void PagesManager::save_forward_page()
{
    forward_page_ = get_current_page();
}

//! Show the "Back" page on the LCD display.
void PagesManager::show_back_page()
{
    if(back_pages_.is_empty())
    {
        Log::error() << F("No Back page defined" )<< Log::endl();
        return;
    }

    show_page(back_pages_.pop(), false);
}

//! Show the "Next" page on the LCD display.
void PagesManager::show_forward_page()
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

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Incoming LCD commands and status update
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

//! Background tasks
void Printer_::task()
{
    dimming_.check();
    read_lcd_serial();
    execute_background_task();
    send_status();
    update_graphs();
}

//! Update the status of the printer on the LCD.
void Printer_::send_status()
{
    auto current_time = millis();
    if(!ELAPSED(current_time, next_update_time_))
        return;
    set_next_update_time();

    WriteRamDataRequest frame{Variable::TargetBed};
    frame << Uint16(Temperature::target_temperature_bed)
          << Uint16(Temperature::degBed())
          << Uint16(Temperature::target_temperature[0])
          << Uint16(Temperature::degHotend(0))
          << Uint16(scale(fanSpeeds[0], 255, 100))
          << Uint16(LOGICAL_Z_POSITION(current_position[Z_AXIS]))
          << FixedSizeString(LCD_::instance().get_message(), 48)
          << FixedSizeRollingString(LCD_::instance().get_message(), 52)
          << FixedSizeString(LCD_::instance().get_progress(), 48);
    frame.send(false);
}

//! Read a frame from the LCD and act accordingly.
void Printer_::read_lcd_serial()
{
    // Format of the frame (example):
    // header | length | command | action | nb words | key code
    // -------|--------|---------|--------|----------|---------
    //      2 |      1 |       1 |      2 |        1 |        2   bytes
    //  5A A5 |     06 |      83 |  04 60 |       01 |    01 50

    IncomingFrame frame;
    if(!frame.available())
        return;

    if(!frame.receive())
    {
        Log::error() << F("reading incoming Frame") << Log::endl();
        return;
    }

    Command command; Action action; Uint8 nb_words; Uint16 value;
    frame >> command >> action >> nb_words >> value;
    auto key_value = static_cast<KeyValue>(value.word);

    // TODO: Check that length == 1, that Hi(action) == 0x04
    Log::log() << F(">>> ") << nb_words.byte << F(" words, Action = ") << static_cast<uint16_t>(action) << F(", KeyValue = ") << value.word << Log::endl();

    switch(action)
    {
        case Action::Screen:                screen(key_value); break;
        case Action::SdPrintCommand:        sd_print_command(key_value); break;
        case Action::UsbPrintCommand:       usb_print_command(key_value); break;
        case Action::LoadUnload:            load_unload(key_value); break;
        case Action::Preheat:               preheat(key_value); break;
        case Action::Move:                  move(key_value); break;
        case Action::SdCard:                sd_card(key_value); break;
        case Action::FactoryReset:          factory_reset(key_value); break;
        case Action::Leveling:              leveling(key_value); break;
        case Action::ExtruderCalibration:   extruder_calibration(key_value); break;
        case Action::XYZMotorsCalibration:  xyz_motors_calibration(key_value); break;
        case Action::PidTuning:             pid_tuning(key_value); break;
        case Action::SensorSettings:        sensor_settings(key_value); break;
        case Action::Firmware:              firmware(key_value); break;
        case Action::LCD:                   lcd(key_value); break;
        case Action::LCDBrightness:         dimming_.change_brightness(key_value); break;
        case Action::Statistics:            statistics(key_value); break;
        case Action::Versions:              versions(key_value); break;
        case Action::PrintSettings:         print_settings(key_value); break;
        case Action::PIDSettings:           pid_settings(key_value); break;
        case Action::StepsSettings:         steps_settings(key_value); break;
        case Action::FeedrateSettings:      feedrate_settings(key_value); break;
        case Action::AccelerationSettings:  acceleration_settings(key_value); break;
        case Action::JerkSettings:          jerk_settings(key_value); break;
        case Action::Copyrights:            copyrights(key_value); break;
        case Action::SensorTuning:          sensor_tuning(key_value); break;
        case Action::MoveXPlus:             move_x_plus(); break;
        case Action::MoveXMinus:            move_x_minus(); break;
        case Action::MoveYPlus:             move_y_plus(); break;
        case Action::MoveYMinus:            move_y_minus(); break;
        case Action::MoveZPlus:             move_z_plus(); break;
        case Action::MoveZMinus:            move_z_minus(); break;
        case Action::MoveEPlus:             move_e_plus(); break;
        case Action::MoveEMinus:            move_e_minus(); break;
        default:                            Log::error() << F("Invalid action ") << static_cast<uint16_t>(action) << Log::endl(); break;
    }
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Main
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

void Printer_::screen(KeyValue key_value)
{
    switch(key_value)
    {
        case KeyValue::Temps:           show_temps(); break;
        case KeyValue::Print:           show_print(); break;
        case KeyValue::Controls:        show_controls(); break;
        case KeyValue::Tuning:          show_tuning(); break;
        case KeyValue::Settings:        show_settings(); break;
        case KeyValue::Infos:           show_infos(); break;
        case KeyValue::Motors:          show_motors(); break;
        case KeyValue::Back:            back(); break;
        default:                        Log::error() << F("Invalid key value ") << static_cast<uint16_t>(key_value) << Log::endl(); break;
    }
}

//! Show one of the temperature graph screens depending of the context: either the SD printing screen,
//! the printing screen or the temperature screen.
void Printer_::show_temps()
{
    set_update_graphs();

    // If there is a SD card print running, display the SD print screen
    if(card.cardOK && card.sdprinting)
    {
        pages_.show_page(Page::SdPrint);
        return;
    }

    // If there is an USB print running, display the USB Print page. Otherwise, the temperature graph page
    pages_.show_page(print_job_timer.isRunning() ? Page::UsbPrint : Page::Temperature);
}

//! Show one of the Printing screens depending of the context: either the SD screen or the SD printing screen.
//! Fallback to the USB printing if the SD card is not accessible.
void Printer_::show_print()
{
    // If there is a SD card print running, display the SD print screen
    if(card.cardOK && card.sdprinting)
    {
        pages_.show_page(Page::SdPrint);
        set_update_graphs();
        return;
    }

    if(print_job_timer.isRunning())
    {
        pages_.show_page(Page::UsbPrint);
        set_update_graphs();
        return;
    }        

    // Try to initialize the SD card
    card.initsd();
    if(!card.cardOK)
    {
        // SD card not accessible so fall back to Temperatures
        pages_.show_page(Page::Temperature);
        set_update_graphs();
        return;
    }

    sd_files_.show();
}

void Printer_::show_controls()
{
    pages_.show_page(Page::Controls);
}

void Printer_::show_tuning()
{
    pages_.show_page(Page::Tuning);
}

void Printer_::show_settings()
{
    pages_.show_page(Page::Settings);
}

void Printer_::show_infos()
{
    pages_.show_page(Page::Infos);
}

void Printer_::show_motors()
{
    pages_.show_page(Page::MotorsSettings);
}

void Printer_::back()
{
    pages_.show_back_page();
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// SD Card
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

void Printer_::sd_card(KeyValue key_value)
{
	switch(key_value)
	{
		case KeyValue::Show:				sd_files_.show(); break;
		case KeyValue::SDUp:				sd_files_.up(); break;
		case KeyValue::SDDown:				sd_files_.down(); break;
		case KeyValue::SDLine1:
		case KeyValue::SDLine2:
		case KeyValue::SDLine3:
		case KeyValue::SDLine4:
		case KeyValue::SDLine5:				sd_files_.select_file(static_cast<uint16_t>(key_value)); break;
		case KeyValue::Back:                sd_files_.back(); break;
		default:                            Log::error() << F("Invalid key value ") << static_cast<uint16_t>(key_value) << Log::endl(); break;
	}
}

SDFilesManager::SDFilesManager(PagesManager& mgr)
: pages_{mgr}
{
}

void SDFilesManager::show()
{
	if(!card.cardOK)
		return;

	nb_files_ = card.getnrfilenames();
	last_file_index_ = nb_files_ - 1;

	show_files();
	pages_.show_page(Page::SdCard);
}

void SDFilesManager::back()
{
    pages_.show_back_page();
}

void SDFilesManager::down()
{
	if(!card.cardOK)
		return;

	if(last_file_index_ >= nb_visible_sd_files)
		last_file_index_ -= nb_visible_sd_files;

	show_files();
}

void SDFilesManager::up()
{
	if(!card.cardOK)
		return;

	if(last_file_index_ + nb_visible_sd_files < nb_files_)
		last_file_index_ += nb_visible_sd_files;

	show_files();
}

//! Show the list of files on SD.
void SDFilesManager::show_files()
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
void SDFilesManager::get_file_name(uint8_t index_in_page, String& name)
{
	if(last_file_index_ >= index_in_page)
	{
		card.getfilename(last_file_index_ - index_in_page);
		name = (card.longFilename[0] == 0) ? card.filename : card.longFilename;
	}
	else
		name = "";
};

//! Select a filename as sent by the LCD screen.
//! @param file_index    The index of the filename to select
void SDFilesManager::select_file(uint16_t file_index)
{
    if(!card.cardOK)
        return;

    if(file_index > last_file_index_)
        return;
    card.getfilename(last_file_index_ - file_index);
    FixedSizeString name{card.longFilename, 26};
    if(name.length() <= 0) // If the SD card is not readable
        return;

    LCD_::instance().set_progress_name(card.longFilename);

    WriteRamDataRequest frame{Variable::CurrentFileName};
    frame << name;
    frame.send(true);

    card.openFile(card.filename, true);
    card.startFileprint();
    print_job_timer.start();

    Stepper::finish_and_disable(); // To circumvent homing problems

    pages_.show_page(Page::SdPrint);
    //set_update_graphs();
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// SD card printing commands
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

//! Handle print commands.
//! @param key_value    The sub-action to handle
void Printer_::sd_print_command(KeyValue key_value)
{
    switch(key_value)
    {
        case KeyValue::PrintStop:           sd_print_stop(); break;
        case KeyValue::PrintPause:          sd_print_pause(); break;
        case KeyValue::PrintResume:         sd_print_resume(); break;
        case KeyValue::Back:                sd_print_back(); break;
        default:                            Log::error() << F("Invalid key value ") << static_cast<uint16_t>(key_value) << Log::endl(); break;
    }
}

//! Stop SD printing
void Printer_::sd_print_stop()
{
    Log::log() << F("Stop Print") << Log::endl();

    LCD_::instance().reset_progress();

    card.stopSDPrint();
    clear_command_queue();
    quickstop_stepper();
    print_job_timer.stop();
    Temperature::disable_all_heaters();

    pages_.show_back_page();
}

//! Pause SD printing
void Printer_::sd_print_pause()
{
    Log::log() << F("Pause Print") << Log::endl();

    LCD::queue_message(F("Pause printing..."));
    card.pauseSDPrint();
    print_job_timer.pause();
#if ENABLED(PARK_HEAD_ON_PAUSE)
    enqueue_and_echo_commands_P(PSTR("M125"));
#endif
}

//! Resume the current SD printing
void Printer_::sd_print_resume()
{
    Log::log() << F("Resume Print") << Log::endl();

    LCD::queue_message(F("Resume printing"));
#if ENABLED(PARK_HEAD_ON_PAUSE)
    enqueue_and_echo_commands_P(PSTR("M24"));
#else
    card.startFileprint();
    print_job_timer.start();
#endif
}

//! Handle the Back button
void Printer_::sd_print_back()
{
    pages_.show_back_page();
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// USB printing commands
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

//! Handle print commands.
//! @param key_value    The sub-action to handle
void Printer_::usb_print_command(KeyValue key_value)
{
    switch(key_value)
    {
        case KeyValue::PrintStop:           usb_print_stop(); break;
        case KeyValue::PrintPause:          usb_print_pause(); break;
        case KeyValue::PrintResume:         usb_print_resume(); break;
        case KeyValue::Back:                usb_print_back(); break;
        default:                            Log::error() << F("Invalid key value ") << static_cast<uint16_t>(key_value) << Log::endl(); break;
    }
}

//! Stop SD printing
void Printer_::usb_print_stop()
{
    Log::log() << F("Stop Print") << Log::endl();

    LCD_::instance().reset_progress();

    clear_command_queue();
    quickstop_stepper();
    print_job_timer.stop();
    Temperature::disable_all_heaters();

    pages_.show_back_page();
}

//! Pause SD printing
void Printer_::usb_print_pause()
{
    Log::log() << F("Pause Print") << Log::endl();

    LCD::queue_message(F("Pause printing"));

    print_job_timer.pause();
#if ENABLED(PARK_HEAD_ON_PAUSE)
    enqueue_and_echo_commands_P(PSTR("M125"));
#endif
}

//! Resume the current SD printing
void Printer_::usb_print_resume()
{
    Log::log() << F("Resume Print") << Log::endl();

    LCD::queue_message(F("Resume printing"));

#if ENABLED(PARK_HEAD_ON_PAUSE)
    enqueue_and_echo_commands_P(PSTR("M24"));
#else
    print_job_timer.start();
#endif
}

//! Handle the Back button
void Printer_::usb_print_back()
{
    pages_.show_back_page();
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Target temperature
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

//! Set the target temperature on the LCD screens
//! @param temperature  The temperature to set
void Printer_::set_target_temperature(uint16_t temperature)
{
    WriteRamDataRequest frame{Variable::TargetTemperature};
    frame << Uint16(temperature);
    frame.send();
}

//! Get the target temperature set on the LCD screen
//! @return     The temperature
uint16_t Printer_::Printer_::get_target_temperature()
{
    ReadRamData frame{Variable::TargetTemperature, 1};
    if(!frame.send_and_receive())
    {
        Log::error() << F("Receiving Frame (Target Temperature)") << Log::endl();
        return 0;
    }

    Uint16 hotend; frame >> hotend;
    return hotend.word;
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Load and Unload Filament
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

//! Handle Load & Unload actions.
//! @param key_value    The sub-action to handle
void Printer_::load_unload(KeyValue key_value)
{
    switch(key_value)
    {
        case KeyValue::Show:                load_unload_show(); break;
        case KeyValue::Load:                load_unload_start(true); break;
        case KeyValue::Unload:              load_unload_start(false); break;
        case KeyValue::Back:                load_unload_stop(); break;
        default:                            Log::error() << F("Invalid key value ") << static_cast<uint16_t>(key_value) << Log::endl(); break;
    }
}

//! Show the Load & Unload screen on the LCD display.
void Printer_::load_unload_show()
{
    set_target_temperature(200);
    pages_.show_page(Page::LoadUnload);
}

//! Start Load or Unload action.
//! @param load    Which action to start (i.e. which screen to display)
void Printer_::load_unload_start(bool load)
{
    auto hotend = get_target_temperature();
    if(hotend <= 0)
        return;

    Temperature::setTargetHotend(hotend, 0);
    enqueue_and_echo_commands_P(PSTR("G91")); // relative mode

    set_background_task(load ? BackgroundTask::LoadFilament : BackgroundTask::UnloadFilament);
    pages_.show_page(load ? Page::Load2 : Page::Unload2);
}

//! Handle back from the Load on Unload LCD screen.
void Printer_::load_unload_stop()
{
    Log::log() << F("Load/Unload Stop");

    clear_background_task();
    clear_command_queue();
    enqueue_and_echo_commands_P(PSTR("G90")); // absolute mode
    Temperature::setTargetHotend(0, 0);

    pages_.show_back_page();
}

//! Load the filament if the temperature is high enough.
void Printer_::load_filament_task()
{
    if(Temperature::current_temperature[0] >= Temperature::target_temperature[0] - 10)
    {
        Log::log() << F("Load Filament") << Log::endl();
        enqueue_and_echo_commands_P(PSTR("G1 E1 F120"));
    }
    set_next_background_task_time();
}

//! Unload the filament if the temperature is high enough.
void Printer_::unload_filament_task()
{
    if(Temperature::current_temperature[0] >= Temperature::target_temperature[0] - 10)
    {
        Log::log() << F("Unload Filament") << Log::endl();
        enqueue_and_echo_commands_P(PSTR("G1 E-1 F120"));
    }
    set_next_background_task_time();
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Preheat & Cooldown
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

//! Handle Preheat actions.
//! @param key_value    Sub-action to handle
void Printer_::preheat(KeyValue key_value)
{
    switch(key_value)
    {
        case KeyValue::Show:            preheat_.show(); break;
        case KeyValue::Back:            preheat_.back(); break;
        case KeyValue::Preset1:
        case KeyValue::Preset2:
        case KeyValue::Preset3:         preheat_.preset(static_cast<uint16_t>(key_value)); break;
        case KeyValue::Cooldown:        cooldown(); break;
        default:                        Log::error() << F("Invalid key value ") << static_cast<uint16_t>(key_value) << Log::endl(); break;
    }
}

//! Cooldown the bed and the nozzle
void Printer_::cooldown()
{
    Log::log() << F("Cooldown") << Log::endl();
    LCD::reset_message();
    Temperature::disable_all_heaters();
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Move & Home
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

//! Execute a move command
void Printer_::move(KeyValue key_value)
{
    switch(key_value)
    {
        case KeyValue::Show:                show_move(); break;
        case KeyValue::MoveXHome:           move_x_home(); break;
        case KeyValue::MoveYHome:           move_y_home(); break;
        case KeyValue::MoveZHome:           move_z_home(); break;
        case KeyValue::MoveAllHome:         move_all_home(); break;
        case KeyValue::MoveAllDisable:      disable_motors(); break;
        case KeyValue::Back:                move_back(); break;
        default:                            Log::error() << F("Invalid key value ") << static_cast<uint16_t>(key_value) << Log::endl(); break;
    }
}

void Printer_::show_move()
{
    Stepper::finish_and_disable(); // To circumvent homing problems
    pages_.show_page(Page::Move);
}

void Printer_::move_back()
{
    pages_.show_back_page();
}

//! Move the nozzle.
void Printer_::move_x_plus()
{
    clear_command_queue();
    enqueue_and_echo_commands_P(PSTR("G91"));
    enqueue_and_echo_commands_P(PSTR("G1 X5 F3000"));
    enqueue_and_echo_commands_P(PSTR("G90"));
}

//! Move the nozzle.
void Printer_::move_x_minus()
{
    clear_command_queue();
    enqueue_and_echo_commands_P(PSTR("G91"));
    enqueue_and_echo_commands_P(PSTR("G1 X-5 F3000"));
    enqueue_and_echo_commands_P(PSTR("G90"));
}

//! Move the nozzle.
void Printer_::move_y_plus()
{
    clear_command_queue();
    enqueue_and_echo_commands_P(PSTR("G91"));
    enqueue_and_echo_commands_P(PSTR("G1 Y5 F3000"));
    enqueue_and_echo_commands_P(PSTR("G90"));
}

//! Move the nozzle.
void Printer_::move_y_minus()
{
    clear_command_queue();
    enqueue_and_echo_commands_P(PSTR("G91"));
    enqueue_and_echo_commands_P(PSTR("G1 Y-5 F3000"));
    enqueue_and_echo_commands_P(PSTR("G90"));
}

//! Move the nozzle.
void Printer_::move_z_plus()
{
    clear_command_queue();
    enqueue_and_echo_commands_P(PSTR("G91"));
    enqueue_and_echo_commands_P(PSTR("G1 Z0.5 F3000"));
    enqueue_and_echo_commands_P(PSTR("G90"));
}

//! Move the nozzle.
void Printer_::move_z_minus()
{
    clear_command_queue();
    enqueue_and_echo_commands_P(PSTR("G91"));
    enqueue_and_echo_commands_P(PSTR("G1 Z-0.5 F3000"));
    enqueue_and_echo_commands_P(PSTR("G90"));
}

//! Extrude some filament.
void Printer_::move_e_plus()
{
    if(Temperature::degHotend(0) < 180)
        return;

    clear_command_queue();
    enqueue_and_echo_commands_P(PSTR("G91"));
    enqueue_and_echo_commands_P(PSTR("G1 E1 F120"));
    enqueue_and_echo_commands_P(PSTR("G90"));
}

//! Unextrude.
void Printer_::move_e_minus()
{
    if(Temperature::degHotend(0) < 180)
        return;

    clear_command_queue();
    enqueue_and_echo_commands_P(PSTR("G91"));
    enqueue_and_echo_commands_P(PSTR("G1 E-1 F120"));
    enqueue_and_echo_commands_P(PSTR("G90"));
}

//! Disable the motors.
void Printer_::disable_motors()
{
    enqueue_and_echo_commands_P(PSTR("M84"));
    axis_homed[X_AXIS] = axis_homed[Y_AXIS] = axis_homed[Z_AXIS] = false;
    axis_known_position[X_AXIS] = axis_known_position[Y_AXIS] = axis_known_position[Z_AXIS] = false;
}

//! Go to home on the X axis.
void Printer_::move_x_home()
{
    enqueue_and_echo_commands_P(PSTR("G28 X0"));
}

//! Go to home on the Y axis.
void Printer_::move_y_home()
{
    enqueue_and_echo_commands_P(PSTR("G28 Y0"));
}

//! Go to home on the Z axis.
void Printer_::move_z_home()
{
    enqueue_and_echo_commands_P(PSTR("G28 Z0"));
}

//! Go to home on all axis.
void Printer_::move_all_home()
{
    enqueue_and_echo_commands_P(PSTR("G28"));
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Settings
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

void Printer_::print_settings(advi3pp::KeyValue key_value)
{
    switch(key_value)
    {
        case KeyValue::Show: print_settings_show(); break;
        case KeyValue::Save: print_settings_save(); break;
        case KeyValue::Back: print_settings_cancel(); break;
        default:                Log::error() << F("Invalid key value ") << static_cast<uint16_t>(key_value) << Log::endl(); break;
    }
}

void Printer_::pid_settings(advi3pp::KeyValue key_value)
{
    switch(key_value)
    {
        case KeyValue::Show: pid_settings_show(true); break;
        case KeyValue::Save: pid_settings_save(); break;
        case KeyValue::Back: pid_settings_cancel(); break;
        default:                Log::error() << F("Invalid key value ") << static_cast<uint16_t>(key_value) << Log::endl(); break;
    }
}

void Printer_::steps_settings(advi3pp::KeyValue key_value)
{
    switch(key_value)
    {
        case KeyValue::Show: steps_settings_show(); break;
        case KeyValue::Save: steps_settings_save(); break;
        case KeyValue::Back: steps_settings_cancel(); break;
        default:                Log::error() << F("Invalid key value ") << static_cast<uint16_t>(key_value) << Log::endl(); break;
    }
}

void Printer_::feedrate_settings(advi3pp::KeyValue key_value)
{
    switch(key_value)
    {
        case KeyValue::Show: feedrate_settings_show(); break;
        case KeyValue::Save: feedrate_settings_save(); break;
        case KeyValue::Back: feedrate_settings_cancel(); break;
        default:                Log::error() << F("Invalid key value ") << static_cast<uint16_t>(key_value) << Log::endl(); break;
    }
}

void Printer_::acceleration_settings(advi3pp::KeyValue key_value)
{
    switch(key_value)
    {
        case KeyValue::Show: acceleration_settings_show(); break;
        case KeyValue::Save: acceleration_settings_save(); break;
        case KeyValue::Back: acceleration_settings_cancel(); break;
        default:                Log::error() << F("Invalid key value ") << static_cast<uint16_t>(key_value) << Log::endl(); break;
    }
}

void Printer_::jerk_settings(advi3pp::KeyValue key_value)
{
    switch(key_value)
    {
        case KeyValue::Show: jerk_settings_show(); break;
        case KeyValue::Save: jerk_settings_save(); break;
        case KeyValue::Back: jerk_settings_cancel(); break;
        default:                Log::error() << F("Invalid key value ") << static_cast<uint16_t>(key_value) << Log::endl(); break;
    }
}

//! Display on the LCD screen the printing settings.
void Printer_::print_settings_show()
{
    WriteRamDataRequest frame{Variable::PrintSettingsSpeed};
    frame << Uint16(feedrate_percentage)
          << Uint16(Temperature::degTargetHotend(0))
          << Uint16(Temperature::degTargetBed())
          << Uint16(scale(fanSpeeds[0], 255, 100));
    frame.send();

    pages_.save_forward_page();
    pages_.show_page(Page::PrintSettings);
}

//! Save the printing settings.
void Printer_::print_settings_save()
{
    ReadRamData response{Variable::PrintSettingsSpeed, 4};
    if(!response.send_and_receive())
    {
        Log::error() << F("Receiving Frame (Print Settings)") << Log::endl();
        return;
    }

    Uint16 speed, hotend, bed, fan;
    response >> speed >> hotend >> bed >> fan;

    feedrate_percentage = speed.word;
    Temperature::setTargetHotend(hotend.word, 0);
    Temperature::setTargetBed(bed.word);
    fanSpeeds[0] = scale(fan.word, 100, 255);

    pages_.show_forward_page();
}

void Printer_::print_settings_cancel()
{
    pages_.show_back_page();
}

//! Show the PID settings
void Printer_::pid_settings_show(bool back, bool init)
{
    if(init)
    {
        old_pid_.init();
        pages_.save_forward_page();
    }        

    WriteRamDataRequest frame{Variable::PidP};
    frame << Uint16(Temperature::Kp * 100)
          << Uint16(unscalePID_i(Temperature::Ki) * 100)
          << Uint16(unscalePID_d(Temperature::Kd) * 100);
    frame.send();

    pages_.show_page(Page::PidSettings, back);
}

//! Save the PID settings
void Printer_::pid_settings_save()
{
    ReadRamData response{Variable::PidP, 3};
    if(!response.send_and_receive())
    {
        Log::error() << F("Receiving Frame (PID Settings)") << Log::endl();
        return;
    }

    Uint16 p, i, d;
    response >> p >> i >> d;

    Temperature::Kp = static_cast<float>(p.word) / 100;
    Temperature::Ki = scalePID_i(static_cast<float>(i.word) / 100);
    Temperature::Kd = scalePID_d(static_cast<float>(d.word) / 100);

    enqueue_and_echo_commands_P(PSTR("M500"));

    pages_.show_forward_page();
}

void Printer_::pid_settings_cancel()
{
    old_pid_.save(); // Restore old values
    pages_.show_back_page();
}

//! Show the Steps settings
//! @param init     Initialize the settings are use those already set
void Printer_::steps_settings_show(bool init)
{
    if(init)
    {
        steps_.init();
        pages_.save_forward_page();
    }        

    WriteRamDataRequest frame{Variable::StepSettingsX};
    frame << Uint16(steps_.axis_steps_per_mm[X_AXIS] * 10)
          << Uint16(steps_.axis_steps_per_mm[Y_AXIS] * 10)
          << Uint16(steps_.axis_steps_per_mm[Z_AXIS] * 10)
          << Uint16(steps_.axis_steps_per_mm[E_AXIS] * 10);
    frame.send();

    pages_.show_page(Page::StepsSettings);
}

//! Save the Steps settings
void Printer_::steps_settings_save()
{
    ReadRamData response{Variable::StepSettingsX, 4};
    if(!response.send_and_receive())
    {
        Log::error() << F("Receiving Frame (Steps Settings)") << Log::endl();
        return;
    }

    Uint16 x, y, z, e;
    response >> x >> y >> z >> e;

    steps_.axis_steps_per_mm[X_AXIS] = static_cast<float>(x.word) / 10;
    steps_.axis_steps_per_mm[Y_AXIS] = static_cast<float>(y.word) / 10;
    steps_.axis_steps_per_mm[Z_AXIS] = static_cast<float>(z.word) / 10;
    steps_.axis_steps_per_mm[E_AXIS] = static_cast<float>(e.word) / 10;
    steps_.save();

    pages_.show_forward_page();
}

void Printer_::steps_settings_cancel()
{
    pages_.show_back_page();
}

//! Show the Feedrate settings
//! @param init     Initialize the settings are use those already set
void Printer_::feedrate_settings_show(bool init)
{
    if(init)
    {    
        feedrates_.init();
        pages_.save_forward_page();
    }        

    WriteRamDataRequest frame{Variable::FeedrateMaxX};
    frame << Uint16(feedrates_.max_feedrate_mm_s[X_AXIS])
          << Uint16(feedrates_.max_feedrate_mm_s[Y_AXIS])
          << Uint16(feedrates_.max_feedrate_mm_s[Z_AXIS])
          << Uint16(feedrates_.max_feedrate_mm_s[E_AXIS])
          << Uint16(feedrates_.min_feedrate_mm_s)
          << Uint16(feedrates_.min_travel_feedrate_mm_s);
    frame.send();

    pages_.show_page(Page::FeedrateSettings);
}

//! Save the Feedrate settings
void Printer_::feedrate_settings_save()
{
    ReadRamData response{Variable::FeedrateMaxX, 6};
    if(!response.send_and_receive())
    {
        Log::error() << F("Receiving Frame (Feedrate Settings)") << Log::endl();
        return;
    }

    Uint16 x, y, z, e, min, travel;
    response >> x >> y >> z >> e >> min >> travel;

    feedrates_.max_feedrate_mm_s[X_AXIS] = static_cast<float>(x.word);
    feedrates_.max_feedrate_mm_s[Y_AXIS] = static_cast<float>(y.word);
    feedrates_.max_feedrate_mm_s[Z_AXIS] = static_cast<float>(z.word);
    feedrates_.max_feedrate_mm_s[E_AXIS] = static_cast<float>(e.word);
    feedrates_.min_feedrate_mm_s         = static_cast<float>(min.word);
    feedrates_.min_travel_feedrate_mm_s  = static_cast<float>(travel.word);
    feedrates_.save();

    pages_.show_forward_page();
}

void Printer_::feedrate_settings_cancel()
{
    pages_.show_back_page();
}

//! Show the Acceleration settings
//! @param init     Initialize the settings are use those already set
void Printer_::acceleration_settings_show(bool init)
{
    if(init)
    {
        accelerations_.init();
        pages_.save_forward_page();
    }        

    WriteRamDataRequest frame{Variable::AccelerationMaxX};
    frame << Uint16(static_cast<uint16_t>(accelerations_.max_acceleration_mm_per_s2[X_AXIS]))
          << Uint16(static_cast<uint16_t>(accelerations_.max_acceleration_mm_per_s2[Y_AXIS]))
          << Uint16(static_cast<uint16_t>(accelerations_.max_acceleration_mm_per_s2[Z_AXIS]))
          << Uint16(static_cast<uint16_t>(accelerations_.max_acceleration_mm_per_s2[E_AXIS]))
          << Uint16(static_cast<uint16_t>(accelerations_.acceleration))
          << Uint16(static_cast<uint16_t>(accelerations_.retract_acceleration))
          << Uint16(static_cast<uint16_t>(accelerations_.travel_acceleration));
    frame.send();

    pages_.show_page(Page::AccelerationSettings);
}

//! Save the Acceleration settings
void Printer_::acceleration_settings_save()
{
    ReadRamData response{Variable::AccelerationMaxX, 7};
    if(!response.send_and_receive())
    {
        Log::error() << F("Receiving Frame (Acceleration Settings)") << Log::endl();
        return;
    }

    Uint16 x, y, z, e, print, retract, travel;
    response >> x >> y >> z >> e >> print >> retract >> travel;

    accelerations_.max_acceleration_mm_per_s2[X_AXIS] = static_cast<uint32_t>(x.word);
    accelerations_.max_acceleration_mm_per_s2[Y_AXIS] = static_cast<uint32_t>(y.word);
    accelerations_.max_acceleration_mm_per_s2[Z_AXIS] = static_cast<uint32_t>(z.word);
    accelerations_.max_acceleration_mm_per_s2[E_AXIS] = static_cast<uint32_t>(e.word);
    accelerations_.acceleration                       = static_cast<float>(print.word);
    accelerations_.retract_acceleration               = static_cast<float>(retract.word);
    accelerations_.travel_acceleration                = static_cast<float>(travel.word);
    accelerations_.save();

    pages_.show_forward_page();
}

void Printer_::acceleration_settings_cancel()
{
    pages_.show_back_page();
}

//! Show the Jerk settings
//! @param init     Initialize the settings are use those already set
void Printer_::jerk_settings_show(bool init)
{
    if(init)
    {
        jerks_.init();
        pages_.save_forward_page();
    }        

    WriteRamDataRequest frame{Variable::JerkX};
    frame << Uint16(jerks_.max_jerk[X_AXIS] * 10)
          << Uint16(jerks_.max_jerk[Y_AXIS] * 10)
          << Uint16(jerks_.max_jerk[Z_AXIS] * 10)
          << Uint16(jerks_.max_jerk[E_AXIS] * 10);
    frame.send();

    pages_.show_page(Page::JerkSettings);
}

//! Save the Jerk settings
void Printer_::jerk_settings_save()
{
    ReadRamData response{Variable::JerkX, 4};
    if(!response.send_and_receive())
    {
        Log::error() << F("Receiving Frame (Acceleration Settings)") << Log::endl();
        return;
    }

    Uint16 x, y, z, e;
    response >> x >> y >> z >> e;

    jerks_.max_jerk[X_AXIS] = static_cast<uint32_t>(x.word / 10);
    jerks_.max_jerk[Y_AXIS] = static_cast<uint32_t>(y.word / 10);
    jerks_.max_jerk[Z_AXIS] = static_cast<uint32_t>(z.word / 10);
    jerks_.max_jerk[E_AXIS] = static_cast<uint32_t>(e.word / 10);
    jerks_.save();

    pages_.show_forward_page();
}

void Printer_::jerk_settings_cancel()
{
    pages_.show_back_page();
}

//! Reset all settings of the printer to factory ones.
void Printer_::factory_reset(KeyValue key_value)
{
    switch(key_value)
    {
        case KeyValue::Show:               show_factory_reset_warning(); break;
        case KeyValue::ResetConfirm:       do_factory_reset(); break;
        case KeyValue::Back:               cancel_factory_reset(); break;
        default:                           Log::error() << F("Invalid key value ") << static_cast<uint16_t>(key_value) << Log::endl(); break;
    }
}

void Printer_::show_factory_reset_warning()
{
    pages_.show_page(Page::FactoryReset);
}

void Printer_::do_factory_reset()
{
    enqueue_and_echo_commands_P(PSTR("M502"));
    enqueue_and_echo_commands_P(PSTR("M500"));
    pages_.show_back_page();
}

void Printer_::cancel_factory_reset()
{
    pages_.show_back_page();
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Statistics
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

//! Display statistics on the LCD screen.
void Printer_::statistics(KeyValue key_value)
{
    switch(key_value)
    {
        case KeyValue::Show:    show_stats(); break;
        case KeyValue::Back:    stats_back(); break;
        default:                Log::error() << F("Invalid key value ") << static_cast<uint16_t>(key_value) << Log::endl(); break;
    }
}

void Printer_::show_stats()
{
    send_stats();
    pages_.show_page(Page::Statistics);
}

void Printer_::stats_back()
{
    pages_.show_back_page();
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Versions
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

void Printer_::get_advi3pp_lcd_version()
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
String Printer_::get_lcd_firmware_version()
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
void Printer_::send_versions()
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

bool Printer_::is_lcd_version_valid() const
{
    return lcd_version_ >= advi3_pp_oldest_lcd_compatible_version && lcd_version_ <= advi3_pp_newest_lcd_compatible_version;
}

//! Display the Versions screen,
void Printer_::versions(KeyValue key_value)
{
    switch(key_value)
    {
		case KeyValue::Show:					versions_show(); break;
        case KeyValue::MismatchForward:         versions_mismatch_forward(); break;
        case KeyValue::Back:                    versions_back(); break;
        default:								Log::error() << F("Invalid key value ") << static_cast<uint16_t>(key_value) << Log::endl(); break;
    }
}

void Printer_::versions_mismatch_forward()
{
    pages_.show_page(Page::Main, false);
}

void Printer_::versions_back()
{
    pages_.show_back_page();
}

void Printer_::versions_show()
{
    pages_.show_page(Page::Versions);
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Copyrights
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

//! Display the Copyrights screen,
void Printer_::copyrights(advi3pp::KeyValue key_value)
{
    switch(key_value)
    {
        case KeyValue::Show:					copyrights_show(); break;
        case KeyValue::Back:                    copyrights_back(); break;
        default:								Log::error() << F("Invalid key value ") << static_cast<uint16_t>(key_value) << Log::endl(); break;
    }
}

void Printer_::copyrights_show()
{
    pages_.show_page(Page::Copyrights);
}

void Printer_::copyrights_back()
{
    pages_.show_back_page();
}


// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// PID Tuning
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

//! Handle PID tuning.
//! @param key_value    The step of the PID tuning
void Printer_::pid_tuning(KeyValue key_value)
{
    switch(key_value)
    {
        case KeyValue::PidTuningStep1:   pid_tuning_step1(); break;
        case KeyValue::PidTuningStep2:   pid_tuning_step2(); break;
        case KeyValue::Back:             pid_tuning_cancel(); break;
        default:                         Log::error() << F("Invalid key value ") << static_cast<uint16_t>(key_value) << Log::endl(); break;
    }
}

//! Show step #1 of PID tuning
void Printer_::pid_tuning_step1()
{
    old_pid_.init();
    set_target_temperature(200);
    pages_.save_forward_page();
    pages_.show_page(Page::PidTuning1);
}

//! Show step #2 of PID tuning
void Printer_::pid_tuning_step2()
{
    auto hotend = get_target_temperature();
    if(hotend <= 0)
        return;

    enqueue_and_echo_commands_P(PSTR("M106 S255")); // Turn on fan
    String auto_pid_command; auto_pid_command << F("M303 S") << hotend << F("E0 C8 U1");
    enqueue_and_echo_command(auto_pid_command.c_str());

    pages_.show_page(Page::PidTuning2);
};

//! PID automatic tuning is finished.
void Printer_::auto_pid_finished()
{
    Log::log() << F("Auto PID finished") << Log::endl();
    enqueue_and_echo_commands_P(PSTR("M106 S0"));
    pid_settings_show(false, false);
}

void Printer_::pid_tuning_cancel()
{
    pages_.show_back_page();
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Leveling
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

void Printer_::leveling(KeyValue key_value)
{
    switch(key_value)
    {
        case KeyValue::Show:            leveling_home(); break;
        case KeyValue::LevelingPoint1:  leveling_point1(); break;
        case KeyValue::LevelingPoint2:  leveling_point2(); break;
        case KeyValue::LevelingPoint3:  leveling_point3(); break;
        case KeyValue::LevelingPoint4:  leveling_point4(); break;
        case KeyValue::LevelingPoint5:  leveling_point5(); break;
        case KeyValue::Back:            leveling_finish(); break;
        default:                        Log::error() << F("Invalid key value ") << static_cast<uint16_t>(key_value) << Log::endl(); break;
    }
}

//! Home the printer for bed leveling.
void Printer_::leveling_home()
{
    pages_.show_waiting_page(F("Homing..."));
    axis_homed[X_AXIS] = axis_homed[Y_AXIS] = axis_homed[Z_AXIS] = false;
    axis_known_position[X_AXIS] = axis_known_position[Y_AXIS] = axis_known_position[Z_AXIS] = false;
    enqueue_and_echo_commands_P(PSTR("G90")); // absolute mode
    enqueue_and_echo_commands_P((PSTR("G28"))); // homing
    set_background_task(BackgroundTask::ManualLeveling);
}

//! Leveling Background task.
void Printer_::manual_leveling_task()
{
    if(axis_homed[X_AXIS] && axis_homed[Y_AXIS] && axis_homed[Z_AXIS])
    {
        Log::log() << F("Leveling Homed, start process") << Log::endl();
        clear_background_task();
        pages_.show_page(Page::ManualLeveling, false);
    }
    else
        set_next_background_task_time(200);
}

//! Handle leveling point #1.
void Printer_::leveling_point1()
{
    Log::log() << F("Level step 1") << Log::endl();
    enqueue_and_echo_commands_P(PSTR("G1 Z10 F2000"));
    enqueue_and_echo_commands_P(PSTR("G1 X30 Y30 F6000"));
    enqueue_and_echo_commands_P(PSTR("G1 Z0 F1000"));
}

//! Handle leveling point #2.
void Printer_::leveling_point2()
{
    Log::log() << F("Level step 2") << Log::endl();
    enqueue_and_echo_commands_P(PSTR("G1 Z10 F2000"));
    enqueue_and_echo_commands_P(PSTR("G1 X170 Y170 F6000"));
    enqueue_and_echo_commands_P(PSTR("G1 Z0 F1000"));
}

//! Handle leveling point #3.
void Printer_::leveling_point3()
{
    Log::log() << F("Level step 3") << Log::endl();
    enqueue_and_echo_commands_P(PSTR("G1 Z10 F2000"));
    enqueue_and_echo_commands_P(PSTR("G1 X170 Y30 F6000"));
    enqueue_and_echo_commands_P(PSTR("G1 Z0 F1000"));
}

//! Handle leveling point #4.
void Printer_::leveling_point4()
{
    Log::log() << F("Level step 4") << Log::endl();
    enqueue_and_echo_commands_P(PSTR("G1 Z10 F2000"));
    enqueue_and_echo_commands_P(PSTR("G1 X30 Y170 F6000"));
    enqueue_and_echo_commands_P(PSTR("G1 Z0 F1000"));
}

//! Handle leveling point #5.
void Printer_::leveling_point5()
{
    Log::log() << F("Level step 5") << Log::endl();
    enqueue_and_echo_commands_P(PSTR("G1 Z10 F2000"));
    enqueue_and_echo_commands_P(PSTR("G1 X100 Y100 F6000"));
    enqueue_and_echo_commands_P(PSTR("G1 Z0 F1000"));
}

//! Handle leveling point #4.
void Printer_::leveling_finish()
{
    enqueue_and_echo_commands_P(PSTR("G1 Z30 F2000"));
    pages_.show_back_page();
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Extruder calibration
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

void Printer_::extruder_calibration(KeyValue key_value)
{
    switch(key_value)
    {
        case KeyValue::Show:                    show_extruder_calibration(); break;
        case KeyValue::CalibrationStart:        start_extruder_calibration(); break;
        case KeyValue::CalibrationSettings:     extruder_calibrartion_settings(); break;
        case KeyValue::Back:                    cancel_extruder_calibration(); break;
        default:                                Log::error() << F("Invalid key value ") << static_cast<uint16_t>(key_value) << Log::endl(); break;
    }
}

//! Show the extruder calibration screen.
void Printer_::show_extruder_calibration()
{
    set_target_temperature(200);

    WriteRamDataRequest frame{Variable::Measure1};
    frame << 200_u16;
    frame.send();

    pages_.save_forward_page();
    pages_.show_page(Page::ExtruderCalibration1);
}

//! Start extruder calibration.
void Printer_::start_extruder_calibration()
{
    auto hotend = get_target_temperature();
    if(hotend <= 0)
        return;

    Temperature::setTargetHotend(hotend, 0);
    set_background_task(BackgroundTask::ExtruderCalibration);
    enqueue_and_echo_commands_P(PSTR("M83"));       // relative E mode
    enqueue_and_echo_commands_P(PSTR("G92 E0"));    // reset E axis

    pages_.show_page(Page::ExtruderCalibration2, false);
}

//! Extruder calibration background task.
void Printer_::extruder_calibration_task()
{
    if(current_position[E_AXIS] >= calibration_extruder_filament)
    {
        extruder_calibration_finished();
        return;
    }

    if(Temperature::current_temperature[0] < Temperature::target_temperature[0] - 10)
    {
        set_next_background_task_time();
        return;
    }

    enqueue_and_echo_commands_P(PSTR("G1 E10 F50")); // Extrude 10mm slowly
    set_next_background_task_time();
}

//! Record the amount of filament extruded.
void Printer_::extruder_calibration_finished()
{
    extruded_ = current_position[E_AXIS];
    Log::log() << F("Filament extruded ") << extruded_ << Log::endl();
    enqueue_and_echo_commands_P(PSTR("G82"));       // absolute E mode
    enqueue_and_echo_commands_P(PSTR("G92 E0"));    // reset E axis

    clear_background_task();
    pages_.show_page(Page::ExtruderCalibration3);
}

//! Compute the extruder (E axis) new value and show the steps settings.
void Printer_::extruder_calibrartion_settings()
{
    ReadRamData response{Variable::Measure1, 1};
    if(!response.send_and_receive())
    {
        Log::error() << F("Receiving Frame (Measures)") << Log::endl();
        return;
    }

    Uint16 e; response >> e;
    e.word /= 10;

    // Fill all values because all 4 axis are displayed by  show_steps_settings
    steps_.axis_steps_per_mm[X_AXIS] = Planner::axis_steps_per_mm[X_AXIS];
    steps_.axis_steps_per_mm[Y_AXIS] = Planner::axis_steps_per_mm[Y_AXIS];
    steps_.axis_steps_per_mm[Z_AXIS] = Planner::axis_steps_per_mm[Z_AXIS];
	steps_.axis_steps_per_mm[E_AXIS] = Planner::axis_steps_per_mm[E_AXIS] * extruded_ / (extruded_ + calibration_extruder_delta - e.word);
    Log::log() << F("Adjust: old = ") << Planner::axis_steps_per_mm[E_AXIS] << F(", expected = ") << extruded_ << F(", measured = ") << (extruded_ + calibration_extruder_delta - e.word) << F(", new = ") << steps_.axis_steps_per_mm[E_AXIS] << Log::endl();

    steps_settings_show(false);
}

//! Cancel the extruder calibration.
void Printer_::cancel_extruder_calibration()
{
    clear_background_task();

    Temperature::setTargetHotend(0, 0);
    Temperature::setTargetBed(0);

    enqueue_and_echo_commands_P(PSTR("G82"));       // absolute E mode
    enqueue_and_echo_commands_P(PSTR("G92 E0"));    // reset E axis

    pages_.show_back_page();
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// XYZ Motors calibration
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

void Printer_::xyz_motors_calibration(KeyValue key_value)
{
    switch(key_value)
    {
        case KeyValue::Show:                    show_xyz_motors_calibration(); break;
        case KeyValue::CalibrationSettings:     xyz_motors_calibration_settings(); break;
        case KeyValue::Back:                    cancel_xyz_motors_calibration(); break;
        default:                                Log::error() << F("Invalid key value ") << static_cast<uint16_t>(key_value) << Log::endl(); break;
    }
}

void Printer_::show_xyz_motors_calibration()
{
    WriteRamDataRequest frame{Variable::Measure1};
    frame << 200_u16 << 200_u16 << 200_u16;
    frame.send();
    pages_.save_forward_page();
    pages_.show_page(Page::XYZMotorsCalibration);
}

float adjust_value(float old, double expected, double measured)
{
	auto new_value = old * expected / measured;
    Log::log() << F("Adjust: old = ") << old << F(", expected = ") << expected << F(", measured = ") << measured << F(", new = ") << new_value << Log::endl();
    return new_value;
};

void Printer_::xyz_motors_calibration_settings()
{
    ReadRamData response{Variable::Measure1, 3};
    if(!response.send_and_receive())
    {
        Log::error() << F("Receiving Frame (Measures)") << Log::endl();
        return;
    }

    Uint16 x, y, z;
    response >> x >> y >> z;

    steps_.axis_steps_per_mm[X_AXIS] = adjust_value(Planner::axis_steps_per_mm[X_AXIS], calibration_cube_size * 10, x.word);
    steps_.axis_steps_per_mm[Y_AXIS] = adjust_value(Planner::axis_steps_per_mm[Y_AXIS], calibration_cube_size * 10, y.word);
    steps_.axis_steps_per_mm[Z_AXIS] = adjust_value(Planner::axis_steps_per_mm[Z_AXIS], calibration_cube_size * 10, z.word);
    // Fill all values because all 4 axis are displayed by  show_steps_settings
    steps_.axis_steps_per_mm[E_AXIS] = Planner::axis_steps_per_mm[E_AXIS];

    steps_settings_show(false);
}

//! Cancel the extruder calibration.
void Printer_::cancel_xyz_motors_calibration()
{
    pages_.show_back_page();
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Sensor Settings
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

void Printer_::sensor_settings(advi3pp::KeyValue key_value)
{
    switch(key_value)
    {
        case KeyValue::Show:            sensor_settings_show(); break;
        case KeyValue::Save:            sensor_settings_save(); break;
        case KeyValue::Back:            sensor_settings_cancel(); break;
        default:                        Log::error() << F("Invalid key value ") << static_cast<uint16_t>(key_value) << Log::endl(); break;
    }
}

void Printer_::sensor_settings_show()
{
#ifdef ADVi3PP_BLTOUCH
    sensor_.send_z_height_to_lcd(zprobe_zoffset);
    pages_.save_forward_page();
    pages_.show_page(Page::SensorSettings);
#else
    pages_.show_page(Page::NoSensor);
#endif
}

void Printer_::sensor_settings_save()
{
#ifdef ADVi3PP_BLTOUCH
    sensor_.save_lcd_z_height();
    pages_.show_forward_page();
#endif
}

void Printer_::sensor_settings_cancel()
{
    pages_.show_back_page();
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Sensor Tuning
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

void Printer_::sensor_tuning(KeyValue key_value)
{
    switch(key_value)
    {
        case KeyValue::Show:            sensor_tuning_show(); break;
        case KeyValue::SensorLeveling:  sensor_leveling(); break;
#ifdef ADVi3PP_BLTOUCH
        case KeyValue::SensorSelfTest:  sensor_.self_test(); break;
        case KeyValue::SensorReset:     sensor_.reset(); break;
        case KeyValue::SensorDeploy:    sensor_.deploy(); break;
        case KeyValue::SensorStow:      sensor_.stow(); break;
        case KeyValue::SensorZHeight:   sensor_z_height(); break;
#endif
        case KeyValue::Back:            sensor_tuning_back(); break;
        default:                        Log::error() << F("Invalid key value ") << static_cast<uint16_t>(key_value) << Log::endl(); break;
    }
}

void Printer_::sensor_tuning_show()
{
#ifdef ADVi3PP_BLTOUCH
    pages_.show_page(Page::SensorTuning);
#else
    pages_.show_page(Page::NoSensor);
#endif
}

void Printer_::sensor_tuning_back()
{
    pages_.show_back_page();
}

void Printer_::sensor_leveling()
{
#ifdef ADVi3PP_BLTOUCH
    sensor_.leveling();
#else
    pages_.show_page(Page::NoSensor);
#endif
}

void Printer_::sensor_z_height()
{
#ifdef ADVi3PP_BLTOUCH
    sensor_.start_z_height();
    pages_.save_forward_page();
    pages_.show_waiting_page(F("Homing..."));
#else
    pages_.show_page(Page::NoSensor);
#endif
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Firmware Settings
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

void Printer_::firmware(KeyValue key_value)
{
    switch(key_value)
    {
        case KeyValue::Show:                firmware_settings_show(); break;
        case KeyValue::ThermalProtection:   firmware_settings_thermal_protection(); break;
        case KeyValue::USBBaudrateMinus:    firmware_settings_baudrate_minus(); break;
        case KeyValue::USBBaudratePlus:     firmware_settings_baudrate_plus(); break;
        case KeyValue::Save:                firmware_settings_save(); break;
        case KeyValue::Back:                firmware_settings_back(); break;
        default:                            Log::error() << F("Invalid key value ") << static_cast<uint16_t>(key_value) << Log::endl(); break;
    }
}

void Printer_::firmware_settings_show()
{
    usb_old_baudrate_ = usb_baudrate_;
    send_usb_baudrate();
    send_features();

    pages_.save_forward_page();
    pages_.show_page(Page::Firmware);
}

void Printer_::firmware_settings_thermal_protection()
{
    flip_bits(features_, Feature::ThermalProtection);
    send_features();
    // TODO
}

void Printer_::firmware_settings_save()
{
    enqueue_and_echo_commands_P(PSTR("M500"));
    pages_.show_back_page();

    change_usb_baudrate();
}

void Printer_::firmware_settings_back()
{
    usb_baudrate_ = usb_old_baudrate_;
    pages_.show_back_page();
}

void Printer_::send_usb_baudrate()
{
    String value; value << usb_baudrate_;

    WriteRamDataRequest frame{Variable::Value};
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

void Printer_::firmware_settings_baudrate_minus()
{
    auto index = UsbBaudrateIndex(usb_baudrate_);
    usb_baudrate_ = index > 0 ? usb_baudrates[index - 1] : usb_baudrates[0];
    send_usb_baudrate();
}

void Printer_::firmware_settings_baudrate_plus()
{
    auto index = UsbBaudrateIndex(usb_baudrate_);
    static const auto max = countof(usb_baudrates) - 1;
    usb_baudrate_ = index < max ? usb_baudrates[index + 1] : usb_baudrates[max];
    send_usb_baudrate();
}

void Printer_::change_usb_baudrate()
{
    // We do not use Log because this message is always output (Log is only active in DEBUG
    Serial.print(F("Switch USB baudrate to ")); Serial.print(usb_baudrate_); Serial.print("\r\n");

    // wait for last transmitted data to be sent
    Serial.flush();
    Serial.begin(usb_baudrate_);
    // empty out possible garbage from input buffer
    while(Serial.available())
        Serial.read();

    // We do not use Log because this message is always output (Log is only active in DEBUG
    Serial.print(F("\r\nUSB baudrate switched to ")); Serial.print(usb_baudrate_); Serial.print("\r\n");
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// LCD Settings
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

void Printer_::lcd(KeyValue key_value)
{
    switch(key_value)
    {
        case KeyValue::Show:                lcd_settings_show(); break;
        case KeyValue::LCDDimming:          lcd_settings_dimming(); break;
        case KeyValue::Back:                lcd_settings_back(); break;
        default:                            Log::error() << F("Invalid key value ") << static_cast<uint16_t>(key_value) << Log::endl(); break;
    }
}

void Printer_::send_features()
{
    WriteRamDataRequest frame{Variable::Features};
    frame << Uint16(static_cast<uint16_t>(features_));
    frame.send();
}

void Printer_::lcd_settings_show()
{
    send_features();
    pages_.save_forward_page();
    pages_.show_page(Page::LCD);
}

void Printer_::lcd_settings_dimming()
{
    flip_bits(features_, Feature::Dimming);
    dimming_.enable(test_one_bit(features_, Feature::Dimming));
    send_features();
}

void Printer_::lcd_settings_back()
{
    pages_.show_back_page();
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Status
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

//! Send statistics to the LCD screen.
void Printer_::send_stats()
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

void Printer_::set_update_graphs()
{
    update_graphs_ = true;
    next_update_graph_time_ = millis() + 2000;
}

void Printer_::update_graphs()
{
    if(!update_graphs_ || !ELAPSED(millis(), next_update_graph_time_))
        return;

    send_graphs_data();
    next_update_graph_time_ = millis() + 500;
}
    
//! Update the graphics (two channels: the bed and the hotend).
void Printer_::send_graphs_data()
{
    WriteCurveDataRequest frame{0b00000011};
    frame << Uint16{Temperature::degBed()}
          << Uint16{Temperature::degHotend(0)};
    frame.send(false);
}

void Printer_::clear_graphs()
{
    WriteRegisterDataRequest request{Register::TrendlineClear}; // TODO: Fix this (Mini DGUS)
    request << 0x55_u8;
    request.send();
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Errors
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

//! Display the Thermal Runaway Error screen.
void Printer_::temperature_error(const char* message)
{
    WriteRamDataRequest frame{Variable::CurrentFileName};
    frame << message;
    frame.send(true);
    pages_.show_page(advi3pp::Page::ThermalRunawayError);
}


// --------------------------------------------------------------------
// PidSettings
// --------------------------------------------------------------------

//! Initialize temporary Step settings.
void PidSettings::init()
{
    Kp = Temperature::Kp;
    Ki = Temperature::Ki;
    Kd = Temperature::Kd;
}

//! Save temporary Step settings.
void PidSettings::save()
{
    Temperature::Kp = Kp;
    Temperature::Ki = Ki;
    Temperature::Kd = Kd;

    enqueue_and_echo_commands_P(PSTR("M500"));
}

// --------------------------------------------------------------------
// StepSettings
// --------------------------------------------------------------------

//! Initialize temporary Step settings.
void StepSettings::init()
{
    axis_steps_per_mm[X_AXIS] = Planner::axis_steps_per_mm[X_AXIS];
    axis_steps_per_mm[Y_AXIS] = Planner::axis_steps_per_mm[Y_AXIS];
    axis_steps_per_mm[Z_AXIS] = Planner::axis_steps_per_mm[Z_AXIS];
    axis_steps_per_mm[E_AXIS] = Planner::axis_steps_per_mm[E_AXIS];
}

//! Save temporary Step settings.
void StepSettings::save()
{
    Planner::axis_steps_per_mm[X_AXIS] = axis_steps_per_mm[X_AXIS];
    Planner::axis_steps_per_mm[Y_AXIS] = axis_steps_per_mm[Y_AXIS];
    Planner::axis_steps_per_mm[Z_AXIS] = axis_steps_per_mm[Z_AXIS];
    Planner::axis_steps_per_mm[E_AXIS] = axis_steps_per_mm[E_AXIS];

    enqueue_and_echo_commands_P(PSTR("M500"));
}

// --------------------------------------------------------------------
// FeedrateSettings
// --------------------------------------------------------------------

//! Initialize temporary Feedrate settings.
void FeedrateSettings::init()
{
    max_feedrate_mm_s[X_AXIS] = Planner::max_feedrate_mm_s[X_AXIS];
    max_feedrate_mm_s[Y_AXIS] = Planner::max_feedrate_mm_s[Y_AXIS];
    max_feedrate_mm_s[Z_AXIS] = Planner::max_feedrate_mm_s[Z_AXIS];
    max_feedrate_mm_s[E_AXIS] = Planner::max_feedrate_mm_s[E_AXIS];
    min_feedrate_mm_s = Planner::min_feedrate_mm_s;
    min_travel_feedrate_mm_s = Planner::min_travel_feedrate_mm_s;
}

//! Save temporary Feedrate settings.
void FeedrateSettings::save()
{
    Planner::max_feedrate_mm_s[X_AXIS] = max_feedrate_mm_s[X_AXIS];
    Planner::max_feedrate_mm_s[Y_AXIS] = max_feedrate_mm_s[Y_AXIS];
    Planner::max_feedrate_mm_s[Z_AXIS] = max_feedrate_mm_s[Z_AXIS];
    Planner::max_feedrate_mm_s[E_AXIS] = max_feedrate_mm_s[E_AXIS];
    Planner::min_feedrate_mm_s = min_feedrate_mm_s;
    Planner::min_travel_feedrate_mm_s = min_travel_feedrate_mm_s;

    enqueue_and_echo_commands_P(PSTR("M500"));
}

// --------------------------------------------------------------------
// AccelerationSettings
// --------------------------------------------------------------------

//! Initialize temporary Acceleration settings.
void AccelerationSettings::init()
{
    max_acceleration_mm_per_s2[X_AXIS] = Planner::max_acceleration_mm_per_s2[X_AXIS];
    max_acceleration_mm_per_s2[Y_AXIS] = Planner::max_acceleration_mm_per_s2[Y_AXIS];
    max_acceleration_mm_per_s2[Z_AXIS] = Planner::max_acceleration_mm_per_s2[Z_AXIS];
    max_acceleration_mm_per_s2[E_AXIS] = Planner::max_acceleration_mm_per_s2[E_AXIS];
    acceleration = Planner::acceleration;
    retract_acceleration = Planner::retract_acceleration;
    travel_acceleration = Planner::travel_acceleration;
}

//! Save temporary Acceleration settings.
void AccelerationSettings::save()
{
    Planner::max_acceleration_mm_per_s2[X_AXIS] = max_acceleration_mm_per_s2[X_AXIS];
    Planner::max_acceleration_mm_per_s2[Y_AXIS] = max_acceleration_mm_per_s2[Y_AXIS];
    Planner::max_acceleration_mm_per_s2[Z_AXIS] = max_acceleration_mm_per_s2[Z_AXIS];
    Planner::max_acceleration_mm_per_s2[E_AXIS] = max_acceleration_mm_per_s2[E_AXIS];
    Planner::acceleration = acceleration;
    Planner::retract_acceleration = retract_acceleration;
    Planner::travel_acceleration =  travel_acceleration;

    enqueue_and_echo_commands_P(PSTR("M500"));
}

// --------------------------------------------------------------------
// JerkSettings
// --------------------------------------------------------------------

//! Initialize temporary Jerk settings.
void JerkSettings::init()
{
    max_jerk[X_AXIS] = Planner::max_jerk[X_AXIS];
    max_jerk[Y_AXIS] = Planner::max_jerk[Y_AXIS];
    max_jerk[Z_AXIS] = Planner::max_jerk[Z_AXIS];
    max_jerk[E_AXIS] = Planner::max_jerk[E_AXIS];
}

//! Save temporary Jerk settings.
void JerkSettings::save()
{
    Planner::max_jerk[X_AXIS] = max_jerk[X_AXIS];
    Planner::max_jerk[Y_AXIS] = max_jerk[Y_AXIS];
    Planner::max_jerk[Z_AXIS] = max_jerk[Z_AXIS];
    Planner::max_jerk[E_AXIS] = max_jerk[E_AXIS];

    enqueue_and_echo_commands_P(PSTR("M500"));
}

// --------------------------------------------------------------------
// Dimming
// --------------------------------------------------------------------

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

uint8_t Dimming::get_adjusted_brithness()
{
    auto brightness = static_cast<uint16_t>(brightness_);
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
    auto brightness = get_adjusted_brithness();

    WriteRegisterDataRequest frame{Register::Brightness};
    frame << Uint8{brightness};
    frame.send(true);
}

void Dimming::change_brightness(KeyValue brightness)
{
    reset();
    brightness_ = static_cast<Brightness>(brightness);
    send_brightness();
}

void Dimming::store_eeprom_data(EepromWrite& eeprom)
{
    eeprom.write(brightness_);
}

void Dimming::restore_eeprom_data(EepromRead& eeprom)
{
    eeprom.read(brightness_);
}

void Dimming::reset_eeprom_data()
{
    brightness_ = DEFAULT_BRIGHTNESS;
}

// --------------------------------------------------------------------
// BLTouch
// --------------------------------------------------------------------

#ifdef ADVi3PP_BLTOUCH

Sensor::Sensor(PagesManager& pages)
: pages_{pages}
{
}

void Sensor::send_z_height_to_lcd(double z_height)
{
    WriteRamDataRequest frame{Variable::SensorOffsetZ};
    frame << Uint16(z_height * 100);
    frame.send();
}

void Sensor::save_lcd_z_height()
{
    ReadRamData response{Variable::SensorOffsetZ, 1};
    if(!response.send_and_receive())
    {
        Log::error() << F("Receiving Frame (Sensor Settings)") << Log::endl();
        return;
    }

    Uint16 offsetZ; response >> offsetZ;
    save_z_height(static_cast<int16_t>(offsetZ.word) / 100.0);
}

void Sensor::leveling()
{
    pages_.show_waiting_page(F("Homing..."));
    enqueue_and_echo_commands_P((PSTR("G28"))); // homing
    enqueue_and_echo_commands_P((PSTR("G29"))); // leveling
    enqueue_and_echo_commands_P((PSTR("M420 S1"))); // Set Bed Leveling State
}

void Sensor::self_test()
{
    enqueue_and_echo_commands_P(PSTR("M280 P0 S120"));
}

void Sensor::reset()
{
    enqueue_and_echo_commands_P(PSTR("M280 P0 S160"));
}

void Sensor::deploy()
{
    enqueue_and_echo_commands_P(PSTR("M280 P0 S10"));
}

void Sensor::stow()
{
    enqueue_and_echo_commands_P(PSTR("M280 P0 S90"));
}

void Sensor::start_z_height()
{
	enqueue_and_echo_commands_P((PSTR("G28"))); // homing
    enqueue_and_echo_commands_P(PSTR("I0"));
}

void Sensor::save_z_height(double height)
{
    String command; command << F("M851 Z") << height;
    enqueue_and_echo_command(command.c_str());
}

#endif

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Preheat
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

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
    presets_[0].hotend = DEFAULT_PREHEAT_PRESET1_HOTEND;
    presets_[1].hotend = DEFAULT_PREHEAT_PRESET2_HOTEND;
    presets_[2].hotend = DEFAULT_PREHEAT_PRESET3_HOTEND;

    presets_[0].bed = DEFAULT_PREHEAT_PRESET1_BED;
    presets_[1].bed = DEFAULT_PREHEAT_PRESET2_BED;
    presets_[2].bed = DEFAULT_PREHEAT_PRESET3_BED;
}

//! Show the preheat screen
void Preheat::show()
{
    Log::log() << F("Preheat page") << Log::endl();
    WriteRamDataRequest frame{Variable::Preset1Bed};
    for(auto& preset : presets_)
        frame << Uint16(preset.bed) << Uint16(preset.hotend);
    frame.send();
    pages_.show_page(Page::Preheat);
}

void Preheat::back()
{
    pages_.show_back_page();
}

//! Preheat the nozzle and save the presets.
//! @param key_value    The index (starting from 1) of the preset to use
void Preheat::preset(uint16_t presetIndex)
{
    Log::log() << F("Preheat Start") << Log::endl();

    presetIndex -= 1;
    if(presetIndex >= NB_PRESETS)
    {
        Log::error() << F("Invalid preset # ") << presetIndex << Log::endl();
        return;
    }

    ReadRamData frame{Variable::Preset1Bed, 6};
    if(!frame.send_and_receive())
    {
        Log::error() << F("Receiving Frame (Presets)") << Log::endl();
        return;
    }

    Uint16 hotend, bed;
    for(auto& preset : presets_)
    {
        frame >> hotend >> bed;
        preset.hotend = hotend.word;
        preset.bed = bed.word;
    }

    // Save presets
    enqueue_and_echo_commands_P(PSTR("M500"));

    const Preset& preset = presets_[presetIndex];

    String command;

    command = F("M104 S"); command << preset.hotend;
    enqueue_and_echo_command(command.c_str());

    command = F("M140 S"); command << preset.bed;
    enqueue_and_echo_command(command.c_str());

    pages_.show_page(Page::Temperature);
    //set_update_graphs();
}

// --------------------------------------------------------------------
// CommandProcessor
// --------------------------------------------------------------------

CommandProcessor::CommandProcessor(PagesManager& pages, Sensor& sensor)
: pages_{pages}, sensor_{sensor}
{
}

//! Process command specific to this printer (I)
void CommandProcessor::process(const GCodeParser& parser)
{
    switch(parser.codenum)
    {
        case 0: icode_0(parser); break;
        default: Log::error() << F("Invalid I-code number ") << static_cast<uint16_t>(parser.codenum) << Log::endl(); break;
    }
}

void CommandProcessor::icode_0(const GCodeParser& parser)
{
#ifdef ADVi3PP_BLTOUCH
    if(axis_unhomed_error())
    {
        pages_.show_back_page();
        return;
    }

    const float old_feedrate_mm_s = feedrate_mm_s;
    feedrate_mm_s = MMM_TO_MMS(XY_PROBE_SPEED);

    do_blocking_move_to(100, 100, current_position[Z_AXIS]);

    LCD::set_status(F("Measuring Z-height (3 times)..."));
    DEPLOY_PROBE();
    double zHeight = run_z_probe();

    do_blocking_move_to_z(current_position[Z_AXIS] + Z_CLEARANCE_BETWEEN_PROBES, MMM_TO_MMS(Z_PROBE_SPEED_FAST));

    feedrate_mm_s = old_feedrate_mm_s;

    sensor_.send_z_height_to_lcd(-zHeight);
    pages_.show_page(Page::SensorSettings, false);
#endif
}

}
