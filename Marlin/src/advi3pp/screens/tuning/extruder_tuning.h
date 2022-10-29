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

//! Extruder Tuning Page
struct ExtruderTuning: Screen<ExtruderTuning> {
  static constexpr Page PAGE = Page::ExtruderTuningTemp;
  static constexpr Action ACTION = Action::ExtruderTuning;

private:
  bool on_dispatch(KeyValue value);
  void on_enter();
  void on_back_command();
  void on_save_command();

  void start_command();
  void settings_command();
  void send_data();
  void heating();
  bool cancel_heating();
  void extrude();
  void extruding();
  bool cancel_extrude();

private:
  float extruded_ = 0.0;
  float previous_z_ = 0.0;
  friend Parent;
};

extern ExtruderTuning extruder_tuning;

}
