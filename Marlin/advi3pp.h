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
class String;
using eeprom_write = void (*)(int &pos, const uint8_t* value, uint16_t size, uint16_t* crc);
using eeprom_read  = void (*)(int &pos, uint8_t* value, uint16_t size, uint16_t* crc);

namespace advi3pp {

enum class Page: uint8_t
{
    None                    = 0,
    Boot                    = 1,
    Main                    = 22,
    Controls                = 24,
    Calibration             = 26,
    Settings                = 28,
    LoadUnload              = 30,
    Load2                   = 32,
    Unload2                 = 34,
    Preheat                 = 36,
    Move                    = 38,
    SdCard                  = 40,
    SdPrint                 = 42,
    UsbPrint                = 44,
    Leveling1               = 46,
    Leveling2               = 48,
    ExtruderCalibration1    = 50,
    ExtruderCalibration2    = 52,
    ExtruderCalibration3    = 54,
    XYZMotorsCalibration    = 56,
    PidTuning1              = 58,
    PidTuning2              = 60,
    MotorsSettings          = 62,
    PidSettings             = 64,
    FactoryReset            = 66,
    Statistics              = 68,
    About                   = 70,
    StepsSettings           = 72,
    FeedrateSettings        = 74,
    AccelerationSettings    = 76,
    JerkSettings            = 78,
    PrintSettings           = 80,
    ThermalRunawayError     = 82,
    Mismatch                = 84,
    Temperature             = 86
};

//! The Duplicator i3 Plus printer.
struct Printer
{
    static void setup();
    static void task();
    static void auto_pid_finished();
    static void store_presets(eeprom_write write, int& eeprom_index, uint16_t& working_crc);
    static void restore_presets(eeprom_read read, int& eeprom_index, uint16_t& working_crc);
    static void reset_presets();
    static void temperature_error();
    static void send_temperatures_data();
};

//! The Duplicator i3 Plus LCD Screen
struct LCD
{
    static void update();
    static void init();
    static bool has_status();
    static void set_status(const char* message, bool persist);
    static void set_status_PGM(const char* message, int8_t level);
    static void set_alert_status_PGM(const char* message);
    static void buttons_update();
    static void reset_alert_level();
    static bool detected();
    static void refresh();
    static void queue_message(const String& message);
    static void reset_message();
};

}

#endif
