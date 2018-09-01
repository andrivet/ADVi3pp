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
#include "advi3pp_utils.h"
#include "advi3pp_.h"

#include <serial.h>
#include <HardwareSerial.h>
//extern HardwareSerial Serial2;

extern uint8_t progress_bar_percent;
extern int16_t lcd_contrast;

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
    extern SensorTuning sensor_tuning;
    extern SensorGrid sensor_grid;
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
    if(eeprom_mismatch.does_mismatch())
    {
        pages.show_page(Page::EEPROMMismatch);
        return;
    }

    if(!versions.is_lcd_version_valid())
    {
        pages.show_page(Page::VersionsMismatch, false);
        return;
    }

    Page page = pages.get_current_page();
    if(page > Page::Main)
        pages.show_page(Page::Main, false);
}

//! Note to forks author:
//! Under GPLv3 provision 7(b), you are not authorized to remove or alter this notice.
void ADVi3pp_::send_gplv3_7b_notice()
{
    Log::log() << F("Based on ADVi3++, Copyright (C) 2017 Sebastien Andrivet") << Log::endl();
}

void ADVi3pp_::send_sponsors()
{
    Log::log() << F("Sponsored by Johnathan Chamberlain, Timothy D Hoogland, Gavin Smith, Sawtoothsquid") << Log::endl();
}

//! Process command specific to this printer (I)
void ADVi3pp_::process_command()
{
    switch(GCodeParser::codenum)
    {
        case 0: icode_0(parser); break;
        default: Log::error() << F("Invalid I-code number ")
                              << static_cast<uint16_t>(GCodeParser::codenum)
                              << Log::endl(); break;
    }
}

//! Store presets in permanent memory.
//! @param write Function to use for the actual writing
//! @param eeprom_index
//! @param working_crc
void ADVi3pp_::store_eeprom_data(eeprom_write write, int& eeprom_index, uint16_t& working_crc)
{
    EepromWrite eeprom{write, eeprom_index, working_crc};

    preheat.store_eeprom_data(eeprom);
    pid_settings.store_eeprom_data(eeprom);
    eeprom.write(features_);
    eeprom.write(usb_baudrate_);
}

//! Restore presets from permanent memory.
//! @param read Function to use for the actual reading
//! @param eeprom_index
//! @param working_crc
void ADVi3pp_::restore_eeprom_data(eeprom_read read, int& eeprom_index, uint16_t& working_crc)
{
    EepromRead eeprom{read, eeprom_index, working_crc};

    preheat.restore_eeprom_data(eeprom);
    pid_settings.restore_eeprom_data(eeprom);
    eeprom.read(features_);
    eeprom.read(usb_baudrate_);

    dimming.enable(test_one_bit(features_, Feature::Dimming));
    enable_buzzer(test_one_bit(features_, Feature::Buzzer));
    enable_buzz_on_press(test_one_bit(features_, Feature::BuzzOnPress));

    check_and_fix();
}

//! Reset presets.
void ADVi3pp_::reset_eeprom_data()
{
    preheat.reset_eeprom_data();
    pid_settings.reset_eeprom_data();
    features_ = DEFAULT_FEATURES;
    usb_baudrate_ = DEFAULT_USB_BAUDRATE;
}

//! Return the size of data specific to ADVi3++
uint16_t ADVi3pp_::size_of_eeprom_data() const
{
    return
        preheat.size_of_eeprom_data() +
        pid_settings.size_of_eeprom_data() +
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

//! Do not do soo many things in setup so do things here
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

    versions.get_advi3pp_lcd_version();
    versions.send_versions();
    graphs.clear();
    dimming.reset();

    show_boot_page();
}

//! Background idle tasks
void ADVi3pp_::idle()
{
    if(init_)
        init();

    dimming.check();
    read_lcd_serial();
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

    WriteRamDataRequest frame{Variable::TargetBed};
    frame << Uint16(Temperature::target_temperature_bed)
          << Uint16(Temperature::degBed())
          << Uint16(Temperature::target_temperature[0])
          << Uint16(Temperature::degHotend(0))
          << Uint16(scale(fanSpeeds[0], 255, 100))
          << Uint16(lround(LOGICAL_Z_POSITION(current_position[Z_AXIS]) * 100.0))
          << FixedSizeString(get_message(), 40)
          << FixedSizeString(get_progress(), 40)
          << FixedSizeString(get_message(), 44, true)
          << Uint16(progress_bar_low)
          << Uint16(progress_var_high);
    frame.send(false);
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

    Command command; Action action; Uint8 nb_words; Uint16 value;
    frame >> command >> action >> nb_words >> value;
    auto key_value = static_cast<KeyValue>(value.word);

    Log::log() << F(">>> ") << nb_words.byte << F(" words, Action = ") << static_cast<uint16_t>(action) << F(", KeyValue = ") << value.word << Log::endl();

    switch(action)
    {
        case Action::Screen:                screen(key_value); break;
        case Action::PrintCommand:          print_command(key_value); break;
        case Action::Wait:                  pages.handle_wait(key_value); break;
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
        case Action::SensorGrid:            sensor_grid.handle(key_value); break;
        case Action::SensorZHeight:         sensor_z_height.handle(key_value); break;
        case Action::ChangeFilament:        change_filament.handle(key_value); break;
        case Action::EEPROMMismatch:        eeprom_mismatch.handle(key_value); break;
        case Action::Sponsors:              sponsors.handle(key_value); break;
        case Action::LinearAdvanceTuning:   linear_advance_tuning.handle(key_value); break;
        case Action::LinearAdvanceSettings: linear_advance_settings.handle(key_value); break;
        case Action::Diagnosis:             diagnosis.handle(key_value); break;

        case Action::MoveXPlus:             move.x_plus(); break;
        case Action::MoveXMinus:            move.x_minus(); break;
        case Action::MoveYPlus:             move.y_plus(); break;
        case Action::MoveYMinus:            move.y_minus(); break;
        case Action::MoveZPlus:             move.z_plus(); break;
        case Action::MoveZMinus:            move.z_minus(); break;
        case Action::MoveEPlus:             move.e_plus(); break;
        case Action::MoveEMinus:            move.e_minus(); break;
        case Action::LCDBrightness:         dimming.change_brightness(static_cast<int16_t>(key_value)); break;
        case Action::BabyMinus:             print_settings.baby_minus(); break;
        case Action::BabyPlus:              print_settings.baby_plus(); break;

        default:                            Log::error() << F("Invalid action ") << static_cast<uint16_t>(action) << Log::endl(); break;
    }
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Screens
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

void ADVi3pp_::screen(KeyValue key_value)
{
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
        case KeyValue::Back:            back(); break;
        default:                        Log::error() << F("Invalid key value ") << static_cast<uint16_t>(key_value) << Log::endl(); break;
    }
}

//! Show one of the temperature graph screens depending of the context: either the SD printing screen,
//! the printing screen or the temperature screen.
void ADVi3pp_::show_temps()
{
    if(!PrintCounter::isRunning() && !PrintCounter::isPaused())
    {
        pages.show_page(Page::Temperature);
        return;
    }

    // If there is a print running (or paused), display the print screen.
    pages.show_page(Page::Print);
}

//! Show one of the Printing screens depending of the context:
//! - If a print is running, display the Print screen
//! - Otherwise, try to access the SD card. Depending of the result, display the SD card Page or the Temperatures page
void ADVi3pp_::show_print()
{
    // If there is a print running (or paused), display the SD or USB print screen
    if(PrintCounter::isRunning() || PrintCounter::isPaused())
    {
        pages.show_page(Page::Print);
        return;
    }

    pages.show_wait_page(F("Try to access the SD card..."));
    task.set_background_task(BackgroundTask(this, &ADVi3pp_::show_sd_or_temp_page));
}

void ADVi3pp_::show_sd_or_temp_page()
{
    task.clear_background_task();

    card.initsd(); // Can take some time
    ADVi3pp_::reset_message();
    if(!card.cardOK)
    {
        // SD card not accessible so fall back to Temperatures
        pages.show_page(Page::Temperature, false);
        return;
    }

    pages.show_page(Page::SdCard, false);
    sd_card.show_first_page();
}

void ADVi3pp_::back()
{
    pages.show_back_page();
}

//! Handle print commands.
//! @param key_value    The sub-action to handle
void ADVi3pp_::print_command(KeyValue key_value)
{
    if(card.isFileOpen())
        sd_print.handle(key_value);
    else
        usb_print.handle(key_value);
}

//! I-code 0: measure z-height
void ADVi3pp_::icode_0(const GCodeParser& parser)
{
#ifdef ADVi3PP_BLTOUCH

    if(axis_unhomed_error())
    {
        pages.show_back_page();
        return;
    }

    const float old_feedrate_mm_s = feedrate_mm_s;
    feedrate_mm_s = MMM_TO_MMS(XY_PROBE_SPEED);

    do_blocking_move_to(X_BED_SIZE / 2 - X_PROBE_OFFSET_FROM_EXTRUDER,
                        Y_BED_SIZE / 2 - Y_PROBE_OFFSET_FROM_EXTRUDER,
                        Z_CLEARANCE_DEPLOY_PROBE);

    ADVi3pp_::set_status(F("Measuring Z-height..."));
    DEPLOY_PROBE();
    auto zHeight = run_z_probe();
    do_blocking_move_to_z(current_position[Z_AXIS] + Z_CLEARANCE_BETWEEN_PROBES, MMM_TO_MMS(Z_PROBE_SPEED_FAST));
    ADVi3pp_::reset_message();

    feedrate_mm_s = old_feedrate_mm_s;

    sensor_settings.send_z_height_to_lcd(-zHeight);
    pages.show_page(Page::SensorSettings, false);
#endif
}

//! Display the Thermal Runaway Error screen.
void ADVi3pp_::temperature_error(const __FlashStringHelper* message)
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
    return message_.length() > 0;
}

void ADVi3pp_::set_status(const char* message)
{
    Log::log() << F("STATUS: ") << message << Log::endl();
    message_ = message;
}

void ADVi3pp_::set_status(const __FlashStringHelper* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    advi3pp.set_status(fmt, args);
    va_end(args);
}

void ADVi3pp_::set_status_PGM(const char* message)
{
    message_ = String{reinterpret_cast<const __FlashStringHelper*>(message)};
    Log::log() << F("STATUS PGM: ") << message_ << Log::endl();
}

void ADVi3pp_::set_alert_status_PGM(const char* message)
{
    set_status_PGM(message);
}

void ADVi3pp_::status_printf_P(const char * fmt, va_list args)
{
    static const size_t MAX_SIZE = 100;
    static char buffer[MAX_SIZE + 1];

    vsnprintf_P(buffer, MAX_SIZE, fmt, args);
    message_ = String{buffer};
    Log::log() << F("STATUS V: ") << message_ << Log::endl();
}

void ADVi3pp_::set_status(const __FlashStringHelper* fmt, va_list args)
{
    static const size_t MAX_SIZE = 100;
    static char buffer[MAX_SIZE + 1];

    vsnprintf_P(buffer, MAX_SIZE, reinterpret_cast<const char*>(fmt), args);
    message_ = String{buffer};
    Log::log() << F("STATUS V: ") << message_ << Log::endl();
}

const String& ADVi3pp_::get_message() const
{
    return message_;
}

void ADVi3pp_::queue_message(const String &message)
{
    String msg{F("M117 ")}; msg << message;
    enqueue_and_echo_command(msg.c_str());
}

void ADVi3pp_::reset_message()
{
    message_ = "";
}

void ADVi3pp_::set_progress_name(const String& name)
{
    progress_name_ = name;
    progress_percent_ = "";
    percent_ = -1;
}

const String& ADVi3pp_::get_progress() const
{
    if(progress_name_.length() <= 0)
        return progress_name_; // i.e. empty

    auto done = card.percentDone();
    if(done != percent_)
    {
        progress_percent_ = progress_name_ + " " + done + "%";
        percent_ = done;
    }
    return progress_percent_;
}

void ADVi3pp_::reset_progress()
{
    progress_name_ = "";
    progress_percent_ = "";
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
void Task::set_background_task(BackgroundTask task, unsigned int delta)
{
    assert(!background_task_);

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

// --------------------------------------------------------------------

void lcd_status_printf_P(const uint8_t /*level*/, const char * const fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    advi3pp::advi3pp.status_printf_P(fmt, args);
    va_end(args);
}

#if ENABLED(ADVANCED_PAUSE_FEATURE)

void lcd_advanced_pause_show_message(const AdvancedPauseMessage message,
                                     const AdvancedPauseMode mode=ADVANCED_PAUSE_MODE_PAUSE_PRINT,
                                     const uint8_t extruder=active_extruder)
{
    advi3pp::advi3pp.advanced_pause_show_message(message);
}
#endif // ADVANCED_PAUSE_FEATURE

