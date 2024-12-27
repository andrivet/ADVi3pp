/**
 * ADVi3++ Firmware For Wanhao Duplicator i3 Plus (based on Marlin)
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

#include "../../core/task.h"
#include "../../core/string.h"
#include "../../core/core.h"
#include "../../core/dgus.h"
#include "../../core/pages.h"

namespace ADVi3pp::Wait {
  bool handle_command(uint16_t key_code);

  void wait();
  void wait(FSTR_P message);
  void wait(CALLBACK_RESULT (*cb)());
  void wait(FSTR_P message, CALLBACK_RESULT (*cb)());
  void wait_back(FSTR_P title, FSTR_P message, void (*cb)());
  void wait_back(FSTR_P title, FSTR_P message);
  void wait_back_continue(FSTR_P title, FSTR_P message, void (*cb)(CALLBACK_SOURCE));
  void wait_user(FSTR_P title, const char *message, bool awaiting);
  void wait_user(FSTR_P title, FSTR_P message, bool awaiting);
  void homing(void (*cb)(), const FlashChar* command = nullptr);
  void homing(const FlashChar* command = nullptr);
  void homing_start();
  void homing_done();
  void ensure_homed(void (*cb)());
  void not_busy();
  void set_message(const char* msg);
  void set_message(FSTR_P msg);
}
