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

#include "../../lib/ADVstd/bitmasks.h"
#include "../../core/screen.h"

namespace ADVi3pp {

#ifdef BLTOUCH
//! BLTouch Testing Page
struct BLTouchTesting: Screen<BLTouchTesting> {
  static constexpr Page PAGE = Page::BLTouchTesting1A;
  static constexpr Action ACTION = Action::BLTouchTesting;

private:
  enum class Wires: uint8_t {
    None    = 0b00000000,
    Brown   = 0b00000001,
    Red     = 0b00000010,
    Orange  = 0b00000100,
    Black   = 0b00001000,
    White   = 0b00010000
  };

private:
  bool on_dispatch(KeyValue key_value);
  bool on_enter();
  void on_back_command();
  void on_save_command();
  void on_abort();

  void step_1a();
  void step_1a_yes();
  void step_1a_no();
  void step_1b();
  void step_1b_slow();
  void step_1b_quick();
  void step_1b_no();
  void step_2();
  void step_2_yes();
  void step_2_no();
  void step_3();
  void step_4();
  void step_3_task();
  uint16_t wire_value(Wires wire);

private:
  Wires tested_ = Wires::None;
  Wires ok_ = Wires::None;

  friend Parent;
};

ENABLE_BITMASK_OPERATOR(BLTouchTesting::Wires);

inline uint16_t BLTouchTesting::wire_value(Wires wire) {
  return test_one_bit(ok_, wire) ? 1 : test_one_bit(tested_, wire) ? 2 : 0;
}

#else
//! BLTouch Testing Page
struct BLTouchTesting: Screen<BLTouchTesting> {
  static constexpr Page PAGE = Page::NoSensor;
  static constexpr Action ACTION = Action::BLTouchTesting;
};
#endif

extern BLTouchTesting bltouch_testing;

}
