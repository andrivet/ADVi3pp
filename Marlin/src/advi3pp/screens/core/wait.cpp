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
#include "../../core/status.h"
#include "wait.h"

namespace ADVi3pp {

Wait wait;


Page Wait::do_prepare_page()
{
    return Page::Waiting;
}

//! Show a simple wait page with a message
void Wait::wait_()
{
    back_ = nullptr;
    continue_ = nullptr;
    pages.show(Page::Waiting);
}

//! Show a simple wait page with a message
void Wait::wait()
{
    status.set(F("Please wait..."));
    wait_();
}

//! Show a simple wait page with a message
//! @param message  The message to display
void Wait::wait(const FlashChar* message)
{
    status.set(message);
    wait_();
}

//! Show a simple wait page with a message
//! @param message  The message to display
void Wait::wait(const char* message)
{
  status.set(message);
  wait_();
}

//! Show a simple wait page with a message
//! @param message  The message to display
//! @param back     Callback to be called when the back button is pressed
void Wait::wait_back(const FlashChar* message, const WaitCallback& back)
{
    status.set(message);
    back_ = back;
    continue_ = nullptr;
    pages.show(Page::WaitBack);
}

//! Ensure a print is not running and if so, display a message
void Wait::wait_back(const FlashChar* message)
{
    status.set(message);
    back_ = WaitCallback{this, &Wait::on_back};
    continue_ = nullptr;
    pages.show(Page::WaitBack);
}

//! Show a simple wait page with a message
//! @param message  The message to display
//! @param back     Callback to be called when the back button is pressed
//! @param cont     Callback to be called when the continue button is pressed
void Wait::wait_back_continue(const FlashChar* message, const WaitCallback& back, const WaitCallback& cont)
{
    status.set(message);
    back_ = back;
    continue_ = cont;
    pages.show(Page::WaitBackContinue);
}

void Wait::wait_user(const char* message) {
  back_ = nullptr;

  status.set(message);
  if(ExtUI::awaitingUserConfirm()) {
    continue_ = WaitCallback{this, &Wait::on_continue};
    pages.show(Page::WaitContinue);
  }
  else {
    continue_ = nullptr;
    pages.show(Page::Waiting);
  }
}

//! Default action when the continue button is pressed (inform Marlin)
bool Wait::on_continue()
{
    ExtUI::setUserConfirmed();
    wait();
    return false;
}

//! Action when the back button is pressed
bool Wait::on_back()
{
    pages.show_back_page();
    return false;
}

//! Handles the Back command
void Wait::do_back_command()
{
    bool continue_processing = true;

    if(!back_)
        Log::error() << F("No Back action defined") << Log::endl();
    else
    {
        continue_processing = back_();
        back_ = nullptr;
    }

    if(continue_processing)
        Parent::do_back_command();
}

//! Handles the Save (Continue) command
void Wait::do_save_command()
{
    bool continue_processing = true;

    if(!continue_)
        Log::error() << F("No Continue action defined") << Log::endl();
    else
    {
        continue_processing = continue_();
        continue_ = nullptr;
    }

    if(continue_processing)
        pages.show_forward_page();
}

}

