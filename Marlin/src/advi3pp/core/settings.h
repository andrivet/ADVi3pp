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

#pragma once

#include <stdint.h>
#include "../lib/ADVstd/ADVcrtp.h"
#include "../lib/ADVstd/array.h"
#include "../../lcd/extui/ui_api.h"
#include "enums.h"
#include "eeprom.h"

namespace ADVi3pp {

enum class TemperatureKind: uint8_t { Bed, Hotend };
constexpr const unsigned nb_temperatures = 2;

const uint16_t default_bed_temperature = 50; //!< Default target temperature for the bed
const uint16_t default_hotend_temperature = 200; //!< Default target temperature for the hotend


template<typename Self>
struct Settings : adv::Crtp<Self, Settings> {
  void write(EepromWrite& eeprom) const { this->self().do_write(eeprom); }
  void read(EepromRead& eeprom) { this->self().do_read(eeprom); }
  bool validate(EepromRead& eeprom) { return this->self().do_validate(eeprom); }
  void reset() { this->self().do_reset(); }
  uint16_t size_of() const { return this->self().do_size_of(); }
};


struct ExtendedSettings {
  void on_factory_reset();
  bool does_eeprom_mismatch() const { return eeprom_mismatch_; }

  uint16_t on_sizeof_settings();
  void on_store_settings(ExtUI::eeprom_write write, int& eeprom_index, uint16_t& working_crc);
  bool on_load_settings(ExtUI::eeprom_read read, int& eeprom_index, uint16_t& working_crc, bool validating);
  void on_settings_written(bool success);
  void on_settings_loaded(bool success);
  void on_settings_validated(bool success);

  uint16_t size_of() const;
  bool write(eeprom_write write, int& eeprom_index, uint16_t& working_crc);
  void read(eeprom_read read, int& eeprom_index, uint16_t& working_crc);
  bool validate(eeprom_read read, int& eeprom_index, uint16_t& working_crc);
  void reset();

  void save();
  void restore();

private:
  bool eeprom_mismatch_ = false;
};

extern ExtendedSettings settings;

}
