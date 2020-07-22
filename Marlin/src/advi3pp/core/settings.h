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


enum class TemperatureKind: uint8_t { Bed, Hotend };
constexpr const unsigned nb_temperatures = 2;

const uint16_t default_bed_temperature = 50; //!< Default target temperature for the bed
const uint16_t default_hotend_temperature = 200; //!< Default target temperature for the hotend

struct Settings
{
    uint16_t size_of() const;
    bool write(eeprom_write write, int& eeprom_index, uint16_t& working_crc);
    void read(eeprom_read read, int& eeprom_index, uint16_t& working_crc);
    bool validate(eeprom_read read, int& eeprom_index, uint16_t& working_crc);
    void reset();

    void save();
    void restore();

    Feature flip_features(Feature features);
    void send_lcd_values(Variable features);
    bool is_feature_enabled(Feature features) const;

    uint16_t get_last_used_temperature(TemperatureKind kind) const;
    void on_set_temperature(TemperatureKind kind, uint16_t temperature);

private:
    const Feature DEFAULT_FEATURES =
            Feature::Dimming |
            Feature::BuzzOnAction;

    Feature features_ = DEFAULT_FEATURES;
    uint16_t last_used_temperature_[nb_temperatures] = {default_bed_temperature, default_hotend_temperature};
};

extern Settings settings;

}
