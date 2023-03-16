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

#include "../parameters.h"
#include "../core/core.h"
#include "../core/status.h"
#include "wait.h"

namespace ADVi3pp {

Wait wait;

bool Wait::reset_and_call(WaitCallback &callback) {
  if(!callback) Log::error() << F("Null callback") << Log::endl();
  // Make a copy because calling callback may set callback. So reset callback before calling it through the copy.
  WaitCallback copy{callback};
  callback = nullptr;
  Log::log() << F("Call callback") << Log::endl();
  return copy();
}

//! Show a simple wait page with a message
void Wait::wait_() {
  Log::log() << F("wait_: Reset callbacks") << Log::endl();
  back_ = nullptr;
  continue_ = nullptr;
  Log::log() << F("Show wait page") << Log::endl();
  pages.show(Page::Waiting, ACTION);
}

//! Show a simple wait page with a message
void Wait::wait() {
  status.set(F("Please wait..."));
  wait_();
}

//! Show a simple wait page with a message
//! @param message  The message to display
void Wait::wait(const FlashChar* message) {
  status.set(message);
  wait_();
}

//! Show a simple wait page with a message
//! @param message  The message to display
void Wait::wait(const char* message) {
  status.set(message);
  wait_();
}

//! Show a simple wait page with a message
//! @param message  The message to display
//! @param back     Callback to be called when the back button is pressed
void Wait::wait_back(const FlashChar* message, const WaitCallback& back) {
  status.set(message);
  back_ = back;
  Log::log() << F("wait_back: Reset callback") << Log::endl();
  continue_ = nullptr;
  Log::log() << F("Show wait page") << Log::endl();
  pages.show(Page::WaitBack, ACTION);
}

//! Ensure a print is not running and if so, display a message
void Wait::wait_back(const FlashChar* message) {
  status.set(message);
  back_ = WaitCallback{this, &Wait::back};
  Log::log() << F("wait_back: Reset callback") << Log::endl();
  continue_ = nullptr;
  Log::log() << F("Show wait page") << Log::endl();
  pages.show(Page::WaitBack, ACTION);
}

//! Show a simple wait page with a message
//! @param message  The message to display
//! @param back     Callback to be called when the back button is pressed
//! @param cont     Callback to be called when the continue button is pressed
void Wait::wait_back_continue(const FlashChar* message, const WaitCallback& back, const WaitCallback& cont) {
  status.set(message);
  back_ = back;
  continue_ = cont;
  Log::log() << F("Show wait page") << Log::endl();
  pages.show(Page::WaitBackContinue, ACTION);
}

void Wait::wait_user(const char* message, bool awaiting) {
  //back_ = nullptr;

  status.set(message);
  if(awaiting) {
    // Ask for an action from the user
    Log::log() << F("Show wait-continue page") << Log::endl();
    continue_ = WaitCallback{this, &Wait::cont};
    pages.show(Page::WaitContinue, ACTION);
  }
  else {
    // Display a wait screen without asking for an action from the user
    Log::log() << F("wait_user: Reset callback") << Log::endl();
    continue_ = nullptr;
    Log::log() << F("Show wait page") << Log::endl();
    pages.show(Page::Waiting, ACTION);
  }
}

void Wait::homing(const WaitCallback& homed, const FlashChar* command) {
  Log::log() << F("homing: Set callback") << Log::endl();
  continue_ = homed;
  status.set(F("Homing..."));
  pages.show(Page::Waiting, ACTION);
  core.inject_commands(command == nullptr ? F("G28 Z O") : command);
  background_task.set(Callback{this, &Wait::home_task}, 200);
}

void Wait::home_task() {
  if (core.is_busy() || !ExtUI::isMachineHomed())
    return;
  Log::log() << F("Homed") << Log::endl();
  background_task.clear();
  pages.clear_temporaries();
  reset_and_call(continue_);
}

//! Default action when the continue button is pressed (inform Marlin)
bool Wait::cont() {
  ExtUI::setUserConfirmed();
  wait();
  return false;
}

//! Action when the back button is pressed
bool Wait::back() {
  pages.show_back_page();
  return false;
}

//! Handles the Back command
void Wait::on_back_command() {
  bool continue_processing = true;

  if(!back_)
    Log::error() << F("No Back action defined") << Log::endl();
  else {
    continue_processing = reset_and_call(back_);
  }

  if(continue_processing)
      Parent::on_back_command();
}

//! Handles the Save (Continue) command
void Wait::on_save_command() {
  bool continue_processing = true;

  if(!continue_)
    Log::error() << F("No Continue action defined") << Log::endl();
  else
    continue_processing = reset_and_call(continue_);

  if(continue_processing)
    pages.show_forward_page();
}

}

