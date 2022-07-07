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

namespace ADVi3pp {

//! Move Page
struct Move: Screen<Move> {
  friend Parent;

  void x_plus_command();
  void x_minus_command();
  void x_home_command();
  void y_plus_command();
  void y_minus_command();
  void y_home_command();
  void z_plus_command();
  void z_minus_command();
  void z_home_command();
  void e_plus_command();
  void e_minus_command();
  void all_home_command();
  void disable_motors_command();

private:
  enum class Direction: uint8_t {
    None,
    X_PLUS,
    Y_PLUS,
    Z_PLUS,
    E_PLUS,
    MINUS = 0x80,
    X_MINUS = MINUS + X_PLUS,
    Y_MINUS = MINUS + Y_PLUS,
    Z_MINUS = MINUS + Z_PLUS,
    E_MINUS = MINUS + E_PLUS
  };

  bool do_dispatch(KeyValue key_value);
  Page do_prepare_page();
  void move(Move::Direction movement);
  void reset_move();
  float getPosition() const;
  void setPosition() const;
  float get_min() const;
  float get_max() const;
  float get_offset() const;
  feedRate_t get_feedrate() const;
  void task();

private:
  Direction direction_ = Direction::None;
  millis_t last_click_time_ = 0, target_move_time_ = 0;
  float target_position_ = 0;
};

extern Move move;

}
