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

#include "../parameters.h"
#include "settings.h"
#include "../versions.h"
#include "../core/core.h"
#include "../core/dgus.h"
#include "../core/dimming.h"
#include "../core/buzzer.h"
#include "../screens/controls/preheat.h"
#include "../screens/settings/sensor_settings.h"
#include "../screens/settings/pid_settings.h"
#include "../screens/settings/eeprom_mismatch.h"


namespace ADVi3pp {

Settings settings;

bool Settings::write(eeprom_write write, int& eeprom_index, uint16_t& working_crc)
{
    EepromWrite eeprom{write, eeprom_index, working_crc};

    eeprom.write(settings_version);
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
    sensor_settings.read(eeprom);
    pid_settings.read(eeprom);
    eeprom.read(features_);

    return version == settings_version;
}

//! Reset presets.
void Settings::reset()
{
    sensor_settings.reset();
    pid_settings.reset();
    features_ = DEFAULT_FEATURES;
}

//! Return the size of data specific to ADVi3++
uint16_t Settings::size_of() const
{
    return
            sizeof(settings_version) +
            sensor_settings.size_of() +
            pid_settings.size_of() +
            sizeof(features_);
}

//! Inform the user that the EEPROM data are not compatible and have been reset
void Settings::mismatch()
{
    // It is not possible to show the Mismatch page now since nothing is yet initialized.
    // It will be done in the setup method.
    eeprom_mismatch.set_mismatch();
}

//! Save the current settings permanently in EEPROM memory
void Settings::save()
{
    eeprom_mismatch.reset_mismatch();
    core.inject_commands(F("M500"));
}

//! Restore settings from EEPROM memory
void Settings::restore()
{
    // Note: Previously, M420 (bed leveling compensation) was reset by M501. It is no more the case.
    core.inject_commands(F("M501"));
}

Feature Settings::flip_features(Feature features)
{
    flip_bits(features_, features);
    return features_ & features;
}

bool Settings::is_feature_enabled(Feature features) const
{
    return test_all_bits(features_, features);
}

void Settings::send_lcd_values(Variable features)
{
    WriteRamDataRequest frame{features};
    frame << Uint16(static_cast<uint16_t>(features_)) << Uint16(ExtUI::get_lcd_contrast());
    frame.send();
}

//! Get the last used temperature for the hotend or the bad
//! @param kind Kind of temperature: hotend or bed
//! @return The last used themperature
uint16_t Settings::get_last_used_temperature(TemperatureKind kind) const
{
    return last_used_temperature_[kind == TemperatureKind::Hotend];
}

//! To be called when a new temperature is selected as a target
//! @param kind Kind of temperature: hotend or bed
//! @param temperature The new target temperature
void Settings::on_set_temperature(TemperatureKind kind, uint16_t temperature)
{
    if(temperature == 0)
        return;
    last_used_temperature_[kind == TemperatureKind::Hotend] = temperature;
    pid_settings.set_best_pid(kind, temperature);
}


}
