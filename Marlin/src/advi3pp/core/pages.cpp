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

#include "../parameters.h"
#include "dgus.h"
#include "pages.h"
#include "core.h"
#include "settings.h"
#include "wait.h"

namespace ADVi3pp {

Pages pages;

inline Log& operator<<(Log& log, Page page) {
  log << static_cast<uint16_t>(page);
  return log;
}

inline Log& operator<<(Log& log, Action action) {
  log << static_cast<uint16_t>(action);
  return log;
}

//! Show the given page on the LCD screen
//! @param [in] page The page to be displayed on the LCD screen
void Pages::show(Page page, Action action) {
  Log::log() << F("show") << page << action << Log::endl();
  auto current = get_current_context();
  // Don't push temporary screens or Main (Main is implicitly always at the top)
  if(!is_temporary(current.page) && current.page != Page::Main)
    back_.push(current);

  send_page_to_lcd(Context{page, action});
}

void Pages::send_page_to_lcd(Context context) {
  Log::log() << F("Display page") << static_cast<double>(context.page & Page::PageNumber) << Log::endl();
  WriteRegisterRequest{Register::PictureID}.write_page(context.page & Page::PageNumber);
  current_ = context;
}

//! Retrieve the current page on the LCD screen
Pages::Context Pages::get_current_context() {
  // Boot page switches automatically (animation) to the Main page
	if(current_.page == Page::None || current_.page == Page::Boot)
    current_= Context{Page::Main, Action::Controls};
  return current_;
}

//! Set page to display after the completion of an operation.
void Pages::save_forward_page() {
  forward_ = get_current_context();
}

//! Show the "Back" page on the LCD display.
void Pages::show_back_page(unsigned nb_back) {
  Context context{Page::Main, Action::Controls};

  for(; nb_back > 0; --nb_back) {
    if (back_.is_empty())
      break;

    context = back_.pop();
    if (context.page == forward_.page)
      forward_ = Context{Page::None, Action::None};
  }

  send_page_to_lcd(context);
}

//! Show the "Next" page on the LCD display.
void Pages::show_forward_page() {
  // If no forward page defined, use the back page
  if(forward_.page == Page::None) {
    show_back_page();
    return;
  }

  while(!back_.is_empty()) {
    auto back = back_.pop();
    if(back.page == forward_.page) {
      send_page_to_lcd(forward_);
      forward_ = Context{Page::None, Action::None};
      return;
    }
  }

  Log::error() << F("Back pages do not contain page") << forward_.page << Log::endl();
  forward_ = Context{Page::None, Action::None};
  send_page_to_lcd(Context{Page::Main, Action::Controls});
}

void Pages::reset() {
  back_.empty();
}

void Pages::save() {
  settings.save();
  if(current_page_ensure_no_move() && core.is_busy()) {
    wait.wait();
    background_task.set(Callback{&Pages::save_task});
  }
  else
    pages.show_forward_page();
}

void Pages::save_task() {
  if(core.is_busy()) return;
  background_task.clear();
  status.reset();
  pages.show_forward_page();
}

void Pages::back() {
  if(current_page_ensure_no_move() && core.is_busy()) {
    wait.wait();
    background_task.set(Callback{&Pages::back_task});
  }
  else
    pages.show_back_page();
}

void Pages::back_task() {
  if(core.is_busy()) return;
  background_task.clear();
  status.reset();
  pages.clear_temporaries();
  pages.show_back_page();
}

void Pages::clear_temporaries() {
  auto current = get_current_context();
  if(!is_temporary(current.page))
    return;

  Log::log() << F("Clear temporaries") << Log::endl();
  while(is_temporary(current.page) && !back_.is_empty())
    current = back_.pop();
  send_page_to_lcd(current);
}

bool Pages::check_no_print(Page page) {
  if(!test_one_bit(page, Page::EnterNoPrint) || !ExtUI::isPrinting())
    return true;
  wait.wait_back(F("This is not accessible when printing"));
  return false;
}

void Pages::go_to_print() {
  auto current = pages.get_current_context();
  // If already on the print page, do nothing
  if(current.page == Page::Print)
    return;

  // Otherwise, pop the back pages and send abort messages
  while(!back_.is_empty() && current.page != Page::Print) {
    core.process_action(current.action, KeyValue::Abort);
    current = back_.pop();
  }
  // Display print page
  send_page_to_lcd(Context{Page::Print, Action::Print});
}

}
