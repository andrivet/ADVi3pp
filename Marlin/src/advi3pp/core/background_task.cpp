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

#include <Arduino.h>
#include "background_task.h"

namespace ADVi3pp {

Task task;

//! Set the next (minimal) update time
//! @param delta    Duration to be added to the current time to compute the next (minimal) update time
void Task::set_next_update_time(unsigned int delta)
{
    next_update_time_ = millis() + delta;
}

//! Is it the time to update?
//! @return true if it is the time to update
bool Task::is_update_time()
{
    auto current_time = millis();
    if(!ELAPSED(current_time, next_update_time_))
        return false;
    set_next_update_time();
    return true;
}

//! Set the next background task and its delay
//! @param task     The next background task
//! @param delta    Duration to be added to the current time to execute the background task
void Task::set_background_task(const BackgroundTask& task, unsigned int delta)
{
    op_time_delta_ = delta;
    background_task_ = task;
    next_op_time_ = millis() + delta;
}

//! Reset the background task
void Task::clear_background_task()
{
    background_task_ = nullptr;
}

//! If there is an operating running, execute its next step
void Task::execute_background_task()
{
    if(!background_task_ || !ELAPSED(millis(), next_op_time_))
        return;

    next_op_time_ = millis() + op_time_delta_;
    background_task_();
}

//! Check if there is a background task to execute
//! @return true if there is a background task to execute
bool Task::has_background_task() const
{
    return bool(background_task_);
}

}
