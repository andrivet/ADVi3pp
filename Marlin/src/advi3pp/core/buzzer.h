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


struct Buzzer: Settings<Buzzer> {
  void buzz(uint8_t duration);
  void buzz_on_action();
  void buzz_on_action(uint8_t duration);
  void buzz_on_press();

  bool is_buzz_on_action_enabled() const { return buzz_on_action_; }
  bool is_buzz_on_press_enabled() const { return buzz_on_press_; }
  uint8_t get_buzz_duration() const { return buzz_duration_; }
  void set_settings(bool buzz_on_action, bool buzz_on_press, uint8_t buzz_duration);

private:
  friend Parent;

  void do_write(EepromWrite& eeprom) const;
  bool do_validate(EepromRead& eeprom);
  void do_read(EepromRead& eeprom);
  void do_reset();
  uint16_t do_size_of() const;

private:
    void send_buzz_command_to_lcd();
    void send_buzz_command_to_lcd(uint8_t duration);

private:
  bool buzz_on_action_ = true;
  bool buzz_on_press_ = false;
  uint8_t buzz_duration_ = 1; // x 10ms
};

extern Buzzer buzzer;

}