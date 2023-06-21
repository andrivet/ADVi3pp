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
#include "print.h"
#include "../../core/core.h"
#include "../../core/status.h"
#include "../../core/wait.h"


namespace ADVi3pp {

Print print;

//! Handle print commands.
//! @param key_value    The sub-action to handle
//! @return             True if the action was handled
bool Print::on_dispatch(KeyValue value) {
  if(Parent::on_dispatch(value))
    return true;

  switch(value) {
    case KeyValue::PrintStop:           stop_command(); break;
    case KeyValue::PrintPause:          pause_resume_command(); break;
    case KeyValue::PrintAdvancedPause:  advanced_pause_command(); break;
    default:                            return false;
  }

  return true;
}

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
bool Print::on_enter() {
#ifdef ADVi3PP_PROBE
  if(!ExtUI::getLevelingActive())
    status.set(F("WARNING: Bed leveling not active."));
#endif
  return true;
}

//! Stop printing
void Print::stop_command() {
  if(!ExtUI::isPrinting()) return;

  wait.wait_back_continue(F("Abort printing?"),
  WaitCallback{this, &Print::cancel_abort_print}, WaitCallback{this, &Print::abort_print});
}

bool Print::cancel_abort_print() {
  status.set(F("Continue printing"));
  return true;
}

bool Print::abort_print() {
  pages.clear_temporaries();
  ExtUI::stopPrint();
  return false;
}

//! Pause printing
void Print::pause_resume_command() {
  if(!ExtUI::isPrinting())
      return;

  if(ExtUI::isPrintingPaused()) {
    wait.wait(F("Resume printing..."));
    ExtUI::resumePrint();
  }
  else{
    wait.wait(F("Pause printing..."));
    ExtUI::pausePrint();
  }
}

//! Advanced Pause for filament change
void Print::advanced_pause_command() {
  if(!ExtUI::isPrinting())
    return;

  wait.wait(F("Pausing..."));
  core.inject_commands(F("M600"));
}

}
