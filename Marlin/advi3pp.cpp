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

#ifdef ADVi3PP_BLTOUCH
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
    extern Sponsors sponsors;
    extern LinearAdvanceTuning linear_advance_tuning;
    extern LinearAdvanceSettings linear_advance_settings;
    extern Diagnosis diagnosis;
    extern SdPrint sd_print;
    extern UsbPrint usb_print;
    extern AdvancedPause pause;
}

// --------------------------------------------------------------------
// ADVi3++ implementation
// --------------------------------------------------------------------

//! Initialize the printer and its LCD
void ADVi3pp_::setup()
{
    init_ = true;

    if(usb_baudrate_ != BAUDRATE)
        change_usb_baudrate(usb_baudrate_);

    Serial2.begin(advi3_pp_baudrate);
}

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
    SERIAL_ECHOLNPGM("Based on ADVi3++, Copyright (C) 2017-2018 Sebastien Andrivet");
}

void ADVi3pp_::send_sponsors()
{
    SERIAL_ECHOLNPGM("Sponsored by Johnathan Chamberlain, Timothy D Hoogland, Gavin Smith, Sawtoothsquid, JeremyThePrintr, K-D Byrne, RonnieL");
}

//! Store presets in permanent memory.
//! @param write Function to use for the actual writing
//! @param eeprom_index
//! @param working_crc
void ADVi3pp_::write(eeprom_write write, int& eeprom_index, uint16_t& working_crc)
{
    EepromWrite eeprom{write, eeprom_index, working_crc};

    preheat.write(eeprom);
    pid_settings.write(eeprom);
    eeprom.write(features_);
    eeprom.write(usb_baudrate_);
}

//! Restore presets from permanent memory.
//! @param read Function to use for the actual reading
//! @param eeprom_index
//! @param working_crc
void ADVi3pp_::read(eeprom_read read, int& eeprom_index, uint16_t& working_crc)
{
    EepromRead eeprom{read, eeprom_index, working_crc};

    preheat.read(eeprom);
    pid_settings.read(eeprom);
    eeprom.read(features_);
    eeprom.read(usb_baudrate_);

    dimming.enable(test_one_bit(features_, Feature::Dimming));
    enable_buzzer(test_one_bit(features_, Feature::Buzzer));
    enable_buzz_on_press(test_one_bit(features_, Feature::BuzzOnPress));

    check_and_fix();
}

//! Reset presets.
void ADVi3pp_::reset()
{
    preheat.reset();
    pid_settings.reset();
    features_ = DEFAULT_FEATURES;
    usb_baudrate_ = DEFAULT_USB_BAUDRATE;
}

//! Return the size of data specific to ADVi3++
uint16_t ADVi3pp_::size_of() const
{
    return
        preheat.size_of() +
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

void ADVi3pp_::save_settings()
{
    eeprom_mismatch.reset_mismatch();
    enqueue_and_echo_commands_P(PSTR("M500"));
}

bool ADVi3pp_::is_thermal_protection_enabled() const
{
    return test_one_bit(features_, Feature::ThermalProtection);
}


// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Incoming LCD commands and status update
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

//! Do not do too many things in setup so do things here
void ADVi3pp_::init()
{
    init_ = false;

#ifdef DEBUG
    Log::log() << F("This is a DEBUG build") << Log::endl();
#endif

#ifdef ADVi3PP_BLTOUCH
    Log::log() << F("This is a BLTouch build") << Log::endl();
#endif

    send_gplv3_7b_notice(); // You are not authorized to remove or alter this notice
    send_sponsors();
    versions.get_version_from_lcd();
    versions.send_advi3pp_version();
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

bool ADVi3pp_::is_busy()
{
    return busy_state != NOT_BUSY || Planner::has_blocks_queued();
}

void ADVi3pp_::update_progress()
{
    // Progress bar % comes from SD when actively printing
    if(card.sdprinting)
        progress_bar_percent = card.percentDone();
}

//! Update the status of the printer on the LCD.
void ADVi3pp_::send_status_data(bool force_update)
{
    if(!force_update && !task.is_update_time())
        return;

    int16_t progress_bar_low = progress_bar_percent >= 50 ? 5 : progress_bar_percent / 10;
    int16_t progress_var_high  = progress_bar_percent < 50 ? 0 : (progress_bar_percent / 10) - 5;

#ifdef ADVi3PP_BLTOUCH
    uint16_t probe_state = planner.leveling_active ? 2 : 1;
#else
    uint16_t probe_state = 0;
#endif

    WriteRamDataRequest frame{Variable::TargetBed};
    frame << Uint16(Temperature::target_temperature_bed)
          << Uint16(Temperature::degBed())
          << Uint16(Temperature::target_temperature[0])
          << Uint16(Temperature::degHotend(0))
          << Uint16(scale(fanSpeeds[0], 255, 100))
          << Uint16(lround(LOGICAL_Z_POSITION(current_position[Z_AXIS]) * 100.0))
          << Uint16(progress_bar_low)
          << Uint16(progress_var_high)
          << 0_u16
          << Uint16(probe_state);
    frame.send(false);

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

    Command command; Action action; Uint8 nb_words; Uint16 value;
    frame >> command >> action >> nb_words >> value;
    auto key_value = static_cast<KeyValue>(value.word);

    Log::log() << F("=R=> ") << nb_words.byte << F(" words, Action = 0x") << static_cast<uint16_t>(action)
               << F(", KeyValue = 0x") << value.word << Log::endl();

    switch(action)
    {
        case Action::Screen:                screens.handle(key_value); break;
        case Action::PrintCommand:          print_command(key_value); break;
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
        case Action::ZHeightMinus:          sensor_z_height.minus(); break;
        case Action::ZHeightPlus:           sensor_z_height.plus(); break;

        default:                            Log::error() << F("Invalid action ") << static_cast<uint16_t>(action) << Log::endl(); break;
    }
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Screens
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

//! Handle print commands.
//! @param key_value    The sub-action to handle
void ADVi3pp_::print_command(KeyValue key_value)
{
    if(card.isFileOpen())
        sd_print.handle(key_value);
    else
        usb_print.handle(key_value);
}

//! Display the Thermal Runaway Error screen.
void ADVi3pp_::temperature_error(const FlashChar* message)
{
    ADVi3pp_::set_status(message);
    send_status_data(true);
    pages.show_page(advi3pp::Page::ThermalRunawayError);
}

void ADVi3pp_::check_and_fix()
{
    if(Planner::max_jerk[X_AXIS] == 0) Planner::max_jerk[X_AXIS] = DEFAULT_XJERK;
    if(Planner::max_jerk[Y_AXIS] == 0) Planner::max_jerk[Y_AXIS] = DEFAULT_YJERK;
    if(Planner::max_jerk[Z_AXIS] == 0) Planner::max_jerk[Z_AXIS] = DEFAULT_ZJERK;
}

bool ADVi3pp_::has_status()
{
    return has_status_;
}

void ADVi3pp_::set_status(const char* message)
{
    message_.set(message).align(Alignment::Left);
    centered_.set(message).align(Alignment::Center);
    has_status_ = true;
}

void ADVi3pp_::set_status(const char* fmt, va_list& args)
{
    message_.set(fmt, args).align(Alignment::Left);
    centered_.set(fmt, args).align(Alignment::Center);
    has_status_ = true;
}

void ADVi3pp_::set_status(const FlashChar* message)
{
    message_.set(message).align(Alignment::Left);
    centered_.set(message).align(Alignment::Center);
    has_status_ = true;
}

void ADVi3pp_::set_status(const FlashChar* fmt, va_list& args)
{
    message_.set(fmt, args).align(Alignment::Left);
    centered_.set(fmt, args).align(Alignment::Center);
    has_status_ = true;
}

void ADVi3pp_::queue_status(const char* message)
{
    ADVString<100> string{F("M117 ")}; string << message;
    enqueue_and_echo_command(string.get());
}

void ADVi3pp_::queue_status(const FlashChar* message)
{
    ADVString<100> string{F("M117 ")}; string << message;
    enqueue_and_echo_command(string.get());
}

void ADVi3pp_::reset_status()
{
    message_.reset().align(Alignment::Left);
    centered_.reset().align(Alignment::Left);
    has_status_ = false;
}

void ADVi3pp_::set_progress_name(const char* name)
{
    progress_name_ = name;
    progress_.reset();
    percent_ = -1;
    compute_progress();
}

void ADVi3pp_::compute_progress()
{
    auto done = card.percentDone();
    if(done == percent_)
        return;

    progress_ = progress_name_ << " " << done << "%";
    percent_ = done;
}

void ADVi3pp_::reset_progress()
{
    progress_name_.reset();
    progress_.reset();
    percent_ = -1;
}

void ADVi3pp_::enable_buzzer(bool enable)
{
    buzzer_enabled_ = enable;
}

void ADVi3pp_::enable_buzz_on_press(bool enable)
{
    buzz_on_press_enabled_ = enable;
    if(enable)
        buzz_(50);
}

//! Activate the LCD internal buzzer for the given duration.
//! Note: The buzzer is not able to produce different frequencies so the 2nd parameter is ignored.
void ADVi3pp_::buzz(long duration, uint16_t)
{
    if(!buzzer_enabled_)
    {
        Log::log() << F("Silent Buzz") << Log::endl();
        return;
    }

    buzz_(duration);
}

void ADVi3pp_::buzz_(long duration)
{
    duration /= 10;

    WriteRegisterDataRequest request{Register::BuzzerBeepingTime};
    request << Uint8(static_cast<uint8_t>(duration > UINT8_MAX ? UINT8_MAX : duration));
    request.send();
}


void ADVi3pp_::buzz_on_press()
{
    if(!buzz_on_press_enabled_)
    {
        Log::log() << F("Silent Buzz") << Log::endl();
        return;
    }
    buzz_(BUZZ_ON_PRESS_DURATION);
}

void ADVi3pp_::change_usb_baudrate(uint32_t baudrate)
{
    usb_baudrate_ = baudrate;

    // We do not use Log because this message is always output (Log is only active in DEBUG)
    SERIAL_ECHO(F("Switch USB baudrate to "));
    SERIAL_ECHO(usb_baudrate_);
    SERIAL_ECHO("\r\n");

    // wait for last transmitted data to be sent
    SERIAL_FLUSH();
    MYSERIAL0.begin(usb_baudrate_);
    // empty out possible garbage from input buffer
    while(MYSERIAL0.available())
        MYSERIAL0.read();

    // We do not use Log because this message is always output (Log is only active in DEBUG
    SERIAL_ECHO(F("\r\nUSB baudrate switched to "));
    SERIAL_ECHO(usb_baudrate_);
    SERIAL_ECHO("\r\n");
}

void ADVi3pp_::change_features(Feature features)
{
    // TODO
    features_ = features;
}

uint16_t  ADVi3pp_::get_last_used_temperature(TemperatureKind kind) const
{
    return last_used_temperature_[kind == TemperatureKind::Hotend];
}

void ADVi3pp_::on_set_temperature(TemperatureKind kind, uint16_t temperature)
{
    if(temperature == 0)
        return;
    last_used_temperature_[kind == TemperatureKind::Hotend] = temperature;
    pid_settings.set_best_pid(kind, temperature);
}

#ifdef ADVi3PP_BLTOUCH
double ADVi3pp_::x_probe_offset_from_extruder() const
{
    return sensor_settings.x_probe_offset_from_extruder();
}

double ADVi3pp_::y_probe_offset_from_extruder() const
{
    return sensor_settings.y_probe_offset_from_extruder();
}

double ADVi3pp_::z_probe_offset_from_extruder() const
{
    return sensor_settings.z_probe_offset_from_extruder();
}
#endif

// --------------------------------------------------------------------
// Graphs
// --------------------------------------------------------------------

Graphs::Graphs()
{
    next_update_graph_time_ = millis() + 1000L * 10; // Wait 10 sec before starting updating graphs
}

void Graphs::update()
{
    if(!ELAPSED(millis(), next_update_graph_time_))
        return;

    send_data();
    next_update_graph_time_ = millis() + 500;
}

//! Update the graphics (two channels: the bed and the hotend).
void Graphs::send_data()
{
    WriteCurveDataRequest frame{0b00000011};
    frame << Uint16{Temperature::degBed()}
          << Uint16{Temperature::degHotend(0)};
    frame.send(false);
}

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

bool Task::has_background_task() const
{
    return bool(background_task_);
}

// --------------------------------------------------------------------

}

