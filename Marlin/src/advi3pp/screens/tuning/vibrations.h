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

//! Vibrations Tuning Page
struct Vibrations: Screen<Vibrations> {
  static constexpr Page PAGE = Page::VibrationsTuning;
  static constexpr Action ACTION = Action::VibrationsTuning;

private:
  bool on_dispatch(KeyValue key_value);
  void on_enter();
  void on_back_command();

private:
  enum class Speed { Slow, Medium, Fast};

  void homing();

  void x_command();
  void y_command();
  void xy_command();
  void yx_command();
  void z_command();

  void move_x();
  void move_y();
  void move_start_xy();
  void move_start_yx();
  void move_xy();
  void move_start_z();
  void move_z();
  void move_finished();
  void move_finished2();

  bool get_values(int &min, int &max);
  void set_values();
  int get_xy_speed();
  int get_z_speed();

  friend Parent;

private:
  Speed speed_ = Speed::Medium;
};

extern Vibrations vibrations;

}
