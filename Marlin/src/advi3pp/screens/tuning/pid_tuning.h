/**
 * ADVi3++ Firmware For Wanhao Duplicator i3 Plus (based on Marlin 2)
 *
 * Copyright (C) 2017-2025 Sebastien Andrivet [https://github.com/andrivet/]
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
#include "../../core/pages.h"
#include "../../../lcd/extui/ui_api.h"

namespace ADVi3pp::PidTuning {
  enum struct RUNNING { NO, EXTRUDER, BED };

  bool handle_command(uint16_t key_code);
  RUNNING is_running();
  void on_start(bool bed);
  void on_progress(int cycleIndex, int nbCycles);
  void on_finished(ExtUI::pidresult_t result);
}
