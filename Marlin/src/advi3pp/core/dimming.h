/**
 * ADVi3++ Firmware For Wanhao Duplicator i3 Plus (based on Marlin 2)
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
#include "settings.h"

namespace ADVi3pp {

//! LCD screen brightness and dimming
struct Dimming : Settings<Dimming> {
  Dimming();

  bool receive();
  void send();
  void sleep_on();
  void sleep_off();
  void send_brightness_to_lcd();
  void send_brightness_to_lcd(uint8_t brightness);

  bool is_enabled() const { return enabled_; }
  uint16_t get_dimming_time() const { return dimming_time_; }
  uint8_t get_normal_brightness() const { return ui.brightness; }
  uint8_t get_dimming_brightness() const { return dimming_brightness_; }
  void set_settings(bool dimming, uint8_t dimming_time, uint8_t normal_brightness, uint8_t dimming_brightness);

private:
  friend Parent;

  void do_write(EepromWrite& eeprom) const;
  bool do_validate(EepromRead& eeprom);
  void do_read(EepromRead& eeprom);
  void do_reset();
  uint16_t do_size_of() const;

private:
  void set_next_checking_time();

private:
  bool dimmed_ = false;
  uint32_t next_check_time_ = 0;
  bool enabled_ = true;
  uint8_t dimming_brightness_ = 5;
  uint8_t dimming_time_ = 2;
};

extern Dimming dimming;

}
