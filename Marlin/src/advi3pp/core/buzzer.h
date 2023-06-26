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
  enum class OPTIONS: uint16_t {
    NONE = 0,
    ON_TOUCH = 1,
    ON_ACTION = 1 << 1
  };

  void buzz(uint8_t duration);
  void buzz_on_action();
  void buzz_on_action(uint8_t duration);
  void buzz_on_press();

  bool is_option_enabled(OPTIONS option) {
    return (ui.tone_options & static_cast<uint16_t>(option)) != 0;
  }

private:
  friend Parent;

private:
    void send_buzz_command_to_lcd();
    void send_buzz_command_to_lcd(uint8_t duration);

private:
  bool buzz_on_action_ = true;
  bool buzz_on_press_ = false;
};

extern Buzzer buzzer;

}