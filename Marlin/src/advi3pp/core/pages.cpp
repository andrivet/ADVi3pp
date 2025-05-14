/**
 * ADVi3++ Firmware For Wanhao Duplicator i3 Plus (based on Marlin)
 *
 * Copyright (C) 2017-2025 Sebastien Andrivet [https://github.com/andrivet/]
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

#include "../../inc/MarlinConfig.h"
#include "../../lcd/extui/ui_api.h"
#include "dgus.h"
#include "core.h"
#include "stack.h"
#include "task.h"
#include "status.h"
#include "progress.h"
#include "../screens/common/wait.h"
#include "pages.h"

namespace ADVi3pp::Pages {

  inline namespace internals {
    constexpr size_t STACK_SIZE = 8; // Does not include Main

    Stack<Page, STACK_SIZE> back_{};
    Page forward_ = Page::None;
    Page current_ = Page::Main;

    void send_page_to_lcd(Page page);
    void reset_forward();
  }

  //! Show the given page on the LCD screen
  //! @param [in] page The page to be displayed on the LCD screen
  void show(Page page) {
    Log::info() << F("Pages::show") << page << Log::endl();

    auto current = get_current_page();
    Log::info() << F("Current page:") << current << Log::endl();
    // Do nothing if it is already the current page
    if(current != Page::Main && page == current) return;
    // Don't push Main or None (Main is implicitly always at the top)
    if(current != Page::Main && current != Page::None)
      back_.push(current);

    Log::info() << F("back pages:") << back_ << Log::endl();
    send_page_to_lcd(page);
  }

  //! Set page to display after the completion of an operation.
  void save_forward_page() {
    forward_ = get_current_page();
    Log::info() << F("Pages::save_forward_page") << forward_ << Log::endl();
  }

  //! Show the "Back" page on the LCD display.
  void show_back_page() {
    Log::info() << F("Pages::show_back_page") << Log::endl();

    if(back_.is_empty()) {
      Log::info() << F("Empty pages stack, show Main") << Log::endl();
      send_page_to_lcd(Page::Main);
      return;
    }

    auto page = back_.pop();
    Log::info() << F("Current Page:") << page << F("back pages:") << back_ << Log::endl();
    send_page_to_lcd(page);
  }

  //! Show the "Next" page on the LCD display.
  void show_forward_page() {
    Log::info() << F("Pages::show_forward_page") << Log::endl();
    // If no forward page defined, use the back page
    if(forward_ == Page::None) {
      show_back_page();
      return;
    }

    while(!back_.is_empty()) {
      auto back = back_.pop();
      if(back == forward_) {
        send_page_to_lcd(forward_);
        reset_forward();
        return;
      }
    }

    Log::error() << F("Forward page not found") << forward_ << Log::endl();
    reset_forward();
    send_page_to_lcd(Page::Main);
  }

  void save(SAVE_OPTIONS save, BACK_OPTIONS options) {
    Log::info() << F("Pages::save") << static_cast<uint16_t>(save) << static_cast<uint16_t>(options) << Log::endl();
    background_task.clear();

    if(test_one_bit(save, SAVE_OPTIONS::SETTINGS))  {
      ExtUI::saveSettings();
      if(test_one_bit(save, SAVE_OPTIONS::MESSAGE)) Status::set(GET_TEXT_F(MSG_SETTINGS_STORED), Status::STATUS_OPTIONS::RESET);
    }
    if(test_one_bit(options, BACK_OPTIONS::FINISH_MOVE) && Core::is_busy()) {
      Wait::wait([]() -> CALLBACK_RESULT {
        if (Core::is_busy()) return CALLBACK_RESULT::CONTINUE;
        clear_temporaries(false);
        Status::set_default();
        show_forward_page();
        return CALLBACK_RESULT::STOP;
      });
    }
    else
      show_forward_page();
  }

  void back(BACK_OPTIONS options) {
    Log::info() << F("Pages::back") << static_cast<uint16_t>(options) << Log::endl();
    Log::info() << F("back pages:") << back_ << Log::endl();
    background_task.clear();

    if(test_one_bit(options, BACK_OPTIONS::FINISH_MOVE) && Core::is_busy())
      Wait::wait([] () -> CALLBACK_RESULT {
        if(Core::is_busy()) return CALLBACK_RESULT::CONTINUE;
        clear_temporaries(false);
        Status::set_default();
        show_back_page();
        return CALLBACK_RESULT::STOP;
      });
    else
      show_back_page();
  }

  void clear_temporaries(bool show) {
    Log::info() << F("Pages::clear_temporaries") << show << Log::endl();

    auto current = get_current_page();
    if(!is_temporary(current)) return;
    while(is_temporary(current) && !back_.is_empty()) current = back_.pop();
    if(is_temporary(current)) current = Page::Main;

    Log::info() << F("current page") << current << F("back pages:") << back_ << Log::endl();

    if(show)
      send_page_to_lcd(current);
    else
      current_ = current;
  }

  void back_all(BACK_ALL_OPTIONS options) {
    Log::info() << F("Pages::back_all") << static_cast<uint16_t>(options) << Log::endl();
    if(test_one_bit(options, BACK_ALL_OPTIONS::SEND_BACK))
      while(!back_.is_empty()) {
        Log::info() << F("  send back to page: ") << get_current_page() << Log::endl();
        Core::process(get_current_page(), KEY_CODE_BACK, 0);
      }
    else
      back_.empty();
    if(test_one_bit(options, BACK_ALL_OPTIONS::SHOW_MAIN)) send_page_to_lcd(Page::Main);
  }

  void clear_current() {
    Log::info() << F("Pages::clear_current") << Log::endl();
    current_ = Page::None;
  }

  //! Retrieve the current page on the LCD screen
  Page get_current_page() {
    return current_;
  }

  inline namespace internals {

    void reset_forward() {
      forward_ = Page::None;
    }

    void send_page_to_lcd(Page page) {
      Log::info() << F("Pages::send_page_to_lcd") << page << Log::endl();
      WriteRegisterRequest{Register::PictureID}.write_page(page);
      current_ = page;
    }

  }
}
