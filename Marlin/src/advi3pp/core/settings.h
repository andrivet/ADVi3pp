/**
 * ADVi3++ Firmware For Wanhao Duplicator i3 Plus (based on Marlin)
 *
 * Copyright (C) 2017-2020 Sebastien Andrivet [https://github.com/andrivet/]
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

#pragma once

#include <stdint.h>
#include "../../lcd/extui/ui_api.h"
#include "enums.h"
#include "eeprom.h"

namespace ADVi3pp {

//! Hotend, bed temperature and fan speed preset.
struct Preset
{
    uint16_t hotend;
    uint8_t bed;
    uint8_t fan;
};

enum class TemperatureKind: uint8_t { Bed, Hotend };
constexpr const unsigned nb_temperatures = 2;


struct Settings
{
    uint16_t size_of() const;
    bool write(eeprom_write write, int& eeprom_index, uint16_t& working_crc);
    bool read(eeprom_read read, int& eeprom_index, uint16_t& working_crc);
    void reset();
    void mismatch();

    void save(); // Save to EEPROM (i.e. M500)
    void restore();

private:
    Feature features_ = Feature::None;
};

extern Settings settings;

}
