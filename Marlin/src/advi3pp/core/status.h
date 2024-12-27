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

#include <stdint.h>
#include "flash_char.h"
#include "../lib/ADVstd/bitmasks.h"
#include "string.h"
#include "task.h"

namespace ADVi3pp::Status {
  enum class STATUS_OPTIONS { NONE = 0x0000, RESET = 0x0001, PERSISTENT = 0x0002};

  void set(const char* message, STATUS_OPTIONS options = STATUS_OPTIONS::NONE);
  void set(FSTR_P message, STATUS_OPTIONS options = STATUS_OPTIONS::NONE);
  void set_default();
  void reset();
  void init();
}

ENABLE_BITMASK_OPERATOR(ADVi3pp::Status::STATUS_OPTIONS);
