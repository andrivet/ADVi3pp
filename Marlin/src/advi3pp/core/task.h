/**
 * ADVi3++ Firmware For Wanhao Duplicator i3 Plus (based on Marlin 2)
 *
 * Copyright (C) 2017-2020 Sebastien Andrivet [https://github.com/andrivet/]
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

#include "../../core/millis_t.h"
#include "../lib/ADVstd/ADVcallback.h"
#include "callback.h"

namespace ADVi3pp {
  constexpr unsigned DEFAULT_TASK_DELAY = 100; // ms

  //! Measure elapsed time, D = delay
  struct Elapse {
    unsigned delay_ = DEFAULT_TASK_DELAY;
    millis_t next_execute_time_ = 0;

    explicit Elapse(unsigned delay);

    bool is_elapsed(bool force = false);
    bool is_pending(bool force = false);
    void reset(unsigned delay = 0);
  };

  //! Task
  struct Task {
    explicit Task(unsigned delay = DEFAULT_TASK_DELAY);

    void set(CALLBACK_RESULT (*callback)(), unsigned delay = DEFAULT_TASK_DELAY);
    void rearm();
    void clear();
    bool execute(bool force_execute = false);

  private:
    Elapse elapse_;
    CALLBACK_RESULT (*callback_)() {nullptr};
  };

  extern Task background_task; // Primary task
  extern Task status_task; // To update status task
  extern Task wait_task; // For all wait screens
}

