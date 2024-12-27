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
#include "../../inc/MarlinConfig.h"
#include "task.h"


namespace ADVi3pp {

  Task background_task{DEFAULT_TASK_DELAY}; // Primary task
  Task status_task{DEFAULT_TASK_DELAY}; // To update status task
  Task wait_task{DEFAULT_TASK_DELAY}; // For all wait screens

  Elapse::Elapse(unsigned delay): delay_{delay} {
    reset();
  }

  bool Elapse::is_elapsed(bool force) {
    if(!force && !ELAPSED(millis(), next_execute_time_)) return false;
    reset();
    return true;
  }

  bool Elapse::is_pending(bool force) {
    if(!force && !PENDING(millis(), next_execute_time_)) return false;
    reset();
    return true;
  }

  void Elapse::reset(unsigned delay) {
    if(delay > 0) delay_ = delay;
    next_execute_time_ = millis() + delay_;
  }

  Task::Task(unsigned delay): elapse_{delay} {}

  //! Set the next task and its delay
  //! @param task     The next background task
  void Task::set(CALLBACK_RESULT (*callback)(), unsigned delay) {
    callback_ = callback;
    elapse_.reset(delay);
  }

  void Task::rearm() {
    elapse_.reset();
  }

  //! Reset the background task
  void Task::clear() {
    callback_ = nullptr;
  }

  //! If there is an operating running, execute its next step
  bool Task::execute(bool force_execute) {
    if(!callback_) return false;
    if(!elapse_.is_elapsed(force_execute)) return false;

    // Clear before calling to avoid reentrancy issues
    auto copy{callback_};
    clear();
    if(copy() == CALLBACK_RESULT::CONTINUE) {
      assert(!callback_);  // Be sure it has not been set in the meantime
      callback_ = copy;
    }
    return true;
  }
}
