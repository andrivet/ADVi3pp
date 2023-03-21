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
#include "pause_options.h"
#include "../../core/wait.h"

namespace ADVi3pp {

PauseOptions pause_options;

//! Handle Preheat actions.
//! @param key_value    Sub-action to handle
//! @return             True if the action was handled
bool PauseOptions::on_dispatch(KeyValue key_value) {
  if(Parent::on_dispatch(key_value))
    return true;

  switch(key_value) {
    case KeyValue::Extrude:     extrude_command(); break;
    case KeyValue::Resume:      resume_command(); break;
    default:                    return false;
  }

  return true;
}

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
bool PauseOptions::on_enter() {
  pause_menu_response = PAUSE_RESPONSE_WAIT_FOR;
  return true;
}

void PauseOptions::extrude_command() {
  pause_menu_response = PAUSE_RESPONSE_EXTRUDE_MORE;
}

void PauseOptions::resume_command() {
  wait.wait(F("Please wait..."));
  pause_menu_response = PAUSE_RESPONSE_RESUME_PRINT;
}

}
