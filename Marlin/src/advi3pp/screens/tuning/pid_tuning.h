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

//! PID Tuning Page
struct PidTuning: Screen<PidTuning> {
  static constexpr Page PAGE = Page::PidTuning;
  static constexpr Action ACTION = Action::PidTuning;

  void on_start();
  void on_progress(int cycleIndex, int nbCycles);
  void on_finished(ExtUI::result_t result);
  void send_data();

private:
  bool on_dispatch(KeyValue value);
  bool on_enter();

  void step2_command();
  void step3_command();
  void cancel_pid();
  void hotend_command();
  void bed_command();

private:
  enum class State: uint8_t {
    None,
    Processing,
    FromLCDMenu = 0x80
  };

  uint16_t temperature_ = 0;
  TemperatureKind kind_ = TemperatureKind::Hotend;
  State state_ = State::None;

  friend Parent;
};

ENABLE_BITMASK_OPERATOR(PidTuning::State);

extern PidTuning pid_tuning;

}
