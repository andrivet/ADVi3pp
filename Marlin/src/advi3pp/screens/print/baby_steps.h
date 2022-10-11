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

#pragma once

#include "../core/screen.h"

namespace ADVi3pp {

//! Print Settings Page
struct BabySteps: Screen<BabySteps> {
  void baby_minus_command();
  void baby_plus_command();

  enum class Multiplier: uint8_t { M1, M2, M3 };

private:
  bool do_dispatch(KeyValue value);
  Page do_prepare_page();
  void do_back_command();
  void send_multiplier() const;
  void send_z_offset();
  int get_multiplier_value() const;

private:
  Multiplier multiplier_ = Multiplier::M1;
  bool save_ = false;

  friend Parent;
};

extern BabySteps baby_steps;

}
