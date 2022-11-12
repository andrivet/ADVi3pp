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

using Callback = adv::Callback<void(*)()>;

//! Task
struct Task {
  enum class Activation { MULIPLE, ONE_TIME };
  static const unsigned DEFAULT_DELAY = 100; // ms

  Task() = default;
  explicit Task(const Callback& callback, unsigned int delay = DEFAULT_DELAY, Activation activation = Activation::MULIPLE);

  void set(const Callback& callback, unsigned int delay = DEFAULT_DELAY, Activation activation = Activation::MULIPLE);
  void clear();
  bool execute(bool force_execute = false);

private:
  void set_next_execute_time();

private:
  unsigned int delay_ = DEFAULT_DELAY;
  Activation activation_ = Activation::MULIPLE;
  millis_t next_execute_time_ = 0;
  Callback callback_;
};

extern Task background_task;

}
