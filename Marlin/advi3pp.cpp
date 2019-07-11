/**
 * Marlin 3D Printer Firmware For Wanhao Duplicator i3 Plus (ADVi3++)
 *
 * Copyright (C) 2017-2019 Sebastien Andrivet [https://github.com/andrivet/]
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

#include <HardwareSerial.h>

#include "serial.h"
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


extern uint8_t progress_bar_percent;
extern int16_t lcd_contrast;
extern void dwell(millis_t time);

namespace
{
    const unsigned long advi3_pp_baudrate = 115200; // Between the LCD panel and the mainboard

    const advi3pp::Feature DEFAULT_FEATURES =
        advi3pp::Feature::ThermalProtection |
        advi3pp::Feature::HeadParking |
        advi3pp::Feature::Dimming |
        advi3pp::Feature::Buzzer;

    const uint32_t DEFAULT_USB_BAUDRATE = BAUDRATE;

    const uint8_t BUZZ_ON_PRESS_DURATION = 10; // x 1 ms
}

#ifdef ADVi3PP_PROBE
bool set_probe_deployed(bool);
float run_z_probe();
extern float zprobe_zoffset;
#endif

namespace advi3pp {

inline namespace singletons
{
    ADVi3pp_ advi3pp;
    Pages pages;
    Task task;
    Feature features;
    Dimming dimming;
    Graphs graphs;

    extern Screens screens;
    extern Wait wait;
    extern Temperatures temperatures;
    extern LoadUnload load_unload;
    extern Preheat preheat;
    extern Move move;
    extern SdCard sd_card;
    extern FactoryReset factory_reset;
    extern ManualLeveling manual_leveling;
    extern ExtruderTuning extruder_tuning;
    extern PidTuning pid_tuning;
    extern FirmwareSettings firmware_settings;
    extern SensorSettings sensor_settings;
    extern NoSensor no_sensor;
    extern AutomaticLeveling automatic_leveling;
    extern LevelingGrid leveling_grid;
    extern SensorTuning sensor_tuning;
    extern SensorZHeight sensor_z_height;
    extern LcdSettings lcd_settings;
    extern Statistics statistics;
    extern Versions versions;
    extern PrintSettings print_settings;
    extern PidSettings pid_settings;
    extern StepSettings steps_settings;
    extern FeedrateSettings feedrates_settings;
    extern AccelerationSettings accelerations_settings;
    extern JerkSettings jerks_settings;
    extern Copyrights copyrights;
    extern ChangeFilament change_filament;
    extern EepromMismatch eeprom_mismatch;
    extern VersionsMismatch versions_mismatch;
    extern Sponsors sponsors;
    extern LinearAdvanceTuning linear_advance_tuning;
    extern LinearAdvanceSettings linear_advance_settings;
    extern Diagnosis diagnosis;
    extern Print print;
    extern AdvancedPause pause;
}

// --------------------------------------------------------------------
// ADVi3++ implementation
// --------------------------------------------------------------------

//! Initialize the printer and its LCD
void ADVi3pp_::setup_lcd_serial()
{
    Serial2.begin(advi3_pp_baudrate);
}

//! Change the baudrate. Check before that it is actually different.
void ADVi3pp_::change_baudrate()
{
    if(usb_baudrate_ != BAUDRATE)
        change_usb_baudrate(usb_baudrate_, false);
}

//! Initialize the printer and its LCD
void ADVi3pp_::setup()
{
    init_ = true;
}

//! Display the Boot animation (page)
void ADVi3pp_::show_boot_page()
{
    if(!eeprom_mismatch.check())
        return;

    if(!versions.check())
        return;

    pages.show_page(Page::Boot, ShowOptions::None);
}

//! Note to forks author:
//! Under GPLv3 provision 7(b), you are not authorized to remove or alter this notice.
void ADVi3pp_::send_gplv3_7b_notice()
{
    SERIAL_ECHOLNPGM("Based on ADVi3++, Copyright (C) 2017-2019 Sebastien Andrivet");
}

//! Send the list of sponsors on the serial port (www.patreon.com/andrivet)
void ADVi3pp_::send_sponsors()
{
    SERIAL_ECHOLNPGM("Sponsored by Alexander Cherenegar, Joshua");
}

//! Store presets in permanent memory.
//! @param write Function to use for the actual writing
//! @param eeprom_index
//! @param working_crc
void ADVi3pp_::write(eeprom_write write, int& eeprom_index, uint16_t& working_crc)
{
    EepromWrite eeprom{write, eeprom_index, working_crc};

    eeprom.write(version_);
    preheat.write(eeprom);
    sensor_settings.write(eeprom);
    pid_settings.write(eeprom);
    eeprom.write(features_);
    eeprom.write(usb_baudrate_);
}

//! Restore presets from permanent memory.
//! @param read Function to use for the actual reading
//! @param eeprom_index
//! @param working_crc
bool ADVi3pp_::read(eeprom_read read, int& eeprom_index, uint16_t& working_crc)
{
    EepromRead eeprom{read, eeprom_index, working_crc};

    eeprom.read(version_);
    preheat.read(eeprom);
    sensor_settings.read(eeprom);
    pid_settings.read(eeprom);
    eeprom.read(features_);
    eeprom.read(usb_baudrate_);

    dimming.enable(test_one_bit(features_, Feature::Dimming), false);
    enable_buzzer(test_one_bit(features_, Feature::Buzzer), false);
    enable_buzz_on_press(test_one_bit(features_, Feature::BuzzOnPress), false);

    return version_ == settings_version;
}

//! Reset presets.
void ADVi3pp_::reset()
{
    version_ = settings_version;
    preheat.reset();
    sensor_settings.reset();
    pid_settings.reset();
    features_ = DEFAULT_FEATURES;
    usb_baudrate_ = DEFAULT_USB_BAUDRATE;
}

//! Return the size of data specific to ADVi3++
uint16_t ADVi3pp_::size_of() const
{
    return
        sizeof(version_) +
        preheat.size_of() +
        sensor_settings.size_of() +
        pid_settings.size_of() +
        sizeof(features_) +
        sizeof(usb_baudrate_);
}


//! Inform the user that the EEPROM data are not compatible and have been reset
void ADVi3pp_::eeprom_settings_mismatch()
{
    // It is not possible to show the Mismatch page now since nothing is yet initialized.
    // It will be done in the setup method.
    eeprom_mismatch.set_mismatch();
}

//! Save the current settings permanently in EEPROM memory
void ADVi3pp_::save_settings()
{
    eeprom_mismatch.reset_mismatch();
    enqueue_and_echo_commands_P(PSTR("M500"));
}

//! Restore settings from EEPROM memory
void ADVi3pp_::restore_settings()
{
    // Note: Previously, M420 (bed leveling compensation) was reset by M501. It is no more the case.
    enqueue_and_echo_commands_P(PSTR("M501"));
}

//! Check if thermal runaway protection is activated or not.
//! @return true if thermal runaway protection is activated
bool ADVi3pp_::is_thermal_protection_enabled() const
{
    return test_one_bit(features_, Feature::ThermalProtection);
}


// --------------------------------------------------------------------
// Incoming LCD commands and status update
// --------------------------------------------------------------------

//! Do not do too many things in setup so do things here
void ADVi3pp_::init()
{
    init_ = false;

#ifdef DEBUG
    Log::log() << F("This is a DEBUG build") << Log::endl();
#endif

#if defined(ADVi3PP_BLTOUCH)
    Log::log() << F("This is a BLTouch build") << Log::endl();
#elif defined(ADVi3PP_MARK2)
    Log::log() << F("This is a Mark II build") << Log::endl();
#elif defined(ADVi3PP_HE180021)
    Log::log() << F("This is a Aldi UK Balco HE180021 build") << Log::endl();
#endif

    send_gplv3_7b_notice(); // You are not authorized to remove or alter this notice
    send_sponsors();
    graphs.clear();
    dimming.reset(true);
    reset_status();
    show_boot_page();
    set_status(F("ADVi3++ is ready"));
}

//! Background idle tasks
void ADVi3pp_::idle()
{
    if(init_)
        init();

    read_lcd_serial();
    dimming.check();
    task.execute_background_task();
    update_progress();
    send_status_data();
    graphs.update();
}

//! Check if the printer is doing something or is idle
//! @return true if the printer is busy or has some blocks queued
bool ADVi3pp_::is_busy()
{
    return busy_state != NOT_BUSY || Planner::has_blocks_queued();
}

//! Update the progress bar if the printer is printing for the SD card
void ADVi3pp_::update_progress()
{
    // Progress bar % comes from SD when actively printing
    if(card.sdprinting)
        progress_bar_percent = card.percentDone();
}

//! Get the current Z height (optionaly multiplied by a factor)
//! @return The current Z height in mm
double ADVi3pp_::get_current_z_height(int multiply) const
{
	auto height = LOGICAL_Z_POSITION(current_position[Z_AXIS]) * multiply;
	if(multiply != 1)
		return round(height);
	return height;
}

//! Update the status of the printer on the LCD.
void ADVi3pp_::send_status_data(bool force_update)
{
    // Right time for an update or force update?
    if(!force_update && !task.is_update_time())
        return;

    // The progress bar is split into two parts because of a limitation of the DWIN panel
    // so compute the progress of each part.
    int16_t progress_bar_low = progress_bar_percent >= 50 ? 5 : progress_bar_percent / 10;
    int16_t progress_var_high  = progress_bar_percent < 50 ? 0 : (progress_bar_percent / 10) - 5;

#ifdef ADVi3PP_PROBE
    uint16_t probe_state = planner.leveling_active ? 2 : 1;
#else
    uint16_t probe_state = 0;
#endif

    // Send the current status in one frame
    WriteRamDataRequest frame{Variable::TargetBed};
    frame << Uint16(Temperature::target_temperature_bed)
          << Uint16(Temperature::degBed())
          << Uint16(Temperature::target_temperature[0])
          << Uint16(Temperature::degHotend(0))
          << Uint16(scale(fanSpeeds[0], 255, 100))
          << Uint16(get_current_z_height(100))
          << Uint16(progress_bar_low)
          << Uint16(progress_var_high)
          << 0_u16 // TODO
          << Uint16(probe_state)
          << Uint16(feedrate_percentage);
    frame.send(false);

    compute_progress();
    // If one of the messages has changed, send them to the LCD panel
    if(message_.has_changed(true) || centered_.has_changed(true) || progress_.has_changed(true))
    {
        frame.reset(Variable::Message);
        frame << message_ << centered_ << progress_;
        frame.send(false);
    }
}

//! Read a frame from the LCD and act accordingly.
void ADVi3pp_::read_lcd_serial()
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

    buzz_on_press();
    dimming.reset();

    Command command{}; Action action{}; Uint8 nb_words; Uint16 value;
    frame >> command >> action >> nb_words >> value;
    auto key_value = static_cast<KeyValue>(value.word);

    Log::log() << F("=R=> ") << nb_words.byte << F(" words, Action = 0x") << static_cast<uint16_t>(action)
               << F(", KeyValue = 0x") << value.word << Log::endl();

    switch(action)
    {
        case Action::Screen:                screens.handle(key_value); break;
        case Action::PrintCommand:          print.handle(key_value); break;
        case Action::Wait:                  wait.handle(key_value); break;
        case Action::LoadUnload:            load_unload.handle(key_value); break;
        case Action::Preheat:               preheat.handle(key_value); break;
        case Action::Move:                  move.handle(key_value); break;
        case Action::SdCard:                sd_card.handle(key_value); break;
        case Action::FactoryReset:          factory_reset.handle(key_value); break;
        case Action::ManualLeveling:        manual_leveling.handle(key_value); break;
        case Action::ExtruderTuning:        extruder_tuning.handle(key_value); break;
        case Action::PidTuning:             pid_tuning.handle(key_value); break;
        case Action::SensorSettings:        sensor_settings.handle(key_value); break;
        case Action::Firmware:              firmware_settings.handle(key_value); break;
        case Action::NoSensor:              no_sensor.handle(key_value); break;
        case Action::LCD:                   lcd_settings.handle(key_value); break;
        case Action::Statistics:            statistics.handle(key_value); break;
        case Action::Versions:              versions.handle(key_value); break;
        case Action::PrintSettings:         print_settings.handle(key_value); break;
        case Action::PIDSettings:           pid_settings.handle(key_value); break;
        case Action::StepsSettings:         steps_settings.handle(key_value); break;
        case Action::FeedrateSettings:      feedrates_settings.handle(key_value); break;
        case Action::AccelerationSettings:  accelerations_settings.handle(key_value); break;
        case Action::JerkSettings:          jerks_settings.handle(key_value); break;
        case Action::Copyrights:            copyrights.handle(key_value); break;
        case Action::SensorTuning:          sensor_tuning.handle(key_value); break;
        case Action::AutomaticLeveling:     automatic_leveling.handle(key_value); break;
        case Action::SensorGrid:            leveling_grid.handle(key_value); break;
        case Action::SensorZHeight:         sensor_z_height.handle(key_value); break;
        case Action::ChangeFilament:        change_filament.handle(key_value); break;
        case Action::EEPROMMismatch:        eeprom_mismatch.handle(key_value); break;
        case Action::Sponsors:              sponsors.handle(key_value); break;
        case Action::LinearAdvanceTuning:   linear_advance_tuning.handle(key_value); break;
        case Action::LinearAdvanceSettings: linear_advance_settings.handle(key_value); break;
        case Action::Diagnosis:             diagnosis.handle(key_value); break;
        case Action::Temperatures:          temperatures.handle(key_value); break;
        case Action::VersionsMismatch:      versions_mismatch.handle(key_value); break;

        case Action::MoveXPlus:             move.x_plus_command(); break;
        case Action::MoveXMinus:            move.x_minus_command(); break;
        case Action::MoveYPlus:             move.y_plus_command(); break;
        case Action::MoveYMinus:            move.y_minus_command(); break;
        case Action::MoveZPlus:             move.z_plus_command(); break;
        case Action::MoveZMinus:            move.z_minus_command(); break;
        case Action::MoveEPlus:             move.e_plus_command(); break;
        case Action::MoveEMinus:            move.e_minus_command(); break;
        case Action::LCDBrightness:         lcd_settings.change_brightness(static_cast<int16_t>(key_value)); break;
        case Action::BabyMinus:             print_settings.baby_minus_command(); break;
        case Action::BabyPlus:              print_settings.baby_plus_command(); break;
        case Action::FeedrateMinus:         print_settings.feedrate_minus_command(); break;
        case Action::FeedratePlus:          print_settings.feedrate_plus_command(); break;
        case Action::FanMinus:              print_settings.fan_minus_command(); break;
        case Action::FanPlus:               print_settings.fan_plus_command(); break;
        case Action::HotendMinus:           print_settings.hotend_minus_command(); break;
        case Action::HotendPlus:            print_settings.hotend_plus_command(); break;
        case Action::BedMinus:              print_settings.bed_minus_command(); break;
        case Action::BedPlus:               print_settings.bed_plus_command(); break;
        case Action::ZHeightMinus:          sensor_z_height.minus(); break;
        case Action::ZHeightPlus:           sensor_z_height.plus(); break;

        default:                            Log::error() << F("Invalid action ") << static_cast<uint16_t>(action) << Log::endl(); break;
    }
}

// --------------------------------------------------------------------
// Screens
// --------------------------------------------------------------------

//! Display the Thermal Runaway Error screen.
void ADVi3pp_::temperature_error(const FlashChar* message)
{
    ADVi3pp_::set_status(message);
    send_status_data(true);
    pages.show_page(advi3pp::Page::ThermalRunawayError);
}

//! Check if there is currently a status to be displayed
//! @return true if there is a status (i.e. a message) to display
bool ADVi3pp_::has_status()
{
    return has_status_;
}

//! Set a status to display (a message)
void ADVi3pp_::set_status(const char* message)
{
    message_.set(message).align(Alignment::Left);
    centered_.set(message).align(Alignment::Center);
    has_status_ = true;
}

//! Set a status to display (a message)
void ADVi3pp_::set_status(const char* fmt, va_list& args)
{
    message_.set(fmt, args).align(Alignment::Left);
    centered_.set(fmt, args).align(Alignment::Center);
    has_status_ = true;
}

//! Set a status to display (a message)
void ADVi3pp_::set_status(const FlashChar* message)
{
    message_.set(message).align(Alignment::Left);
    centered_.set(message).align(Alignment::Center);
    has_status_ = true;
}

//! Set a status to display (a message)
void ADVi3pp_::set_status(const FlashChar* fmt, va_list& args)
{
    message_.set(fmt, args).align(Alignment::Left);
    centered_.set(fmt, args).align(Alignment::Center);
    has_status_ = true;
}

//! Clear the status (nothing to display)
void ADVi3pp_::reset_status()
{
    message_.reset().align(Alignment::Left);
    centered_.reset().align(Alignment::Left);
    has_status_ = false;
}

//! Handle Stop and Wait from the host: display a wait/continue page
void ADVi3pp_::stop_and_wait()
{
    wait.show_continue();
}

//! Set the name for the progess message. Usually, it is the name of the file printed.
void ADVi3pp_::set_progress_name(const char* name)
{
    progress_name_ = name;
    progress_.reset().align(Alignment::Left);
    percent_ = -1;
    compute_progress();
}

//! Compute the current progress message (name and percentage)
void ADVi3pp_::compute_progress()
{
    auto done = card.percentDone();
    if(done == percent_)
        return;

    progress_ = progress_name_;
	if(progress_.length() > 0)
		progress_  << " " << done << "%";
    progress_.align(Alignment::Left);
    percent_ = done;
}

//! Clear the progress message
void ADVi3pp_::reset_progress()
{
    progress_name_.reset();
    progress_.reset().align(Alignment::Left);
    percent_ = -1;
}

//! Enable or disable the buzzer
void ADVi3pp_::enable_buzzer(bool enable, bool /*doIt*/)
{
    buzzer_enabled_ = enable;
}

//! Enable or disable the buzzer when the LCD panel is presses
void ADVi3pp_::enable_buzz_on_press(bool enable, bool doIt)
{
    buzz_on_press_enabled_ = enable;
    if(enable && doIt)
        buzz_(50);
}

//! Activate the LCD internal buzzer for the given duration.
//! Note: If the buzzer is disabled, does nothing.
void ADVi3pp_::buzz(long duration)
{
    dimming.reset();
    if(!buzzer_enabled_)
    {
        Log::log() << F("Silent Buzz") << Log::endl();
        return;
    }

    buzz_(duration);
}

//! Send the buzz command to the LCD panel
//! @param duration Duration of the sound
void ADVi3pp_::buzz_(long duration)
{
    duration /= 10;

    WriteRegisterDataRequest request{Register::BuzzerBeepingTime};
    request << Uint8(static_cast<uint8_t>(duration > UINT8_MAX ? UINT8_MAX : duration));
    request.send();
}

//! Buzz briefly when the LCD panel is pressed.
//! Note: If buzz on press is disabled, does nothing
void ADVi3pp_::buzz_on_press()
{
    if(!buzz_on_press_enabled_)
    {
        Log::log() << F("Silent Buzz") << Log::endl();
        return;
    }
    buzz_(BUZZ_ON_PRESS_DURATION);
}

//! Change the USB baudrate between the printer and the host
//! @param baudrate New baudrate
//! @param disconnect Disconnect or not the host
void ADVi3pp_::change_usb_baudrate(uint32_t baudrate, bool disconnect)
{
    if(disconnect)
        SERIAL_ECHOLNPGM("//action:disconnect");

    // wait for last transmitted data to be sent
    MYSERIAL0.flush();
    MYSERIAL0.flushTX();
    MYSERIAL0.end();

    dwell(500); // Wait a little before reconnecting

    usb_baudrate_ = baudrate;
    MYSERIAL0.begin(usb_baudrate_);
    for(auto i = 0; i < 8; ++i)
        SERIAL_CHAR(' ');
    SERIAL_PROTOCOLLNPGM();
    SERIAL_PROTOCOLLNPGM("start");
}

//! Change the current set of features of ADVi3++ (thermal protection, dimming, ...)
void ADVi3pp_::change_features(Feature features)
{
    features_ = features;
}

//! Get the last used temperature for the hotend or the bad
//! @param kind Kind of temperature: hotend or bed
//! @return The last used themperature
uint16_t  ADVi3pp_::get_last_used_temperature(TemperatureKind kind) const
{
    return last_used_temperature_[kind == TemperatureKind::Hotend];
}

//! To be called when a new temperature is selected as a target
//! @param kind Kind of temperature: hotend or bed
//! @param temperature The new target temperature
void ADVi3pp_::on_set_temperature(TemperatureKind kind, uint16_t temperature)
{
    if(temperature == 0)
        return;
    last_used_temperature_[kind == TemperatureKind::Hotend] = temperature;
    pid_settings.set_best_pid(kind, temperature);
}

//! Process G-Codes specific to this firmware (Ax)
void ADVi3pp_::process_command(const GCodeParser& parser)
{
    switch(parser.codenum)
    {
        case 0: print.process_pause_code(); break;
        case 1: print.process_stop_code(); break;
        default: Log::error() << F("Invalid command ") << static_cast<uint16_t>(parser.codenum) << Log::endl(); break;
    }
}


// --------------------------------------------------------------------
// Graphs
// --------------------------------------------------------------------

//! Constructor
//! Initialize the update time
Graphs::Graphs()
{
    next_update_graph_time_ = millis() + 1000L * 10; // Wait 10 sec before starting updating graphs
}

//! Update the graphs (if the update delay has elapsed)
void Graphs::update()
{
    if(!ELAPSED(millis(), next_update_graph_time_))
        return;

    send_data();
    next_update_graph_time_ = millis() + 500;
}

//! Update the graphs on the LCD panel (two channels: the bed and the hotend).
void Graphs::send_data()
{
    WriteCurveDataRequest frame{0b00000011};
    frame << Uint16{Temperature::degBed()}
          << Uint16{Temperature::degHotend(0)};
    frame.send(false);
}

//! Clear the graphs
void Graphs::clear()
{
    WriteRegisterDataRequest request{Register::TrendlineClear}; // TODO: Fix this (Mini DGUS)
    request << 0x55_u8;
    request.send();
}

// --------------------------------------------------------------------
// Background tasks
// --------------------------------------------------------------------

//! Set the next (minimal) update time
//! @param delta    Duration to be added to the current time to compute the next (minimal) update time
void Task::set_next_update_time(unsigned int delta)
{
    next_update_time_ = millis() + delta;
}

//! Is it the time to update?
//! @return true if it is the time to update
bool Task::is_update_time()
{
    auto current_time = millis();
    if(!ELAPSED(current_time, next_update_time_))
        return false;
    set_next_update_time();
    return true;
}

//! Set the next background task and its delay
//! @param task     The next background task
//! @param delta    Duration to be added to the current time to execute the background task
void Task::set_background_task(const BackgroundTask& task, unsigned int delta)
{
    op_time_delta_ = delta;
    background_task_ = task;
    next_op_time_ = millis() + delta;
}

//! Reset the background task
void Task::clear_background_task()
{
    background_task_ = nullptr;
}

//! If there is an operating running, execute its next step
void Task::execute_background_task()
{
    if(!background_task_ || !ELAPSED(millis(), next_op_time_))
        return;

    next_op_time_ = millis() + op_time_delta_;
    background_task_();
}

//! Check if there is a background task to execute
//! @return true if there is a background task to execute
bool Task::has_background_task() const
{
    return bool(background_task_);
}

// --------------------------------------------------------------------

}

