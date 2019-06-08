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
#ifndef ADV_I3_PLUS_PLUS_H
#define ADV_I3_PLUS_PLUS_H

#include <stdint.h>
#include <stdarg.h>
#include "enum.h"
#include "advi3pp_pages.h"

class MarlinSettings;
class GCodeParser;
class __FlashStringHelper;
using FlashChar = __FlashStringHelper;

using eeprom_write = void (*)(int &pos, const uint8_t* value, uint16_t size, uint16_t* crc);
using eeprom_read  = void (*)(int &pos, uint8_t* value, uint16_t size, uint16_t* crc, const bool force);

namespace advi3pp {

enum class TemperatureKind: uint8_t { Bed, Hotend };

//! ADVi3++ public facade, i.e. interface between Marlin code and ADVi3++ code
struct ADVi3pp
{
    static void setup_lcd_serial();
    static void change_baudrate();
    static void setup();
    static void idle();
    static void auto_pid_finished(bool success);
    static void g29_leveling_finished(bool success);
    static void pause_finished(bool success);
    static void write(eeprom_write write, int& eeprom_index, uint16_t& working_crc);
    static bool read(eeprom_read read, int& eeprom_index, uint16_t& working_crc);
    static void reset();
    static uint16_t size_of();
    static void eeprom_settings_mismatch();
    static void temperature_error(const FlashChar* message);
    static bool is_thermal_protection_enabled();
    static void set_brightness(int16_t britghness);
    static bool has_status();
    static void set_status(const char* message);
    static void set_status(const FlashChar* message);
    static void set_status(const char* fmt, va_list& args);
    static void set_status(const FlashChar* fmt, va_list& args);
    static void set_status_v(const FlashChar* fmt, ...);
    static void advanced_pause_show_message(AdvancedPauseMessage message);
    static void reset_status();
    static void buzz(long duration, uint16_t frequency = 0);
    static void on_set_temperature(TemperatureKind kind, uint16_t temperature);
    static void stop_and_wait();
    static void process_command(const GCodeParser& parser);

#ifdef ADVi3PP_PROBE
    static double x_probe_offset_from_extruder();
    static double y_probe_offset_from_extruder();
    static int left_probe_bed_position();
    static int right_probe_bed_position();
    static int front_probe_bed_position();
    static int back_probe_bed_position();
#endif
};

}

#endif
