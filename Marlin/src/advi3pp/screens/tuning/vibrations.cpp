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

#include "../../parameters.h"
#include "../../core/core.h"
#include "../../core/dgus.h"
#include "../../core/wait.h"
#include "vibrations.h"

namespace ADVi3pp {

Vibrations vibrations;

const int XY_SLOW = 4000;
const int XY_MEDIUM = 6000;
const int XY_FAST = 8000;
const int Z_SLOW = 1000;
const int Z_MEDIUM = 1200;
const int Z_FAST = 1400;

//! Execute command
//! @param key_value    The sub-action to handle
//! @return             True if the action was handled
bool Vibrations::on_dispatch(KeyValue key_value) {
  if(Parent::on_dispatch(key_value))
      return true;

  switch(key_value) {
    case KeyValue::VibrationsX:     x_command(); break;
    case KeyValue::VibrationsY:     y_command(); break;
    case KeyValue::VibrationsXY:    xy_command(); break;
    case KeyValue::VibrationsYX:    yx_command(); break;
    case KeyValue::VibrationsZ:     z_command(); break;
    default: return false;
  }

  return true;
}

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
void Vibrations::on_enter() {
  set_values();
  wait.wait();
  core.inject_commands(F("G28 O"));
}

//! Execute the Back command
void Vibrations::on_back_command() {
  wait.wait(F("Please wait for the move to finish..."));
  background_task.set(Callback{this, &Vibrations::move_finished});
}

void Vibrations::move_finished() {
  if(core.is_busy()) return;

  if(ExtUI::getAxisPosition_mm(ExtUI::Z) != 10) {
    core.inject_commands(F("G1 Z10 F1200"));
    background_task.set(Callback{this, &Vibrations::move_finished2});
    return;
  }

  background_task.clear();
  status.reset();
  pages.show_back_page(2);
}

void Vibrations::move_finished2() {
  if(core.is_busy()) return;

  background_task.clear();
  status.reset();
  pages.show_back_page(2);
}

void Vibrations::move_x() {
  if(core.is_busy()) return;

  int min, max;
  if(!get_values(min, max)) return;

  if(min < X_MIN_BED) min = X_MIN_BED;
  if(max > X_MAX_BED) max = X_MAX_BED;

  int new_position = (ExtUI::getAxisPosition_mm(ExtUI::X) == max) ? min : max;

  ADVString<20> cmd;
  cmd.format(F("G1 X%i F%i"), new_position, get_xy_speed());
  core.inject_commands(cmd.get());
}

void Vibrations::move_y() {
  if(core.is_busy()) return;

  int min, max;
  if(!get_values(min, max)) return;

  if(min < Y_MIN_BED) min = Y_MIN_BED;
  if(max > Y_MAX_BED) max = Y_MAX_BED;

  int new_position = (ExtUI::getAxisPosition_mm(ExtUI::Y) == max) ? min : max;

  ADVString<20> cmd;
  cmd.format(F("G1 Y%i F%i"), new_position, get_xy_speed());
  core.inject_commands(cmd.get());
}

void Vibrations::move_start_z() {
  ADVString<40> cmd;
  cmd.format(F("G1 X%i F6000\nG1 Y%i F6000"), X_CENTER, Y_CENTER);
  core.inject_commands(cmd.get());
  background_task.set(Callback{this, &Vibrations::move_z});
}

void Vibrations::move_z() {
  if(core.is_busy()) return;

  int min, max;
  if(!get_values(min, max)) return;

  if(min < 5) min = 5;
  if(max > 150) max = 150; // Don't go to the limit, could be dangerous with some mods

  int new_position = (ExtUI::getAxisPosition_mm(ExtUI::Z) == max) ? min : max;

  ADVString<20> cmd;
  cmd.format(F("G1 Z%i F%i"), new_position, get_z_speed());
  core.inject_commands(cmd.get());
}

void Vibrations::move_start_xy() {
  ADVString<40> cmd;
  cmd.format(F("G1 X%i F6000\nG1 Y%i F6000"), X_MIN_BED, Y_MIN_BED);
  core.inject_commands(cmd.get());
  background_task.set(Callback{this, &Vibrations::move_xy});
}

void Vibrations::move_start_yx() {
  ADVString<40> cmd;
  cmd.format(F("G1 X%i F6000\nG1 Y%i F6000"), X_MIN_BED, Y_MAX_BED);
  core.inject_commands(cmd.get());
  background_task.set(Callback{this, &Vibrations::move_xy});
}

void Vibrations::move_xy() {
  if(core.is_busy()) return;

  int min, max;
  if(!get_values(min, max)) return;

  int min_x = (min < X_MIN_BED) ? X_MIN_BED : min;
  int max_x = (max > X_MAX_BED) ? X_MAX_BED : max;
  int min_y = (min < Y_MIN_BED) ? Y_MIN_BED : min;
  int max_y = (max > Y_MAX_BED) ? Y_MAX_BED : max;

  int new_x_position = (ExtUI::getAxisPosition_mm(ExtUI::X) == max_x) ? min_x : max_x;
  int new_y_position = (ExtUI::getAxisPosition_mm(ExtUI::Y) == max_y) ? min_y : max_y;

  ADVString<40> cmd;
  cmd.format(F("G1 X%i Y%i F%i"), new_x_position, new_y_position, get_xy_speed());
  core.inject_commands(cmd.get());
}

bool Vibrations::get_values(int &min, int &max) {
  ReadRam frame{Variable::Value0};
  if(!frame.send_receive(3)) {
    Log::error() << F("Receiving Frame (Min Max)") << Log::endl();
    return false;
  }

  min = frame.read_word();
  max = frame.read_word();
  speed_ = static_cast<Speed>(frame.read_word());
  if(min < 0) min = 0;
  if(min >= max) return false;

  return true;
}

void Vibrations::set_values() {
  WriteRamRequest{Variable::Value0}.write_words(X_MIN_BED, X_MAX_BED, 1);
}

int Vibrations::get_xy_speed() {
  switch(speed_) {
    case Speed::Slow: return XY_SLOW;
    case Speed::Medium: return XY_MEDIUM;
    case Speed::Fast: return XY_FAST;
    default: return XY_MEDIUM;
  }
}

int Vibrations::get_z_speed() {
  switch(speed_) {
    case Speed::Slow: return Z_SLOW;
    case Speed::Medium: return Z_MEDIUM;
    case Speed::Fast: return Z_FAST;
    default: return Z_MEDIUM;
  }
}

void Vibrations::x_command() {
  background_task.set(Callback{this, &Vibrations::move_x});
}

void Vibrations::y_command() {
  background_task.set(Callback{this, &Vibrations::move_y});
}

void Vibrations::xy_command() {
  background_task.set(Callback{this, &Vibrations::move_start_xy});
}

void Vibrations::yx_command() {
  background_task.set(Callback{this, &Vibrations::move_start_yx});
}

void Vibrations::z_command() {
  background_task.set(Callback{this, &Vibrations::move_start_z});
}

}
