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
#ifndef ADV_I3_PLUS_PLUS_H
#define ADV_I3_PLUS_PLUS_H

#include <stdint.h>
#include <stdarg.h>

class MarlinSettings;
class GCodeParser;
class String;
class __FlashStringHelper;

using eeprom_write = void (*)(int &pos, const uint8_t* value, uint16_t size, uint16_t* crc);
using eeprom_read  = void (*)(int &pos, uint8_t* value, uint16_t size, uint16_t* crc, const bool force);

namespace advi3pp {

enum class Page: uint8_t
{
    None                    = 0,
    Boot                    = 1,
    Main                    = 22,
    Controls                = 24,
    Tuning                  = 26,
    Settings                = 28,
    LoadUnload              = 30,
    WaitBack                = 32,
    WaitBackContinue        = 34,
    Preheat                 = 36,
    Move                    = 38,
    SdCard                  = 40,
    Print                   = 42,
    Sponsors                = 44,
    Waiting                 = 46,
    ManualLeveling          = 48,
    ExtruderTuningTemp      = 50,
    WaitContinue            = 52,
    ExtruderTuningMeasure   = 54,
    PidTuning1              = 58,
    PidTuning2              = 60,
    MotorsSettings          = 62,
    PidSettings             = 64,
    FactoryReset            = 66,
    Statistics              = 68,
    Versions                = 70,
    StepsSettings           = 72,
    FeedrateSettings        = 74,
    AccelerationSettings    = 76,
    JerkSettings            = 78,
    PrintSettings           = 80,
    ThermalRunawayError     = 82,
    VersionsMismatch        = 84,
    Temperature             = 86,
    Infos                   = 88,
    Firmware                = 90,
    NoSensor                = 92,
    SensorSettings          = 94,
    LCD                     = 96,
    Copyrights              = 98,
    SensorTuning            = 100,
    SensorGrid              = 102,
    EEPROMMismatch          = 104,
    ZHeightTuning           = 106
};

//! The Duplicator i3 Plus printer.
struct Printer
{
    static void setup();
    static void task();
    static void auto_pid_finished();
    static void g29_leveling_finished(bool success);
    static void store_eeprom_data(eeprom_write write, int& eeprom_index, uint16_t& working_crc);
    static void restore_eeprom_data(eeprom_read read, int& eeprom_index, uint16_t& working_crc);
    static void reset_eeprom_data();
    static uint16_t size_of_eeprom_data();
    static void eeprom_settings_mismatch();
    static void save_settings();
    static void temperature_error(const __FlashStringHelper* message);
    static void update();
    static bool is_thermal_protection_enabled();
    static void process_command(const GCodeParser& parser);
};

//! The Duplicator i3 Plus LCD Screen
struct LCD
{
    static void update();
    static void init();
    static bool has_status();
    static void set_status(const char* message, bool persist);
    static void set_status_PGM(const char* message, int8_t level = 0);
    static void set_status(const __FlashStringHelper* fmt, ...);
    static void set_alert_status_PGM(const char* message);
    static void buttons_update();
    static void reset_alert_level();
    static bool detected();
    static void refresh();
    static void queue_message(const String& message);
    static void reset_message();
    static void enable_buzzer(bool enable);
    static void enable_buzz_on_press(bool enable);
    static void buzz(long duration, uint16_t frequency = 0);
    static void buzz_on_press();
};

}

#endif
