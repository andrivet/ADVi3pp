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

#include "../parameters.h"
#include <Arduino.h>
#include "task.h"

namespace ADVi3pp {

Task task;

Task::Task(const Callback& callback, unsigned int delay, Activation activation)
: delay_{delay}, activation_{activation}, callback_{callback} {
    set_next_execute_time();
}

//! Set the next task and its delay
//! @param task     The next background task
//! @param delta    Duration to be added to the current time to execute the background task
void Task::set(const Callback& callback, unsigned int delay, Activation activation)
{
    delay_ = delay;
    activation_ = activation;
    callback_ = callback;
    set_next_execute_time();
}

//! Reset the background task
void Task::clear()
{
    callback_ = nullptr;
}

//! If there is an operating running, execute its next step
bool Task::execute(bool force_execute)
{
    if(!callback_)
        return false;

    if(!force_execute && !ELAPSED(millis(), next_execute_time_))
        return false;

    // Clear before calling to avoid reentrancy issues
    Callback callback{callback_};
    if(activation_ == Activation::ONE_TIME)
      callback_ = nullptr;
    else
      set_next_execute_time();

    callback();
    return true;
}

void Task::set_next_execute_time() {
    next_execute_time_ = millis() + delay_;
}

}
