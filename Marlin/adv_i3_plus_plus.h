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


class MarlinSettings;
using eeprom_write = void (*)(int &pos, const uint8_t* value, uint16_t size, uint16_t* crc);
using eeprom_read  = void (*)(int &pos, uint8_t* value, uint16_t size, uint16_t* crc);

namespace advi3pp
{

    enum class Page: uint8_t
    {
        None                = 0,
        Boot                = 1,
        Main                = 21,
        SdCard              = 31,
        Print               = 33,
        PrintSettings       = 35,
        Tools               = 37,
        Preheat             = 39,
        Move                = 41,
        System              = 43,
        PidSettings         = 45,
        MotoSettings        = 47,
        Filament            = 49,
        Unload              = 51,
        Load                = 53,
        LevelingStart       = 55,
        Leveling            = 56,
        LcdUpdate           = 58,
        Statistics          = 59,
        AutoPidTuning       = 61,
        Temperature         = 63,
        AutoPidGraph        = 65,
        AutoPidFinished     = 66,
        ThermalRunawayError = 68,
        About               = 69,
        FactoryReset        = 71,
        Mismatch            = 73
    };

    //!
    //! The Duplicator i3 Plus printer and its LCD screen
    //!
    struct i3PlusPrinter
    {
        static void setup();
        static void task();
        static void show_page(Page page);
        static void update_graph_data();
        static void auto_pid_finished();
        static void store_presets(eeprom_write write, int eeprom_index, uint16_t& working_crc);
        static void restore_presets(eeprom_read read, int eeprom_index, uint16_t& working_crc);
        static void reset_presets();
        static void temperature_error();
    };

}

#endif
