/**
 * ADVi3++ Firmware For Wanhao Duplicator i3 Plus (based on Marlin)
 *
 * Copyright (C) 2017-2022 Sebastien Andrivet [https://github.com/andrivet/]
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

#include "../../inc/MarlinConfig.h"
#include "settings.h"
#include "../core/core.h"
#include "../core/dgus.h"
#include "../core/pages.h"
#include "../core/dimming.h"
#include "../core/pid.h"
#include "../core/buzzer.h"
#include "../screens/tuning/setup.h"


namespace ADVi3pp {

ExtendedSettings settings;


void ExtendedSettings::on_factory_reset() {
  settings.reset();
  if(does_eeprom_mismatch())
    return;

  pages.reset();
  setup.show();
}

void ExtendedSettings::on_store_settings(ExtUI::eeprom_write write, int& eeprom_index, uint16_t& working_crc) {
  settings.write(write, eeprom_index, working_crc);
}

bool ExtendedSettings::on_load_settings(ExtUI::eeprom_read read, int& eeprom_index, uint16_t& working_crc, bool validating) {
  if(validating)
    return settings.validate(read, eeprom_index, working_crc);
  settings.read(read, eeprom_index, working_crc);
  return true;
}

uint16_t ExtendedSettings::on_sizeof_settings() {
  return settings.size_of();
}

void ExtendedSettings::on_settings_written(bool success) {}

void ExtendedSettings::on_settings_loaded(bool success) {
  if(!success)
    eeprom_mismatch_ = true;
}

void ExtendedSettings::on_settings_validated(bool success) {
  if(!success)
    eeprom_mismatch_ = true;
}

bool ExtendedSettings::write(eeprom_write write, int& eeprom_index, uint16_t& working_crc) {
  EepromWrite eeprom{write, eeprom_index, working_crc};

  eeprom.write(settings_version);
  pid.write(eeprom);
  dimming.write(eeprom);

  return true;
}

bool ExtendedSettings::validate(eeprom_read read, int& eeprom_index, uint16_t& working_crc) {
  bool valid = true;
  EepromRead eeprom{read, eeprom_index, working_crc};

  uint16_t version = 0;
  eeprom.read(version);
  if(version != settings_version)
    valid = false;

  if(!pid.validate(eeprom) ||
     !dimming.validate(eeprom)
  )
    valid = false;

  return valid;
}

void ExtendedSettings::read(eeprom_read read, int& eeprom_index, uint16_t& working_crc) {
  EepromRead eeprom{read, eeprom_index, working_crc};

  uint16_t version = 0;
  eeprom.read(version);
  pid.read(eeprom);
  dimming.read(eeprom);
}

//! Reset presets.
void ExtendedSettings::reset() {
  pid.reset();
  dimming.reset();
}

//! Return the size of data specific to ADVi3++
uint16_t ExtendedSettings::size_of() const {
  return
    sizeof(settings_version) +
    pid.size_of() +
    dimming.size_of();
}

//! Save the current settings permanently in EEPROM memory
void ExtendedSettings::save() {
  eeprom_mismatch_ = false;
  ExtUI::saveSettings();
}

//! Restore settings from EEPROM memory
void ExtendedSettings::restore() {
  // Note: Previously, M420 (bed leveling compensation) was reset by M501. It is no more the case.
  ExtUI::loadSettings();
}

}
