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

#include "../../lib/ADVstd/array.h"
#include "../../core/screen.h"

namespace ADVi3pp {

#ifdef ADVi3PP_PROBE

//! X Twist Tuning Page
struct XTwist: Screen<XTwist> {
  static constexpr Page PAGE = Page::XTwist;
  static constexpr Action ACTION = Action::XTwist;

  enum class Point: uint8_t {L, M, R};

  void minus();
  void plus();

private:
  enum class Multiplier: uint8_t { M1, M2, M3 };

  bool on_dispatch(KeyValue key_value);
  void on_enter();
  void on_save_command();
  void on_back_command();
  void on_abort();

  void post_home_task();
  void multiplier1_command();
  void multiplier2_command();
  void multiplier3_command();
  void move_x(Point x);
  void point_L_command();
  void point_M_command();
  void point_R_command();
  double get_multiplier_value() const;
  void adjust_height(double offset);
  void send_data() const;
  float get_x_mm(Point x) const;

private:
  Multiplier multiplier_ = Multiplier::M1;
  Point point_ = Point::L;
  adv::array<float, ExtUI::xTwistPoints> z_offsets_;

  friend Parent;
};

#else

struct XTwist: Screen<XTwist> {
  static constexpr Page PAGE = Page::NoSensor;
  static constexpr Action ACTION = Action::XTwist;

  void on_mesh_updated(const int8_t xpos, const int8_t ypos, const float zval) {}
  void minus() {}
  void plus() {}
};

#endif

extern XTwist xtwist;

}
