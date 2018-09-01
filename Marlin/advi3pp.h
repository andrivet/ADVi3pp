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
#include "advi3pp_pages.h"

class MarlinSettings;
class GCodeParser;
class String;
class __FlashStringHelper;

using eeprom_write = void (*)(int &pos, const uint8_t* value, uint16_t size, uint16_t* crc);
using eeprom_read  = void (*)(int &pos, uint8_t* value, uint16_t size, uint16_t* crc, const bool force);

namespace advi3pp {

//! ADVi3++ public facade.
struct ADVi3pp
{
    static void setup();
    static void idle();
    static void auto_pid_finished();
    static void g29_leveling_finished(bool success);
    static void store_eeprom_data(eeprom_write write, int& eeprom_index, uint16_t& working_crc);
    static void restore_eeprom_data(eeprom_read read, int& eeprom_index, uint16_t& working_crc);
    static void reset_eeprom_data();
    static uint16_t size_of_eeprom_data();
    static void eeprom_settings_mismatch();
    static void temperature_error(const __FlashStringHelper* message);
    static bool is_thermal_protection_enabled();
    static void process_command();
    static void set_brightness(int16_t britghness);
    static bool has_status();
    static void set_status(const char* message, bool persist);
    static void set_status_PGM(const char* message, int8_t level = 0);
    static void set_alert_status_PGM(const char* message);
    static void reset_message();
    static void buzz(long duration, uint16_t frequency = 0);
};

}

#endif
