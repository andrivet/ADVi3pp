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

#include "../core/task.h"
#include "../core/string.h"
#include "../core/core.h"
#include "../core/status.h"
#include "../core/dgus.h"
#include "../core/screen.h"

namespace ADVi3pp {

using WaitCallback = adv::Callback<bool(*)()>;

//! Display wait page and messages
struct Wait: Screen<Wait> {
  static constexpr Page PAGE = Page::Waiting;
  static constexpr Action ACTION = Action::Wait;

  void wait();
  void wait(const FlashChar* message);
  void wait(const char* message);
  void wait_back(const FlashChar* message, const WaitCallback& back);
  void wait_back(const FlashChar* message);
  void wait_back_continue(const FlashChar* message, const WaitCallback& back, const WaitCallback& cont);
  void wait_user(const char* message);

  void home_and_wait(const Callback &f, const FlashChar* cmd = nullptr, const FlashChar* msg = nullptr);
  bool check_homed();

private:
  void on_save_command();
  void on_back_command();

  bool cont();
  bool back();
  void wait_();

private:
  WaitCallback back_;
  WaitCallback continue_;

  friend Parent;
};

extern Wait wait;

// --------------------------------------------------------------------

}

