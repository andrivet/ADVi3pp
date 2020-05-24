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

#include "settings.h"
#include "../versions.h"
#include "../core/dimming.h"
#include "../core/buzzer.h"
#include "../screens/controls/preheat.h"
#include "../screens/settings/sensor_settings.h"
#include "../screens/settings/pid_settings.h"

namespace ADVi3pp {

const Feature DEFAULT_FEATURES =
        Feature::HeadParking |
        Feature::Dimming |
        Feature::Buzzer;

Settings settings;

bool Settings::write(eeprom_write write, int& eeprom_index, uint16_t& working_crc)
{
    EepromWrite eeprom{write, eeprom_index, working_crc};

    eeprom.write(settings_version);
    preheat.write(eeprom);
    sensor_settings.write(eeprom);
    pid_settings.write(eeprom);
    eeprom.write(features_);

    return true;
}

bool Settings::read(eeprom_read read, int& eeprom_index, uint16_t& working_crc)
{
    EepromRead eeprom{read, eeprom_index, working_crc};

    uint16_t version = 0;
    eeprom.read(version);
    preheat.read(eeprom);
    sensor_settings.read(eeprom);
    pid_settings.read(eeprom);
    eeprom.read(features_);

    dimming.enable(test_one_bit(features_, Feature::Dimming), false);
    buzzer.enable(test_one_bit(features_, Feature::Buzzer));
    buzzer.enable_on_press(test_one_bit(features_, Feature::BuzzOnPress), false);

    return version == settings_version;
}

//! Reset presets.
void Settings::reset()
{
    preheat.reset();
    sensor_settings.reset();
    pid_settings.reset();
    features_ = DEFAULT_FEATURES;
}

//! Return the size of data specific to ADVi3++
uint16_t Settings::size_of() const
{
    return
            sizeof(settings_version) +
            preheat.size_of() +
            sensor_settings.size_of() +
            pid_settings.size_of() +
            sizeof(features_);
}

void Settings::save()
{
    ExtUI::injectCommands_P(PSTR("M500"));
}

}
