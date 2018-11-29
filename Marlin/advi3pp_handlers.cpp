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
#include "duration_t.h"

#include "advi3pp.h"
#include "advi3pp_log.h"
#include "advi3pp_dgus.h"
#include "advi3pp_stack.h"
#include "advi3pp_.h"

#include <HardwareSerial.h>

// --------------------------------------------------------------------
// From Marlin / Arduino
// --------------------------------------------------------------------

uint8_t progress_bar_percent;
int16_t lcd_contrast;
extern char __bss_end;
extern char* __brkval;

#ifdef ADVi3PP_BLTOUCH
bool set_probe_deployed(bool);
float run_z_probe();
extern float zprobe_zoffset;
#endif

namespace
{
    const uint16_t advi3_pp_version = 0x400;
    const uint16_t advi3_pp_oldest_lcd_compatible_version = 0x400;
    const uint16_t advi3_pp_newest_lcd_compatible_version = 0x400;

    const uint16_t nb_visible_sd_files = 5;
    const uint16_t tuning_extruder_filament = 100; // 10 cm
	const uint16_t tuning_extruder_delta = 20; // 2 cm

    const uint32_t usb_baudrates[] = {9600, 19200, 38400, 57600, 115200, 230400, 250000};

    const int8_t BRIGHTNESS_MIN = 0x01;
    const int8_t BRIGHTNESS_MAX = 0x40;
    const uint8_t DIMMING_RATIO = 5; // in percent
    const uint16_t DIMMING_DELAY = 5 * 60; // 5 minutes

    const advi3pp::Preset DEFAULT_PREHEAT_PRESET[5] = {
        {180, 50, 0},
        {200, 60, 0},
        {220, 70, 0},
        {180, 00, 0},
        {200, 00, 0}
    };
}

namespace advi3pp {

inline namespace singletons
{
    extern ADVi3pp_ advi3pp;
    extern Pages pages;
    extern Task task;
    extern Feature features;
    extern Dimming dimming;
    extern Graphs graphs;

    Screens screens;
    Wait wait;
    Temperatures temperatures;
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
    AutomaticLeveling automatic_leveling;
    LevelingGrid leveling_grid;
    SensorTuning sensor_tuning;
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

template <typename T, size_t N>
constexpr size_t countof(T const (&)[N]) noexcept { return N; }

// --------------------------------------------------------------------
// Pages management
// --------------------------------------------------------------------

//! Show the given page on the LCD screen
//! @param [in] page The page to be displayed on the LCD screen
void Pages::show_page(Page page, ShowOptions options)
{
    Log::log() << F("Show page ") << static_cast<uint8_t>(page) << Log::endl();

    if(test_one_bit(options, ShowOptions::SaveBack))
    {
        auto current = get_current_page();
        Log::log() << F("Save back page ") << static_cast<uint8_t>(current) << Log::endl();
        back_pages_.push(current);
    }

    WriteRegisterDataRequest frame{Register::PictureID};
    frame << 00_u8 << page;
    frame.send(true);

    current_page_ = page;
}

//! Retrieve the current page on the LCD screen
Page Pages::get_current_page()
{
    // Boot page switches automatically (animation) to the Main page
    return current_page_ == Page::Boot ? Page::Main : current_page_;
}

//! Set page to display after the completion of an operation.
void Pages::save_forward_page()
{
    auto current = get_current_page();
    Log::log() << F("Save forward page ") << static_cast<uint8_t>(current) << Log::endl();
    forward_page_ = current;
}

//! Show the "Back" page on the LCD display.
void Pages::show_back_page()
{
    forward_page_ = Page::None;

    if(back_pages_.is_empty())
    {
        Log::log() << F("No back page, show Main") << Log::endl();
        show_page(Page::Main, ShowOptions::None);
        return;
    }

    auto back = back_pages_.pop();
    Log::log() << F("Pop back page ") << static_cast<uint8_t>(back) << Log::endl();
    show_page(back, ShowOptions::None);
}

//! Show the "Next" page on the LCD display.
void Pages::show_forward_page()
{
    if(forward_page_ == Page::None)
    {
        show_back_page();
        return;
    }

    if(!back_pages_.contains(forward_page_))
    {
        Log::error() << F("Back pages do not contain forward page ") << static_cast<uint8_t>(forward_page_) << Log::endl();
        return;
    }

    while(!back_pages_.is_empty())
    {
        Page back_page = back_pages_.pop();
        Log::log() << F("Pop back page ") << static_cast<uint8_t>(back_page) << Log::endl();
        if(back_page == forward_page_)
        {
            Log::log() << F("Show forward page ") << static_cast<uint8_t>(forward_page_) << Log::endl();
            show_page(forward_page_, ShowOptions::None);
            forward_page_ = Page::None;
            return;
        }
    }
}

// --------------------------------------------------------------------
// Screens
// --------------------------------------------------------------------

bool Screens::do_dispatch(KeyValue key_value)
{
    // Do not call Parent::do_dispatch

    switch(key_value)
    {
        case KeyValue::Temps:           show_temps(); break;
        case KeyValue::Print:           show_print(); break;
        case KeyValue::Controls:        pages.show_page(Page::Controls); break;
        case KeyValue::Tuning:          pages.show_page(Page::Tuning); break;
        case KeyValue::Settings:        pages.show_page(Page::Settings); break;
        case KeyValue::Infos:           pages.show_page(Page::Infos); break;
        case KeyValue::Motors:          pages.show_page(Page::MotorsSettings); break;
        case KeyValue::Leveling:        pages.show_page(Page::Leveling); break;
        case KeyValue::Back:            back_command(); break;
        default:                        return false;
    }

    return true;
}

//! Show one of the temperature graph screens depending of the context: either the SD printing screen,
//! the printing screen or the temperature screen.
void Screens::show_temps()
{
    if(!PrintCounter::isRunning() && !PrintCounter::isPaused())
    {
        temperatures.show();
        return;
    }

    // If there is a print running (or paused), display the print screen.
    pages.show_page(Page::Print);
}

//! Show one of the Printing screens depending of the context:
//! - If a print is running, display the Print screen
//! - Otherwise, try to access the SD card. Depending of the result, display the SD card Page or the Temperatures page
void Screens::show_print()
{
    // If there is a print running (or paused), display the SD or USB print screen
    if(PrintCounter::isRunning() || PrintCounter::isPaused())
    {
        pages.show_page(Page::Print);
        return;
    }

    wait.show(F("Try to access the SD card..."));
    task.set_background_task(BackgroundTask{this, &Screens::show_sd_or_temp_page});
}

void Screens::show_sd_or_temp_page()
{
    task.clear_background_task();

    card.initsd(); // Can take some time
    advi3pp.reset_status();
    if(!card.cardOK)
    {
        // SD card not accessible so fall back to Temperatures
        temperatures.show(false);
        return;
    }

    sd_card.show(ShowOptions::None);
}

// --------------------------------------------------------------------
// Wait
// --------------------------------------------------------------------

Page Wait::do_prepare_page()
{
    return Page::Waiting;
}

void Wait::show(const FlashChar* message, ShowOptions options)
{
    advi3pp.set_status(message);
    back_ = nullptr;
    continue_ = nullptr;
    pages.show_page(Page::Waiting, options);
}

void Wait::show(const FlashChar* message, const WaitCallback& back, ShowOptions options)
{
    advi3pp.set_status(message);
    back_ = back;
    continue_ = nullptr;
    pages.show_page(Page::WaitBack, options);
}

void Wait::show(const FlashChar* message, const WaitCallback& back, const WaitCallback& cont, ShowOptions options)
{
    advi3pp.set_status(message);
    back_ = back;
    continue_ = cont;
    pages.show_page(Page::WaitBackContinue, options);
}

void Wait::show_continue(const FlashChar* message, const WaitCallback& cont, ShowOptions options)
{
    advi3pp.set_status(message);
    back_ = nullptr;
    continue_ = cont;
    pages.show_page(Page::WaitContinue, options);
}

void Wait::do_back_command()
{
    if(!back_)
        Log::error() << F("No Back action defined") << Log::endl();
    else
    {
        back_();
        back_ = nullptr;
    }

    Parent::do_back_command();
}

void Wait::do_save_command()
{
    if(!continue_)
        Log::error() << F("No Continue action defined") << Log::endl();
    else
    {
        continue_();
        continue_ = nullptr;
    }

    pages.show_forward_page();
}

// --------------------------------------------------------------------
// Temperatures Graph
// --------------------------------------------------------------------

Page Temperatures::do_prepare_page()
{
    return Page::Temperature;
}

void Temperatures::show(const WaitCallback& back, bool save_back)
{
    back_ = back;
    Parent::show(save_back ? ShowOptions::SaveBack : ShowOptions::None);
}

void Temperatures::show(bool save_back)
{
    back_ = nullptr;
    Parent::show(save_back ? ShowOptions::SaveBack : ShowOptions::None);
}

void Temperatures::do_back_command()
{
    if(back_)
    {
        back_();
        back_ = nullptr;
    }

    Parent::do_back_command();
}

// --------------------------------------------------------------------
// Load and Unload Filament
// --------------------------------------------------------------------

//! Handle Load & Unload actions.
//! @param key_value    The sub-action to handle
bool LoadUnload::do_dispatch(KeyValue key_value)
{
    if(Parent::do_dispatch(key_value))
        return true;

    switch(key_value)
    {
        case KeyValue::Load:    load_command(); break;
        case KeyValue::Unload:  unload_command(); break;
        default:                return false;
    }

    return true;
}

//! Get the Load & Unload screen.
Page LoadUnload::do_prepare_page()
{
    WriteRamDataRequest frame{Variable::Value0};
    frame << Uint16(advi3pp.get_last_used_temperature(TemperatureKind::Hotend));
    frame.send();
    return Page::LoadUnload;
}

void LoadUnload::prepare(const BackgroundTask& background)
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

    task.set_background_task(background);
    wait.show(F("Wait until the target temp is reached..."), WaitCallback{this, &LoadUnload::stop});
}

//! Start Load action.
void LoadUnload::load_command()
{
    prepare(BackgroundTask(this, &LoadUnload::load_start_task));
}

//! Start Load action.
void LoadUnload::unload_command()
{
    prepare(BackgroundTask(this, &LoadUnload::unload_start_task));
}

//! Handle back from the Load on Unload LCD screen.
void LoadUnload::stop()
{
    Log::log() << F("Load/Unload Stop") << Log::endl();

    advi3pp.reset_status();
    task.set_background_task(BackgroundTask(this, &LoadUnload::stop_task));
    clear_command_queue();
    Temperature::setTargetHotend(0, 0);
}

void LoadUnload::stop_task()
{
    if(advi3pp.is_busy() || !task.has_background_task())
        return;

    task.clear_background_task();

    // Do this asynchronously to avoid race conditions
    enqueue_and_echo_commands_P(PSTR("M82"));       // absolute E mode
    enqueue_and_echo_commands_P(PSTR("G92 E0"));    // reset E axis
}

void LoadUnload::start_task(const char* command, const BackgroundTask& back_task)
{
    if(Temperature::current_temperature[0] >= Temperature::target_temperature[0] - 10)
    {
        Log::log() << F("Load/Unload Filament") << Log::endl();
        advi3pp.buzz(100); // Inform the user that the extrusion starts
        enqueue_and_echo_commands_P(command);
        task.set_background_task(back_task);
        advi3pp.set_status(F("Press Back when the filament comes out..."));
    }
}

//! Load the filament if the temperature is high enough.
void LoadUnload::load_start_task()
{
    start_task(PSTR("G1 E1 F120"), BackgroundTask(this, &LoadUnload::load_task));
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
    start_task(PSTR("G1 E-1 F120"), BackgroundTask(this, &LoadUnload::unload_task));
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
bool Preheat::do_dispatch(KeyValue key_value)
{
    if(Parent::do_dispatch(key_value))
        return true;

    switch(key_value)
    {
        case KeyValue::PresetPrevious:  previous_command(); break;
        case KeyValue::PresetNext:      next_command(); break;
        case KeyValue::Cooldown:        cooldown_command(); break;
        default:                        return false;
    }

    return true;
}

//! Store presets in permanent memory.
void Preheat::do_write(EepromWrite& eeprom) const
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
void Preheat::do_read(EepromRead& eeprom)
{
    for(auto& preset: presets_)
    {
        eeprom.read(preset.hotend);
        eeprom.read(preset.bed);
    }
}

//! Reset presets.
void Preheat::do_reset()
{
    for(size_t i = 0; i < NB_PRESETS; ++i)
    {
        presets_[i].hotend  = DEFAULT_PREHEAT_PRESET[i].hotend;
        presets_[i].bed     = DEFAULT_PREHEAT_PRESET[i].bed;
        presets_[i].fan     = DEFAULT_PREHEAT_PRESET[i].fan;
    }
}

uint16_t Preheat::do_size_of() const
{
    return NB_PRESETS * (sizeof(Preset::hotend) + sizeof(Preset::bed));
}

void Preheat::send_presets()
{
    Log::log() << F("Preheat page") << Log::endl();
    WriteRamDataRequest frame{Variable::Value0};
    frame << Uint16(presets_[index_].hotend)
          << Uint16(presets_[index_].bed)
          << Uint16(presets_[index_].fan);
    frame.send();

    ADVString<8> preset;
    preset << index_ + 1 << F(" / ") << NB_PRESETS;
    frame.reset(Variable::ShortText0);
    frame << preset;
    frame.send();
}

void Preheat::retrieve_presets()
{
    ReadRamData frame{Variable::Value0, 3};
    if(!frame.send_and_receive())
    {
        Log::error() << F("Error receiving presets") << Log::endl();
        return;
    }

    Uint16 hotend, bed, fan;
    frame >> hotend >> bed >> fan;

    presets_[index_].hotend = hotend.word;
    presets_[index_].bed = bed.word;
    presets_[index_].fan = fan.word;
}

//! Get the preheat screen
Page Preheat::do_prepare_page()
{
    send_presets();
    return Page::Preheat;
}

void Preheat::previous_command()
{
    if(index_ <= 0)
        return;
    retrieve_presets();
    --index_;
    send_presets();
}

void Preheat::next_command()
{
    if(index_ >= NB_PRESETS - 1)
        return;
    retrieve_presets();
    ++index_;
    send_presets();
}

void Preheat::do_save_command()
{
    retrieve_presets();

    const Preset& preset = presets_[index_];

    ADVString<15> command;

    command = F("M104 S"); command << preset.hotend;
    enqueue_and_echo_command(command.get());

    command = F("M140 S"); command << preset.bed;
    enqueue_and_echo_command(command.get());

    command = F("M106 S"); command << preset.fan;
    enqueue_and_echo_command(command.get());

    advi3pp.save_settings();
    temperatures.show(false);
}

//! Cooldown the bed and the nozzle
void Preheat::cooldown_command()
{
    Log::log() << F("Cooldown") << Log::endl();
    advi3pp.reset_status();
    Temperature::disable_all_heaters();
}


// --------------------------------------------------------------------
// Move & Home
// --------------------------------------------------------------------

//! Execute a move command
bool Move::do_dispatch(KeyValue key_value)
{
    if(Parent::do_dispatch(key_value))
        return true;

    switch(key_value)
    {
        case KeyValue::MoveXHome:           x_home_command(); break;
        case KeyValue::MoveYHome:           y_home_command(); break;
        case KeyValue::MoveZHome:           z_home_command(); break;
        case KeyValue::MoveAllHome:         all_home_command(); break;
        case KeyValue::DisableMotors:       disable_motors_command(); break;
        default:                            return false;
    }

    return true;
}

Page Move::do_prepare_page()
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
void Move::x_plus_command()
{
    move(PSTR("G1 X4 F1000"), 150);
}

//! Move the nozzle.
void Move::x_minus_command()
{
    move(PSTR("G1 X-4 F1000"), 150);
}

//! Move the nozzle.
void Move::y_plus_command()
{
    move(PSTR("G1 Y4 F1000"), 150);
}

//! Move the nozzle.
void Move::y_minus_command()
{
    move(PSTR("G1 Y-4 F1000"), 150);
}

//! Move the nozzle.
void Move::z_plus_command()
{
    move(PSTR("G1 Z0.5 F1000"), 10);
}

//! Move the nozzle.
void Move::z_minus_command()
{
    move(PSTR("G1 Z-0.5 F1000"), 10);
}

//! Extrude some filament.
void Move::e_plus_command()
{
    if(Temperature::degHotend(0) < 180)
        return;

    clear_command_queue();
    enqueue_and_echo_commands_P(PSTR("G91"));
    enqueue_and_echo_commands_P(PSTR("G1 E1 F120"));
    enqueue_and_echo_commands_P(PSTR("G90"));
}

//! Unextrude.
void Move::e_minus_command()
{
    if(Temperature::degHotend(0) < 180)
        return;

    clear_command_queue();
    enqueue_and_echo_commands_P(PSTR("G91"));
    enqueue_and_echo_commands_P(PSTR("G1 E-1 F120"));
    enqueue_and_echo_commands_P(PSTR("G90"));
}

//! Disable the motors.
void Move::disable_motors_command()
{
    enqueue_and_echo_commands_P(PSTR("M84"));
    axis_homed = 0;
    axis_known_position = 0;
}

//! Go to home on the X axis.
void Move::x_home_command()
{
    enqueue_and_echo_commands_P(PSTR("G28 X0"));
}

//! Go to home on the Y axis.
void Move::y_home_command()
{
    enqueue_and_echo_commands_P(PSTR("G28 Y0"));
}

//! Go to home on the Z axis.
void Move::z_home_command()
{
    enqueue_and_echo_commands_P(PSTR("G28 Z0"));
}

//! Go to home on all axis.
void Move::all_home_command()
{
    enqueue_and_echo_commands_P(PSTR("G28"));
}

// --------------------------------------------------------------------
// Sensor Tuning
// --------------------------------------------------------------------

#ifdef ADVi3PP_BLTOUCH

bool SensorTuning::do_dispatch(KeyValue key_value)
{
    if(Parent::do_dispatch(key_value))
        return true;

    switch(key_value)
    {
        case KeyValue::SensorSelfTest:  self_test_command(); break;
        case KeyValue::SensorReset:     reset_command(); break;
        case KeyValue::SensorDeploy:    deploy_command(); break;
        case KeyValue::SensorStow:      stow_command(); break;
        default:                        return false;
    }

    return true;
}

Page SensorTuning::do_prepare_page()
{
    return Page::SensorTuning;
}

void SensorTuning::self_test_command()
{
    enqueue_and_echo_commands_P(PSTR("M280 P0 S120"));
}

void SensorTuning::reset_command()
{
    enqueue_and_echo_commands_P(PSTR("M280 P0 S160"));
}

void SensorTuning::deploy_command()
{
    enqueue_and_echo_commands_P(PSTR("M280 P0 S10"));
}

void SensorTuning::stow_command()
{
    enqueue_and_echo_commands_P(PSTR("M280 P0 S90"));
}

#else

Page SensorTuning::do_prepare_page()
{
    return Page::NoSensor;
}

#endif

// --------------------------------------------------------------------
// Automatic Leveling
// --------------------------------------------------------------------

#ifdef ADVi3PP_BLTOUCH

Page AutomaticLeveling::do_prepare_page()
{
    sensor_interactive_leveling_ = true;
    pages.save_forward_page();
    wait.show(F("Homing..."));
    enqueue_and_echo_commands_P(PSTR("G28"));                   // homing
    enqueue_and_echo_commands_P(PSTR("G1 Z10 F240"));           // raise head
    enqueue_and_echo_commands_P(PSTR("G29 E"));                 // leveling
    enqueue_and_echo_commands_P(PSTR("G1 X0 Y0 F3000"));        // go back to corner

    return Page::None;
}

void AutomaticLeveling::g29_leveling_finished(bool success)
{
    if(!success)
    {
        if(!sensor_interactive_leveling_ && !IS_SD_FILE_OPEN) // i.e. USB print
            SERIAL_ECHOLNPGM("//action:disconnect"); // "disconnect" is the only standard command to stop an USB print

        if(sensor_interactive_leveling_)
            wait.show(F("Leveling failed"), WaitCallback{this, &AutomaticLeveling::g29_leveling_failed});
        else
            advi3pp.set_status(F("Leveling failed"));

        sensor_interactive_leveling_ = false;
        return;
    }

    advi3pp.reset_status();

    if(sensor_interactive_leveling_)
    {
        sensor_interactive_leveling_ = false;
        leveling_grid.show(ShowOptions::None);
    }
    else
    {
        enqueue_and_echo_commands_P(PSTR("M500"));      // Save settings (including mash)
        enqueue_and_echo_commands_P(PSTR("M420 S1"));   // Set bed leveling state (enable)
    }
}

void AutomaticLeveling::g29_leveling_failed()
{
    pages.show_back_page();
}

#else

Page AutomaticLeveling::do_prepare_page()
{
    return Page::NoSensor;
}

#endif

// --------------------------------------------------------------------
// Leveling Grid
// --------------------------------------------------------------------

#ifdef ADVi3PP_BLTOUCH

Page LevelingGrid::do_prepare_page()
{
    WriteRamDataRequest frame{Variable::Value0};
    for(auto y = 0; y < GRID_MAX_POINTS_Y; y++)
        for(auto x = 0; x < GRID_MAX_POINTS_X; x++)
            frame << Uint16(static_cast<int16_t>(z_values[x][y] * 1000));
    frame.send();

    return Page::SensorGrid;
}

void LevelingGrid::do_save_command()
{
    enqueue_and_echo_commands_P(PSTR("M500"));      // Save settings (including mash)
    enqueue_and_echo_commands_P(PSTR("M420 S1"));   // Set bed leveling state (enable)
    Parent::do_save_command();
}

#else

Page LevelingGrid::do_prepare_page()
{
    return Page::NoSensor;
}

#endif

// --------------------------------------------------------------------
// Manual Leveling
// --------------------------------------------------------------------

bool ManualLeveling::do_dispatch(KeyValue key_value)
{
    if(Parent::do_dispatch(key_value))
        return true;

    switch(key_value)
    {
        case KeyValue::LevelingPoint1:  point1_command(); break;
        case KeyValue::LevelingPoint2:  point2_command(); break;
        case KeyValue::LevelingPoint3:  point3_command(); break;
        case KeyValue::LevelingPoint4:  point4_command(); break;
        case KeyValue::LevelingPoint5:  point5_command(); break;
        case KeyValue::LevelingPointA:  pointA_command(); break;
        case KeyValue::LevelingPointB:  pointB_command(); break;
        case KeyValue::LevelingPointC:  pointC_command(); break;
        case KeyValue::LevelingPointD:  pointD_command(); break;
        default:                        return false;
    }

    return true;
}

void ManualLeveling::do_back_command()
{
    enqueue_and_echo_commands_P(PSTR("G1 Z30 F2000"));
    Parent::do_back_command();
}

//! Home the printer for bed leveling.
Page ManualLeveling::do_prepare_page()
{
    wait.show(F("Homing..."));
    ::axis_homed = 0;
    ::axis_known_position = 0;
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
    advi3pp.reset_status();
    task.clear_background_task();
    pages.show_page(Page::ManualLeveling, ShowOptions::None);
}

//! Handle leveling point #1.
void ManualLeveling::point1_command()
{
    Log::log() << F("Level point 1") << Log::endl();
    enqueue_and_echo_commands_P(PSTR("G1 Z10 F2000"));
    enqueue_and_echo_commands_P(PSTR("G1 X30 Y30 F6000"));
    enqueue_and_echo_commands_P(PSTR("G1 Z0 F1000"));
}

//! Handle leveling point #2.
void ManualLeveling::point2_command()
{
    Log::log() << F("Level point 2") << Log::endl();
    enqueue_and_echo_commands_P(PSTR("G1 Z10 F2000"));
    enqueue_and_echo_commands_P(PSTR("G1 X30 Y170 F6000"));
    enqueue_and_echo_commands_P(PSTR("G1 Z0 F1000"));
}

//! Handle leveling point #3.
void ManualLeveling::point3_command()
{
    Log::log() << F("Level point 3") << Log::endl();
    enqueue_and_echo_commands_P(PSTR("G1 Z10 F2000"));
    enqueue_and_echo_commands_P(PSTR("G1 X170 Y170 F6000"));
    enqueue_and_echo_commands_P(PSTR("G1 Z0 F1000"));
}

//! Handle leveling point #4.
void ManualLeveling::point4_command()
{
    Log::log() << F("Level point 4") << Log::endl();
    enqueue_and_echo_commands_P(PSTR("G1 Z10 F2000"));
    enqueue_and_echo_commands_P(PSTR("G1 X170 Y30 F6000"));
    enqueue_and_echo_commands_P(PSTR("G1 Z0 F1000"));
}

//! Handle leveling point #5.
void ManualLeveling::point5_command()
{
    Log::log() << F("Level point 5") << Log::endl();
    enqueue_and_echo_commands_P(PSTR("G1 Z10 F2000"));
    enqueue_and_echo_commands_P(PSTR("G1 X100 Y100 F6000"));
    enqueue_and_echo_commands_P(PSTR("G1 Z0 F1000"));
}

void ManualLeveling::pointA_command()
{
    Log::log() << F("Level point A") << Log::endl();
    enqueue_and_echo_commands_P(PSTR("G1 Z10 F2000"));
    enqueue_and_echo_commands_P(PSTR("G1 X100 Y30 F6000"));
    enqueue_and_echo_commands_P(PSTR("G1 Z0 F1000"));
}

void ManualLeveling::pointB_command()
{
    Log::log() << F("Level point B") << Log::endl();
    enqueue_and_echo_commands_P(PSTR("G1 Z10 F2000"));
    enqueue_and_echo_commands_P(PSTR("G1 X30 Y100 F6000"));
    enqueue_and_echo_commands_P(PSTR("G1 Z0 F1000"));
}

void ManualLeveling::pointC_command()
{
    Log::log() << F("Level point C") << Log::endl();
    enqueue_and_echo_commands_P(PSTR("G1 Z10 F2000"));
    enqueue_and_echo_commands_P(PSTR("G1 X100 Y170 F6000"));
    enqueue_and_echo_commands_P(PSTR("G1 Z0 F1000"));
}

void ManualLeveling::pointD_command()
{
    Log::log() << F("Level point D") << Log::endl();
    enqueue_and_echo_commands_P(PSTR("G1 Z10 F2000"));
    enqueue_and_echo_commands_P(PSTR("G1 X170 Y100 F6000"));
    enqueue_and_echo_commands_P(PSTR("G1 Z0 F1000"));
}


// --------------------------------------------------------------------
// SD Card
// --------------------------------------------------------------------

bool SdCard::do_dispatch(KeyValue key_value)
{
    if(Parent::do_dispatch(key_value))
        return true;

	switch(key_value)
	{
		case KeyValue::SDUp:	up_command(); break;
		case KeyValue::SDDown:	down_command(); break;
		case KeyValue::SDLine1:
		case KeyValue::SDLine2:
		case KeyValue::SDLine3:
		case KeyValue::SDLine4:
		case KeyValue::SDLine5:	select_file_command(static_cast<uint16_t>(key_value) - 1); break;
		default:                return false;
	}

	return true;
}

Page SdCard::do_prepare_page()
{
    show_first_page();
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

void SdCard::down_command()
{
	if(!card.cardOK)
		return;

	if(last_file_index_ >= nb_visible_sd_files)
		last_file_index_ -= nb_visible_sd_files;

    show_current_page();
}

void SdCard::up_command()
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
    WriteRamDataRequest frame{Variable::LongText0};

    ADVString<sd_file_length> name;
    for(uint8_t index = 0; index < nb_visible_sd_files; ++index)
    {
        get_file_name(index, name);
        frame << name; // Important to truncate, there is only limited space
    }
    frame.send(true);
}

//! Get a filename with a given index.
//! @param index    Index of the filename
//! @param name     Copy the filename into this Chars
void SdCard::get_file_name(uint8_t index_in_page, ADVString<sd_file_length>& name)
{
    name.reset();
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
void SdCard::select_file_command(uint16_t file_index)
{
    if(!card.cardOK)
        return;

    if(file_index > last_file_index_)
        return;

    card.getfilename(last_file_index_ - file_index);
    if(card.filenameIsDir)
        return;

    const char* filename = (card.longFilename[0] == 0) ? card.filename : card.longFilename;
    if(filename == nullptr) // If the SD card is not readable
        return;

    advi3pp.set_progress_name(filename);

    WriteRamDataRequest frame{Variable::FileName};
    frame << ADVString<26>(filename);
    frame.send(true);

    card.openFile(card.filename, true); // use always short filename so it will work even if the filename is long
    card.startFileprint();
    PrintCounter::start();

    pages.show_page(Page::Print);
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

bool SdPrint::do_is_printing() const
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

bool UsbPrint::do_is_printing() const
{
    return PrintCounter::isRunning();
}

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
    wait.show(F("Pausing..."));
}

void AdvancedPause::insert_filament()
{
    wait.show_continue(F("Insert filament and press continue..."),
                       WaitCallback{this, &AdvancedPause::filament_inserted}, ShowOptions::None);
}

void AdvancedPause::filament_inserted()
{
    ::wait_for_user = false;
    wait.show(F("Filament inserted.."), ShowOptions::None);
}

void AdvancedPause::printing()
{
    advi3pp.set_status(F("Printing"));
    pages.show_forward_page();
}

// --------------------------------------------------------------------
// Sensor Z-Height
// --------------------------------------------------------------------

#ifdef ADVi3PP_BLTOUCH

bool SensorZHeight::do_dispatch(KeyValue key_value)
{
    if(Parent::do_dispatch(key_value))
        return true;

    switch(key_value)
    {
        case KeyValue::ZHeight01:       multiplier01_command(); break;
        case KeyValue::ZHeight05:       multiplier05_command(); break;
        case KeyValue::ZHeight10:       multiplier10_command(); break;
        default:                        return false;
    }

    return true;
}

Page SensorZHeight::do_prepare_page()
{
    pages.save_forward_page();
    send_data();
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

    task.clear_background_task();
    advi3pp.reset_status();
    pages.show_page(Page::ZHeightTuning);
}

void SensorZHeight::do_back_command()
{
    enqueue_and_echo_commands_P((PSTR("G28 X0 Y0"))); // homing
}

void SensorZHeight::multiplier01_command()
{
    multiplier_ = 0.1;
}

void SensorZHeight::multiplier05_command()
{
    multiplier_ = 0.5;
}

void SensorZHeight::multiplier10_command()
{
    multiplier_ = 1.0;
}

void SensorZHeight::minus()
{
    height_ -= multiplier_;
    adjust_height();
}

void SensorZHeight::plus()
{
    height_ += multiplier_;
    adjust_height();
}

void SensorZHeight::adjust_height()
{
    ADVString<16> command;
    command << F("M851 Z") << height_;
    enqueue_and_echo_command(command.get());
    enqueue_and_echo_commands_P(PSTR("G1 Z0"));
}

void SensorZHeight::send_data() const
{
    WriteRamDataRequest frame{Variable::Value0};
    frame << Uint16(height_);
    frame.send();
}

#else

Page SensorZHeight::do_prepare_page()
{
    return Page::NoSensor;
}

#endif

// --------------------------------------------------------------------
// Extruder tuning
// --------------------------------------------------------------------

bool ExtruderTuning::do_dispatch(KeyValue key_value)
{
    if(Parent::do_dispatch(key_value))
        return true;

    switch(key_value)
    {
        case KeyValue::TuningStart:     start_command(); break;
        case KeyValue::TuningSettings:  settings_command(); break;
        default:                        return false;
    }

    return true;
}

//! Show the extruder tuning screen.
Page ExtruderTuning::do_prepare_page()
{
    steps_settings.backup();
    WriteRamDataRequest frame{Variable::Value0};
    frame << Uint16(advi3pp.get_last_used_temperature(TemperatureKind::Hotend));
    frame.send();
    return Page::ExtruderTuningTemp;
}

//! Start extruder tuning.
void ExtruderTuning::start_command()
{
    ReadRamData frame{Variable::Value0, 1};
    if(!frame.send_and_receive())
    {
        Log::error() << F("Receiving Frame (Target Temperature)") << Log::endl();
        return;
    }

    Uint16 hotend; frame >> hotend;
    wait.show(F("Heating the extruder..."), WaitCallback{this, &ExtruderTuning::cancel}, ShowOptions::None);
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

    ADVString<20> command; command << F("G1 E") << tuning_extruder_filament << " F50"; // Extrude slowly
    enqueue_and_echo_command(command.get());

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
    advi3pp.reset_status();
    finished();
}

//! Record the amount of filament extruded.
void ExtruderTuning::finished()
{
    Log::log() << F("Filament extruded ") << extruded_ << Log::endl();
    enqueue_and_echo_commands_P(PSTR("M82"));       // absolute E mode
    enqueue_and_echo_commands_P(PSTR("G92 E0"));    // reset E axis

    task.clear_background_task();
    pages.show_page(Page::ExtruderTuningMeasure, ShowOptions::None);
}

void ExtruderTuning::cancel()
{
    ::wait_for_user = ::wait_for_heatup = false;
    task.clear_background_task();
    Temperature::setTargetHotend(0, 0);
}

//! Cancel the extruder tuning.
void ExtruderTuning::do_back_command()
{
    task.clear_background_task();

    Temperature::setTargetHotend(0, 0);

    enqueue_and_echo_commands_P(PSTR("M82"));       // absolute E mode
    enqueue_and_echo_commands_P(PSTR("G92 E0"));    // reset E axis

    Parent::do_back_command();
}

//! Compute the extruder (E axis) new value and show the steps settings.
void ExtruderTuning::settings_command()
{
    ReadRamData frame{Variable::Value0, 1};
    if(!frame.send_and_receive())
    {
        Log::error() << F("Receiving Frame (Measures)") << Log::endl();
        return;
    }

    Uint16 e; frame >> e;
    auto new_value = Planner::axis_steps_per_mm[E_AXIS] * extruded_ / (extruded_ + tuning_extruder_delta - e.word / 10.0);

    Log::log()
            << F("Adjust: old = ") << Planner::axis_steps_per_mm[E_AXIS]
            << F(", expected = ") << extruded_
            << F(", measured = ") << (extruded_ + tuning_extruder_delta - e.word)
            << F(", new = ") << new_value << Log::endl();

    Planner::axis_steps_per_mm[E_AXIS] = new_value;
    steps_settings.show(ShowOptions::None);
}

// --------------------------------------------------------------------
// PID Tuning
// --------------------------------------------------------------------

//! Handle PID tuning.
//! @param key_value    The step of the PID tuning
bool PidTuning::do_dispatch(KeyValue key_value)
{
    if(Parent::do_dispatch(key_value))
        return true;

    switch(key_value)
    {
        case KeyValue::PidTuningStep2:  step2_command(); break;
        case KeyValue::PidTuningHotend: hotend_command(); break;
        case KeyValue::PidTuningBed:    bed_command(); break;
        default:                        return false;
    }

    return true;
}

//! Show step #1 of PID tuning
Page PidTuning::do_prepare_page()
{
    pages.save_forward_page();
    hotend_command();
    advi3pp.reset_status();
    return Page::PidTuning;
}

void PidTuning::send_data()
{
    WriteRamDataRequest frame{Variable::Value0};
    frame << Uint16(temperature_)
          << Uint16(kind_ != TemperatureKind::Hotend);
    frame.send();
}

void PidTuning::hotend_command()
{
    temperature_ = advi3pp.get_last_used_temperature(TemperatureKind::Hotend);
    kind_ = TemperatureKind::Hotend;
    send_data();
}

void PidTuning::bed_command()
{
    temperature_ = advi3pp.get_last_used_temperature(TemperatureKind::Bed);
    kind_ = TemperatureKind::Bed;
    send_data();
}

//! Show step #2 of PID tuning
void PidTuning::step2_command()
{
    advi3pp.reset_status();

    ReadRamData frame{Variable::Value0, 2};
    if(!frame.send_and_receive())
    {
        Log::error() << F("Receiving Frame (Target Temperature)") << Log::endl();
        return;
    }

    Uint16 temperature; frame >> temperature; temperature_ = temperature.word;

    if(kind_ == TemperatureKind::Hotend)
        enqueue_and_echo_commands_P(PSTR("M106 S255")); // Turn on fan (only for hotend)

    ADVString<20> auto_pid_command;
    auto_pid_command << F("M303 S") << temperature_
                     << (kind_ == TemperatureKind::Hotend ? F(" E0 U1") : F(" E-1 U1"));
    enqueue_and_echo_command(auto_pid_command.get());

    temperatures.show(WaitCallback{this, &PidTuning::cancel_pid});
}

void PidTuning::cancel_pid()
{
    ::wait_for_user = ::wait_for_heatup = false;
}

//! PID automatic tuning is finished.
void PidTuning::finished(bool success)
{
    Log::log() << F("Auto PID finished: ") << (success ? F("success") : F("failed")) << Log::endl();
    enqueue_and_echo_commands_P(PSTR("M106 S0"));
    if(!success)
    {
        advi3pp.set_status(F("PID tuning failed"));
        pages.show_back_page();
        return;
    }

    advi3pp.reset_status();
    pid_settings.add_pid(kind_, temperature_);
    bool inTuning = pages.get_current_page() == Page::PidTuning;
    pid_settings.show(inTuning ? ShowOptions::None : ShowOptions::SaveBack);
}

// --------------------------------------------------------------------
// Linear Advance Tuning
// --------------------------------------------------------------------

Page LinearAdvanceTuning::do_prepare_page()
{
    return Page::LinearAdvanceTuning;
}


// --------------------------------------------------------------------
// Diagnosis
// --------------------------------------------------------------------

Page Diagnosis::do_prepare_page()
{
    return Page::Diagnosis;
}


// --------------------------------------------------------------------
// Sensor Settings
// --------------------------------------------------------------------

#ifdef ADVi3PP_BLTOUCH

SensorSettings::SensorSettings()
{
    do_reset();
}

const FlashChar* SensorSettings::get_sensor_name() const
{
    // Note: F macro can be called only in a function, this is why this is coded like this

    auto advi3_side          = F("ADVi3++ Left Side");
    auto teaching_tech_side  = F("Teaching Tech L. Side");
    auto teaching_tech_front = F("Teaching Tech Front");
    auto custom              = F("Custom");

    static const FlashChar* names[NB_POSITIONS] = {advi3_side, teaching_tech_side, teaching_tech_front, custom};
    return names[index_];
}

bool SensorSettings::do_dispatch(KeyValue key_value)
{
    if(Parent::do_dispatch(key_value))
        return true;

    switch(key_value)
    {
        case KeyValue::SensorSettingsPrevious:  previous_command(); break;
        case KeyValue::SensorSettingsNext:      next_command(); break;
        default:                                return false;
    }

    return true;
}

Page SensorSettings::do_prepare_page()
{
    send_data();
    pages.save_forward_page();
    return Page::SensorSettings;
}

void SensorSettings::do_write(EepromWrite& eeprom) const
{
    for(size_t i = 0; i < NB_POSITIONS; ++i)
    {
        eeprom.write(positions_[i].x);
        eeprom.write(positions_[i].y);
        eeprom.write(positions_[i].z);
    }
}

void SensorSettings::do_read(EepromRead& eeprom)
{
    for(size_t i = 0; i < NB_POSITIONS; ++i)
    {
        eeprom.read(positions_[i].x);
        eeprom.read(positions_[i].y);
        eeprom.read(positions_[i].z);
    }
}

void SensorSettings::do_reset()
{
    positions_[0] = { -2800, -4000, -154 };
    positions_[1] = {  1000,  1100,  1200 };
    positions_[2] = {  2000,  2100,  2200 };
    positions_[3] = {  3000,  3100,  3200 };
    positions_[3] = {  0,  0,  0 };
}

uint16_t SensorSettings::do_size_of() const
{
    return NB_POSITIONS * sizeof(SensorPosition);
}

void SensorSettings::previous_command()
{
    if(index_ <= 0)
        return;
    get_data();
    index_ -= 1;
    send_data();
}

void SensorSettings::next_command()
{
    if(index_ >= NB_POSITIONS - 1)
        return;
    get_data();
    index_ += 1;
    send_data();
}

void SensorSettings::send_data() const
{
    ADVString<32> title{get_sensor_name()};

    WriteRamDataRequest frame{Variable::Value0};
    frame << Uint16(positions_[index_].x) << Uint16(positions_[index_].y) << Uint16(positions_[index_].z);
    frame.send();

    frame.reset(Variable::LongTextCentered0);
    frame << title.align(Alignment::Center);
    frame.send();
}

void SensorSettings::get_data()
{
    ReadRamData frame{Variable::Value0, 3};
    if(!frame.send_and_receive())
    {
        Log::error() << F("Receiving Frame (Sensor Settings)") << Log::endl();
        return;
    }

    Uint16 x, y, z;
    frame >> x >> y >> z;

    positions_[index_].x = x.word;
    positions_[index_].y = y.word;
    positions_[index_].z = z.word;
}

double SensorSettings::x_probe_offset_from_extruder() const
{
    return positions_[index_].x / 100.0;
}

double SensorSettings::y_probe_offset_from_extruder() const
{
    return positions_[index_].y / 100.0;
}

double SensorSettings::z_probe_offset_from_extruder() const
{
    return positions_[index_].z / 100.0;
}

#else

Page SensorSettings::do_prepare_page()
{
    return Page::NoSensor;
}

#endif



// --------------------------------------------------------------------
// Firmware Settings
// --------------------------------------------------------------------

bool FirmwareSettings::do_dispatch(KeyValue key_value)
{
    if(Parent::do_dispatch(key_value))
        return true;

    switch(key_value)
    {
        case KeyValue::ThermalProtection:   thermal_protection_command(); break;
        case KeyValue::RunoutSensor:        runout_sensor_command(); break;
        case KeyValue::USBBaudrateMinus:    baudrate_minus_command(); break;
        case KeyValue::USBBaudratePlus:     baudrate_plus_command(); break;
        default:                            return false;
    }

    return true;
}

Page FirmwareSettings::do_prepare_page()
{
    usb_baudrate_ = advi3pp.get_current_baudrate();
    features_ = advi3pp.get_current_features();
    send_usb_baudrate();
    send_features();
    return Page::Firmware;
}

void FirmwareSettings::thermal_protection_command()
{
    flip_bits(features_, Feature::ThermalProtection);
    send_features();
}

void FirmwareSettings::runout_sensor_command()
{
    flip_bits(features_, Feature::RunoutSensor);
    send_features();
}

void FirmwareSettings::do_save_command()
{
    advi3pp.change_usb_baudrate(usb_baudrate_);
    advi3pp.change_features(features_);
    Parent::do_save_command();
}

void FirmwareSettings::send_features() const
{
    WriteRamDataRequest frame{Variable::Value0}; frame << Uint16(static_cast<uint16_t>(features_)); frame.send();
}

void FirmwareSettings::send_usb_baudrate() const
{
    ADVString<6> value; value << usb_baudrate_;

    WriteRamDataRequest frame{Variable::ShortText0};
    frame << value;
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

void FirmwareSettings::baudrate_minus_command()
{
    auto index = UsbBaudrateIndex(usb_baudrate_);
    usb_baudrate_ = index > 0 ? usb_baudrates[index - 1] : usb_baudrates[0];
    send_usb_baudrate();
}

void FirmwareSettings::baudrate_plus_command()
{
    auto index = UsbBaudrateIndex(usb_baudrate_);
    static const auto max = countof(usb_baudrates) - 1;
    usb_baudrate_ = index < max ? usb_baudrates[index + 1] : usb_baudrates[max];
    send_usb_baudrate();
}

// --------------------------------------------------------------------
// LCD Settings
// --------------------------------------------------------------------

bool LcdSettings::do_dispatch(KeyValue key_value)
{
    if(Parent::do_dispatch(key_value))
        return true;

    switch(key_value)
    {
        case KeyValue::LCDDimming:          dimming_command(); break;
        case KeyValue::BuzzerOnAction:      buzz_on_action_command(); break;
        case KeyValue::BuzzOnPress:         buzz_on_press_command(); break;
        default:                            return false;
    }

    return true;
}

Page LcdSettings::do_prepare_page()
{
    features_ = advi3pp.get_current_features();
    send_data();
    return Page::LCD;
}

void LcdSettings::dimming_command()
{
    flip_bits(features_, Feature::Dimming);
    dimming.enable(test_one_bit(features_, Feature::Dimming));
    send_data();
    advi3pp.save_settings();
}

void LcdSettings::change_brightness(uint16_t brightness)
{
    dimming.change_brightness(brightness);
    send_data();
}

void LcdSettings::buzz_on_action_command()
{
    flip_bits(features_, Feature::Buzzer);
    advi3pp.enable_buzzer(test_one_bit(features_, Feature::Buzzer));
    send_data();
    advi3pp.save_settings();
}

void LcdSettings::buzz_on_press_command()
{
    flip_bits(features_, Feature::BuzzOnPress);
    advi3pp.enable_buzz_on_press(test_one_bit(features_, Feature::BuzzOnPress));
    send_data();
    advi3pp.save_settings();
}

void LcdSettings::send_data() const
{
    WriteRamDataRequest frame{Variable::Value0};
    frame << Uint16(static_cast<uint16_t>(features_)) << Uint16(::lcd_contrast);
    frame.send();
}


// --------------------------------------------------------------------
// Print Settings
// --------------------------------------------------------------------

bool PrintSettings::do_dispatch(KeyValue key_value)
{
    if(Parent::do_dispatch(key_value))
        return true;

    switch(key_value)
    {
        case KeyValue::Baby001:     multiplier_ = Multiplier::M0_01; break;
        case KeyValue::Baby005:     multiplier_ = Multiplier::M0_05; break;
        case KeyValue::Baby010:     multiplier_ = Multiplier::M0_10; break;
        default:                    return false;
    }

    send_data();
    return true;
}

double PrintSettings::get_multiplier_value() const
{
    switch(multiplier_)
    {
        case Multiplier::M0_01: return 0.01;
        case Multiplier::M0_05: return 0.05;
        case Multiplier::M0_10: return 0.10;
        default: break;
    }

    Log::error() << F("Invalid multiplier value: ") << static_cast<uint16_t >(multiplier_) << Log::endl();
    return 0.01;
}

void PrintSettings::send_data() const
{
    WriteRamDataRequest frame{Variable::Value0};
    frame << Uint16(feedrate_percentage)
          << Uint16(scale(fanSpeeds[0], 255, 100))
          << Uint16(Temperature::degTargetHotend(0))
          << Uint16(Temperature::degTargetBed())
          << Uint16(static_cast<uint16_t>(multiplier_));
    frame.send();
}

//! Display on the LCD screen the printing settings.
Page PrintSettings::do_prepare_page()
{
    send_data();
    return Page::PrintSettings;
}

//! Save the printing settings.
void PrintSettings::do_save_command()
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

    Parent::do_save_command();
}

void PrintSettings::baby_minus_command()
{
    ADVString<20> auto_pid_command; auto_pid_command << F("M290 Z-") << get_multiplier_value();
    enqueue_and_echo_command(auto_pid_command.get());
}

void PrintSettings::baby_plus_command()
{
    ADVString<20> auto_pid_command; auto_pid_command << F("M290 Z") << get_multiplier_value();
    enqueue_and_echo_command(auto_pid_command.get());
}

// --------------------------------------------------------------------
// PidSettings
// --------------------------------------------------------------------

PidSettings::PidSettings()
{
    do_reset();
}

bool PidSettings::do_dispatch(KeyValue key_value)
{
    if(Parent::do_dispatch(key_value))
        return true;

    switch(key_value)
    {
        case KeyValue::PidSettingsHotend:   hotend_command(); break;
        case KeyValue::PidSettingsBed:      bed_command(); break;
        case KeyValue::PidSettingPrevious:  previous_command(); break;
        case KeyValue::PidSettingNext:      next_command(); break;
        default:                            return false;
    }

    return true;
}

void PidSettings::hotend_command()
{
    save_data();
    kind_ = TemperatureKind::Hotend;
    send_data();
}

void PidSettings::bed_command()
{
    save_data();
    kind_ = TemperatureKind::Bed;
    send_data();
}

void PidSettings::previous_command()
{
    if(index_ <= 0)
        return;
    save_data();
    index_ -= 1;
    send_data();
}

void PidSettings::next_command()
{
    if(index_ >= NB_PIDs - 1)
        return;
    save_data();
    index_ += 1;
    send_data();
}

void PidSettings::do_write(EepromWrite& eeprom) const
{
    for(size_t i = 0; i < NB_PIDs; ++i)
    {
        eeprom.write(bed_pid_[i]);
        eeprom.write(hotend_pid_[i]);
    }
}

void PidSettings::do_read(EepromRead& eeprom)
{
    for(size_t i = 0; i < NB_PIDs; ++i)
    {
        eeprom.read(bed_pid_[i]);
        eeprom.read(hotend_pid_[i]);
    }
}

void PidSettings::do_reset()
{
    for(size_t i = 0; i < NB_PIDs; ++i)
    {
        bed_pid_[i].temperature_ = default_bed_temperature;
        bed_pid_[i].Kp_ = DEFAULT_bedKp;
        bed_pid_[i].Ki_ = DEFAULT_bedKi;
        bed_pid_[i].Kd_ = DEFAULT_bedKd;

        hotend_pid_[i].temperature_ = default_hotend_temperature;
        hotend_pid_[i].Kp_ = DEFAULT_Kp;
        hotend_pid_[i].Ki_ = DEFAULT_Ki;
        hotend_pid_[i].Kd_ = DEFAULT_Kd;
    }
}

uint16_t PidSettings::do_size_of() const
{
    return NB_PIDs * 2 * sizeof(Pid);
}

void PidSettings::set_current_pid() const
{
    if(kind_ == TemperatureKind::Hotend)
    {
        const Pid& pid = hotend_pid_[index_];

        Temperature::Kp = pid.Kp_;
        Temperature::Ki = scalePID_i(pid.Ki_);
        Temperature::Kd = scalePID_d(pid.Kd_);

        Log::log() << F("Set Hotend PID #") << index_ << F(" for temperature ") << pid.temperature_
                   << F(", P = ") << pid.Kp_ << F(", I = ") << pid.Ki_ << F(", D = ") << pid.Kd_ << Log::endl();
    }
    else
    {
        const Pid& pid = bed_pid_[index_];

        Temperature::bedKp = pid.Kp_;
        Temperature::bedKi = scalePID_i(pid.Ki_);
        Temperature::bedKd = scalePID_d(pid.Kd_);

        Log::log() << F("Set Bed PID #") << index_ << F(" for temperature ") << pid.temperature_
                   << F(", P = ") << pid.Kp_ << F(", I = ") << pid.Ki_ << F(", D = ") << pid.Kd_ << Log::endl();
    }
}

void PidSettings::get_current_pid()
{
    if(kind_ == TemperatureKind::Hotend)
    {
        Pid& pid = hotend_pid_[index_];

        pid.Kp_ = Temperature::Kp;
        pid.Ki_ = unscalePID_i(Temperature::Ki);
        pid.Kd_ = unscalePID_d(Temperature::Kd);

        Log::log() << F("Get Hotend PID #") << index_
                   << F(", P = ") << pid.Kp_ << F(", I = ") << pid.Ki_ << F(", D = ") << pid.Kd_ << Log::endl();
    }
    else
    {
        Pid& pid = bed_pid_[index_];

        pid.Kp_ = Temperature::bedKp;
        pid.Ki_ = unscalePID_i(Temperature::bedKi);
        pid.Kd_ = unscalePID_d(Temperature::bedKd);

        Log::log() << F("Get Hotend PID #") << index_
                   << F(", P = ") << pid.Kp_ << F(", I = ") << pid.Ki_ << F(", D = ") << pid.Kd_ << Log::endl();
    }
}

void PidSettings::add_pid(TemperatureKind kind, uint16_t temperature)
{
    kind_ = kind;
    Pid* pid = kind_ == TemperatureKind::Hotend ? hotend_pid_ : bed_pid_;
    for(size_t i = 0; i < NB_PIDs; ++i)
    {
        if(temperature == pid[i].temperature_)
        {
			Log::log() << (kind == TemperatureKind::Bed ? F("Bed") : F("Hotend"))
			           << F(" PID with temperature 0x") << temperature << F(" found, update settings") << Log::endl();
            index_ = i;
            get_current_pid();
            return;
        }
    }

	Log::log() << (kind == TemperatureKind::Bed ? F("Bed") : F("Hotend"))
	           << F(" PID with temperature 0x") << temperature << F(" NOT found, update settings #0") << Log::endl();
    // Temperature not found, so assign index 0, move PIDs and forget the last one
    index_ = 0;
    for(size_t i = 1; i < NB_PIDs; ++i)
        pid[i] = pid[i - 1];
}

void PidSettings::set_best_pid(TemperatureKind kind, uint16_t temperature)
{
    index_ = 0;
    kind_ = kind;

    uint16_t best_difference = 500;
    Pid* pid = kind_ == TemperatureKind::Hotend ? hotend_pid_ : bed_pid_;

    for(size_t i = 0; i < NB_PIDs; ++i)
    {
        auto difference = abs(temperature - pid[i].temperature_);
        if(difference < best_difference)
        {
            best_difference = difference;
            index_ = i;
        }
    }
	
	Log::log() << (kind_ == TemperatureKind::Bed ? F("Bed") : F("Hotend"))
	           << F(" PID with smallest difference (") << best_difference << F(") is at index #") << index_ << Log::endl();
    set_current_pid();
}

void PidSettings::send_data() const
{
    const Pid& pid = (kind_ == TemperatureKind::Hotend ? hotend_pid_ : bed_pid_)[index_];
    Log::log() << F("Send ") << (kind_ == TemperatureKind::Bed ? F("Bed") : F("Hotend")) << F(" PID #") << index_
               << F(", P = ") << pid.Kp_ << F(", I = ") << pid.Ki_ << F(", D = ") << pid.Kd_ << Log::endl();

    WriteRamDataRequest frame{Variable::Value0};
    frame << (kind_ == TemperatureKind::Hotend ? 0_u16 : 1_u16)
          << Uint16(pid.temperature_)
          << Uint16(pid.Kp_ * 100)
          << Uint16(pid.Ki_ * 100)
          << Uint16(pid.Kd_ * 100);
    frame.send();

    ADVString<8> indexes;
    indexes << index_ + 1 << F(" / ") << NB_PIDs;
    frame.reset(Variable::ShortText0);
    frame << indexes;
    frame.send();
}

void PidSettings::save_data()
{
    Pid& pid = (kind_ == TemperatureKind::Hotend ? hotend_pid_ : bed_pid_)[index_];

    ReadRamData response{Variable::Value0, 5};
    if(!response.send_and_receive())
    {
        Log::error() << F("Receiving Frame (PID Settings)") << Log::endl();
        return;
    }

    Uint16 kind, temperature, p, i, d;
    response >> kind >> temperature >> p >> i >> d;

    assert(kind.word == (kind_ == TemperatureKind::Hotend ? 0 : 1));

    pid.Kp_ = static_cast<float>(p.word) / 100;
    pid.Ki_ = static_cast<float>(i.word) / 100;
    pid.Kd_ = static_cast<float>(d.word) / 100;
	
	set_current_pid();
}

//! Show the PID settings
Page PidSettings::do_prepare_page()
{
    send_data();
    return Page::PidSettings;
}

//! Save the PID settings
void PidSettings::do_save_command()
{
    save_data();

    Pid& pid = (kind_ == TemperatureKind::Hotend ? hotend_pid_ : bed_pid_)[index_];
    Temperature::Kp = pid.Kp_;
    Temperature::Ki = pid.Ki_;
    Temperature::Kd = pid.Kd_;

    Parent::do_save_command();
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
void StepSettings::do_restore()
{
    Planner::axis_steps_per_mm[X_AXIS] = backup_[X_AXIS];
    Planner::axis_steps_per_mm[Y_AXIS] = backup_[Y_AXIS];
    Planner::axis_steps_per_mm[Z_AXIS] = backup_[Z_AXIS];
    Planner::axis_steps_per_mm[E_AXIS] = backup_[E_AXIS];
}

//! Show the Steps settings
//! @param init     Initialize the settings are use those already set
Page StepSettings::do_prepare_page()
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
void StepSettings::do_save_command()
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

    Parent::do_save_command();
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
void FeedrateSettings::do_restore()
{
    Planner::max_feedrate_mm_s[X_AXIS] = backup_max_feedrate_mm_s_[X_AXIS];
    Planner::max_feedrate_mm_s[Y_AXIS] = backup_max_feedrate_mm_s_[Y_AXIS];
    Planner::max_feedrate_mm_s[Z_AXIS] = backup_max_feedrate_mm_s_[Z_AXIS];
    Planner::max_feedrate_mm_s[E_AXIS] = backup_max_feedrate_mm_s_[E_AXIS];
    Planner::min_feedrate_mm_s = backup_min_feedrate_mm_s_;
    Planner::min_travel_feedrate_mm_s = backup_min_travel_feedrate_mm_s_;
}

//! Show the Feedrate settings
Page FeedrateSettings::do_prepare_page()
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
void FeedrateSettings::do_save_command()
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

    Parent::do_save_command();
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
void AccelerationSettings::do_restore()
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
Page AccelerationSettings::do_prepare_page()
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
void AccelerationSettings::do_save_command()
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

    Parent::do_save_command();
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
void JerkSettings::do_restore()
{
    Planner::max_jerk[X_AXIS] = backup_max_jerk_[X_AXIS];
    Planner::max_jerk[Y_AXIS] = backup_max_jerk_[Y_AXIS];
    Planner::max_jerk[Z_AXIS] = backup_max_jerk_[Z_AXIS];
    Planner::max_jerk[E_AXIS] = backup_max_jerk_[E_AXIS];
}

//! Show the Jerk settings
Page JerkSettings::do_prepare_page()
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
void JerkSettings::do_save_command()
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

    Parent::do_save_command();
}

// --------------------------------------------------------------------
// Linear Advance Settings
// --------------------------------------------------------------------

void LinearAdvanceSettings::do_backup()
{
    backup_extruder_advance_K = Planner::extruder_advance_K;
}

void LinearAdvanceSettings::do_restore()
{
    Planner::extruder_advance_K = backup_extruder_advance_K;
}

Page LinearAdvanceSettings::do_prepare_page()
{
    WriteRamDataRequest frame{Variable::Value0};
    frame << Uint16(Planner::extruder_advance_K * 10);
    frame.send();

    return Page::LinearAdvanceSettings;
}

void LinearAdvanceSettings::do_save_command()
{
    ReadRamData response{Variable::Value0, 1};
    if(!response.send_and_receive())
    {
        Log::error() << F("Receiving Frame (Linear Advance Settings)") << Log::endl();
        return;
    }

    Uint16 k; response >> k;
    Planner::extruder_advance_K = k.word / 10.0;

    Parent::do_save_command();
}

// --------------------------------------------------------------------
// Factory Reset
// --------------------------------------------------------------------

Page FactoryReset::do_prepare_page()
{
    return Page::FactoryReset;
}

void FactoryReset::do_save_command()
{
    enqueue_and_echo_commands_P(PSTR("M502"));
    Parent::do_save_command();
}

// --------------------------------------------------------------------
// Statistics
// --------------------------------------------------------------------

Page Statistics::do_prepare_page()
{
    send_stats();
    return Page::Statistics;
}

char* get_heap_end()
{
    return __brkval ? __brkval : &__bss_end;
}

char* get_stack_top()
{
    char v;
    return &v + 1;
}

uint16_t get_free_SRAM()
{
	auto stack = get_stack_top();
	auto heap = get_heap_end();
	auto free_ram = static_cast<uint16_t>(stack - heap);
	
	Log::log() << F("SRAM Top/Stack = 0x") << reinterpret_cast<uint16_t>(stack)
	           << F(", Bottom/Heap = 0x") << reinterpret_cast<uint16_t>(heap)
	           << F(", Free = 0x") << free_ram << Log::endl();
	
    return free_ram;
}

void Statistics::send_stats()
{
    printStatistics stats = PrintCounter::getStats();

    ADVString<48> printTime{duration_t{stats.printTime}};
    ADVString<48> longestPrint{duration_t{stats.longestPrint}};

    ADVString<48> filament_used;
    filament_used << static_cast<unsigned int>(stats.filamentUsed / 1000)
                  << '.'
                  << (static_cast<unsigned int>(stats.filamentUsed / 100) % 10)
                  << 'm';

    WriteRamDataRequest frame{Variable::Value0};
    frame << Uint16(stats.totalPrints)
          << Uint16(stats.finishedPrints)
          << Uint16(get_free_SRAM());
    frame.send();

    frame.reset(Variable::LongText0);
    frame << printTime.align(Alignment::Left)
          << longestPrint.align(Alignment::Left)
          << filament_used.align(Alignment::Left);
    frame.send();
}

// --------------------------------------------------------------------
// Versions
// --------------------------------------------------------------------

bool Versions::check()
{
    if(!is_lcd_version_valid())
    {
        send_versions();
        pages.show_page(Page::VersionsMismatch, ShowOptions::None);
        return false;
    }

    return true;
}

void Versions::get_version_from_lcd()
{
    ReadRamData frame{Variable::ADVi3ppLCDversion, 1};
    if(!frame.send_and_receive())
    {
        Log::error() << F("Receiving Frame (Measures)") << Log::endl();
        return;
    }

    Uint16 version; frame >> version;
    Log::log() << F("ADVi3++ LCD version = ") <<  version.word << Log::endl();
    lcd_version_ = version.word;
}

//! Get the current LCD firmware version.
//! @return     The version as a string.
template<size_t L>
ADVString<L>& get_lcd_firmware_version(ADVString<L>& lcd_version)
{
    ReadRegister response{Register::Version, 1};
    if(!response.send_and_receive())
    {
        Log::error() << F("Receiving Frame (Version)") << Log::endl();
        return lcd_version;
    }

    Uint8 version; response >> version;
    Log::log() << F("LCD Firmware raw version = ") << version.byte << Log::endl();

    lcd_version << (version.byte / 0x10) << '.' << (version.byte % 0x10);
    return lcd_version;
}

//! Convert a version from its hexadecimal representation.
//! @param hex_version  Hexadecimal representation of the version
//! @return             Version as a string
template<size_t L>
ADVString<L>& convert_version(ADVString<L>& version, uint16_t hex_version)
{
    version << hex_version / 0x0100 << '.' << (hex_version % 0x100) / 0x10 << '.' << hex_version % 0x10;
    return version;
}

//! Send the different versions to the LCD screen.
void Versions::send_versions()
{
    ADVString<16> marlin_version{SHORT_BUILD_VERSION};
    ADVString<16> motherboard_version;
    ADVString<16> advi3pp_lcd_version;
    ADVString<16> lcd_firmware_version;

    marlin_version.align(Alignment::Left);
    convert_version(motherboard_version, advi3_pp_version).align(Alignment::Left);
    convert_version(advi3pp_lcd_version, lcd_version_).align(Alignment::Left);
    get_lcd_firmware_version(lcd_firmware_version).align(Alignment::Left);

    WriteRamDataRequest frame{Variable::ShortText0};
    frame << motherboard_version
          << advi3pp_lcd_version
          << lcd_firmware_version
          << marlin_version;
    frame.send();
}

void Versions::send_advi3pp_version()
{
    ADVString<16> motherboard_version;
    convert_version(motherboard_version, advi3_pp_version).align(Alignment::Left);

    WriteRamDataRequest frame{Variable::ADVi3ppversion};
    frame << motherboard_version;
    frame.send();
}

bool Versions::is_lcd_version_valid()
{
    return lcd_version_ >= advi3_pp_oldest_lcd_compatible_version && lcd_version_ <= advi3_pp_newest_lcd_compatible_version;
}

Page Versions::do_prepare_page()
{
    send_versions();
    return Page::Versions;
}

// --------------------------------------------------------------------
// Sponsors
// --------------------------------------------------------------------

Page Sponsors::do_prepare_page()
{
    return Page::Sponsors;
}

// --------------------------------------------------------------------
// Copyrights
// --------------------------------------------------------------------

Page Copyrights::do_prepare_page()
{
    return Page::Copyrights;
}

// --------------------------------------------------------------------
// Change Filament
// --------------------------------------------------------------------

Page ChangeFilament::do_prepare_page()
{
    return Page::None;
}


// --------------------------------------------------------------------
// EEPROM data mismatch
// --------------------------------------------------------------------

bool EepromMismatch::check()
{
    if(does_mismatch())
    {
        show(ShowOptions::None);
        return false;
    }

    return true;
}

Page EepromMismatch::do_prepare_page()
{
    return Page::EEPROMMismatch;
}

void EepromMismatch::do_save_command()
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
// No Sensor
// --------------------------------------------------------------------

Page NoSensor::do_prepare_page()
{
    return Page::NoSensor;
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
    reset(true);
}

void Dimming::set_next_checking_time()
{
    next_check_time_ = millis() + 200;
}

void Dimming::set_next_dimmming_time()
{
    next_dimming_time_ = millis() + 1000ul * DIMMING_DELAY;
}

uint8_t Dimming::get_adjusted_brightness()
{
    int16_t brightness = ::lcd_contrast;
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

    // Reset TouchPanelFlag
    WriteRegisterDataRequest request{Register::TouchPanelFlag};
    request << 00_u8;
    request.send(false);

    if(value.byte == 0x5A)
    {
        Log::log() << F("Panel touched, reset dimming") << Log::endl();
        reset();
        return;
    }

    if(!dimming_ && ELAPSED(millis(), next_dimming_time_))
    {
        Log::log() << F("Delay elapsed, dim the panel") << Log::endl();
        dimming_ = true;
        send_brightness();
    }
}

void Dimming::reset(bool force)
{
    set_next_dimmming_time();
    if(!force && !dimming_) // Already reset, nothing more to do (unless force is true)
        return;

    dimming_ = false;
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

}
