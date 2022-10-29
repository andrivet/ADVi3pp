/**
 * ADVi3++ Firmware For Wanhao Duplicator i3 Plus (based on Marlin)
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

#pragma once

#include "../../core/task.h"
#include "../../core/screen.h"

namespace ADVi3pp {

//! Load and Unload Page
struct LoadUnload: Screen<LoadUnload> {
  static constexpr Page PAGE =  Page::LoadUnload;
  static constexpr Action ACTION = Action::LoadUnload;

private:
  bool on_dispatch(KeyValue key_value);
  void on_enter();
  void on_back_command();

  void prepare();
  void load_command();
  void unload_command();
  void send_data();

  friend Parent;

private:
  float previous_z_ = 0;
};

extern LoadUnload load_unload;

}
