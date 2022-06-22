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

#include "../core/screen.h"
#include "../../core/buzzer.h"

namespace ADVi3pp {

//! Beeper Setting Page
struct BeeperSettings: Screen<BeeperSettings>
{
  void duration_command(uint16_t duration);

private:
  friend Parent;

  bool do_dispatch(KeyValue key_value);
  Page do_prepare_page();
  void do_save_command();
  void send_values() const;
  bool get_values();

  void on_action_command();
  void on_press_command();

  bool buzz_on_action_ = true;
  bool buzz_on_press_ = false;
  uint8_t buzz_duration_ = 1; // x 10ms
};

extern BeeperSettings beeper_settings;

}
