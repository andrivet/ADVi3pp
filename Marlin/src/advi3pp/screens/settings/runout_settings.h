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

#include "../../core/screen.h"

namespace ADVi3pp {

//! Runout Setting Page
struct RunoutSettings: Screen<RunoutSettings> {
  static constexpr Page PAGE = Page::Runout;
  static constexpr Action ACTION = Action::Runout;

private:
  bool on_dispatch(KeyValue key_value);
  void on_enter();
  void on_back_command();
  void on_save_command();

  void enable_command();
  void high2low_command();
  void low2high_command();
  uint16_t get_filament_state();
  void send_data();

  friend Parent;

private:
  bool inverted_ = false;
};

extern RunoutSettings runout_settings;

}
