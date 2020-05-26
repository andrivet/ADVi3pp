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

namespace ADVi3pp {

using BackgroundTask = adv::Callback<void(*)()>;
using WaitCallback = adv::Callback<bool(*)()>;

//! Background Task
struct Task
{
    void set_background_task(const BackgroundTask& task, unsigned int delta = 500);
    void clear_background_task();
    void execute_background_task();
    bool has_background_task() const;
    bool is_update_time();

private:
    void set_next_update_time(unsigned int delta = 500);

private:
    unsigned int op_time_delta_ = 500;
    millis_t next_op_time_ = 0;
    millis_t next_update_time_ = 0;
    BackgroundTask background_task_;
};

extern Task task;

}
