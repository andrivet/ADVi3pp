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

//! Automatic Leveling Page
struct AutomaticLeveling: Screen<AutomaticLeveling> {
#ifdef ADVi3PP_PROBE
  static constexpr Page PAGE = Page::AutomaticLeveling;
  static constexpr Action ACTION = Action::AutomaticLeveling;
#else
  static constexpr Page PAGE = Page::NoSensor;
  static constexpr Action ACTION = Action::None;
#endif

  void on_start() {}
  void on_progress(uint8_t index, uint8_t x, uint8_t y);
  void on_done(bool success);

private:
  bool on_dispatch(KeyValue key_value);
  void on_enter();
  void on_back_command();
  void on_abort();

  void reset_command();
  void start();
  void home_task();

private:
  bool lcd_leveling_ = false;

  friend Parent;
};

extern AutomaticLeveling automatic_leveling;

}
