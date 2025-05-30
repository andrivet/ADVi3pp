/**
 * ADVi3++ Firmware For Wanhao Duplicator i3 Plus (based on Marlin 2)
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

#include "src/inc/MarlinConfig.h"
#include "src/lcd/extui/ui_api.h"
#include "../../core/core.h"
#include "../../core/status.h"
#include "../../core/progress.h"
#include "wait.h"

namespace ADVi3pp::Wait {

  inline namespace internals {
    constexpr size_t TITLE_LENGTH = 18;

    DynamicCallback callback_{};

    void save_command();
    void back_command();

    void wait_();
  }

  bool handle_command(uint16_t key_code) {
    switch(key_code) {
      case KEY_CODE_BACK: back_command(); break;
      case KEY_CODE_SAVE: save_command(); break;
      default: return false;
    }
    return true;
  }

  void set_message(const char* msg) {
    Log::info() << F("Wait::set_message") << msg << Log::endl();
    Status::set(msg, Status::STATUS_OPTIONS::RESET | Status::STATUS_OPTIONS::PERSISTENT);
  }

  void set_message(FSTR_P msg) {
    Log::info() << F("Wait::set_message") << msg << Log::endl();
    Status::set(msg, Status::STATUS_OPTIONS::RESET | Status::STATUS_OPTIONS::PERSISTENT);
  }

  void set_title(FSTR_P title) {
    WriteRamRequest{Variable::LongTextCentered0}.write_centered_text(title, TITLE_LENGTH);
  }

  //! Show a simple wait page with a message
  void wait() {
    set_message(GET_TEXT_F(MSG_PLEASE_WAIT));
    wait_();
  }

  //! Show a simple wait page with a message
  void wait(CALLBACK_RESULT (*cb)()) {
    set_message(GET_TEXT_F(MSG_PLEASE_WAIT));
    wait_task.set(cb);
    wait_();
  }

  //! Show a simple wait page with a message
  //! @param message  The message to display
  void wait(FSTR_P message) {
    set_message(message);
    wait_();
  }

  //! Show a simple wait page with a message
  //! @param message  The message to display
  void wait(FSTR_P message, CALLBACK_RESULT (*cb)()) {
    set_message(message);
    wait_task.set(cb);
    wait_();
  }

  //! Show a simple wait page with a message
  //! @param message  The message to display
  //! @param back     Callback to be called when the back button is pressed
  void wait_back(FSTR_P title, FSTR_P message, void (*cb)()) {
    Log::info() << F("Wait::wait_back") << Log::endl();
    set_title(title);
    set_message(message);
    callback_ = cb;
    Core::send_lcd_zero();
    Pages::show(Page::WaitBack);
  }

  //! Ensure a print is not running and if so, display a message
  void wait_back(FSTR_P title, FSTR_P message) {
    Log::info() << F("Wait::wait_back") << Log::endl();
    set_title(title);
    set_message(message);
    callback_ = [] () -> CALLBACK_RESULT {
      Log::info() << F("Wait::callback") << Log::endl();
      Pages::back(Pages::BACK_OPTIONS::NONE);
      return CALLBACK_RESULT::STOP;
    };
    Core::send_lcd_zero();
    Pages::show(Page::WaitBack);
  }

  //! Show a simple wait page with a message
  //! @param message  The message to display
  //! @param back     Callback to be called when the back button is pressed
  //! @param cont     Callback to be called when the continue button is pressed
  void wait_back_continue(FSTR_P title, FSTR_P message, void (*cb)(CALLBACK_SOURCE)) {
    Log::info() << F("Wait::wait_back_continue") << Log::endl();
    set_title(title);
    set_message(message);
    callback_ = cb;
    Core::send_lcd_zero();
    Pages::show(Page::WaitBackContinue);
  }

  void wait_user(bool awaiting) {
    Log::info() << F("Wait::wait_user") << awaiting << Log::endl();
    if(awaiting) {
      // Ask for an action from the user
      callback_ = [] () -> void {
        Log::info() << F("Wait::callback") << Log::endl();
        ExtUI::setUserConfirmed(false);
        Status::reset();
      };
      Core::send_lcd_zero();
      Pages::show(Page::WaitContinue);
    }
    else {
      // Display a wait screen without asking for an action from the user
      callback_ = nullptr;
      Pages::show(Page::Wait);
    }
  }

  void wait_user(FSTR_P title, const char * const message, bool awaiting) {
    Log::info() << F("Wait::wait_user") << title << message << awaiting << Log::endl();
    Pages::clear_temporaries(false);
    set_title(title);
    set_message(message);
    wait_user(awaiting);
  }

  void wait_user(FSTR_P title, FSTR_P message, bool awaiting) {
    Log::info() << F("Wait::wait_user") << title << message << awaiting << Log::endl();
    Pages::clear_temporaries(false);
    set_title(title);
    set_message(message);
    wait_user(awaiting);
  }

  void homing(void (*cb)(), const FlashChar* command) {
    Log::info() << F("Wait::homing") << Log::endl();
    callback_ = cb;
    set_message(GET_TEXT_F(MSG_HOMING));
    Pages::show(Page::Wait);
    Core::inject_commands(command == nullptr ? F("G28") : command);
  }

  void homing(const FlashChar* command) {
    homing(nullptr, command);
  }

  void homing_start() {
    Log::info() << F("Wait::homing_start") << Log::endl();
    Status::set(GET_TEXT_F(MSG_HOMING), Status::STATUS_OPTIONS::RESET);
  }

  void homing_done() {
    Log::info() << F("Wait::homing_done") << Log::endl();
    Pages::clear_temporaries(!callback_);
    Status::set(GET_TEXT_F(ADVI3PP_MSG_HOMED)); // No reset to avoid erasing probing errors
    callback_();
    callback_ = nullptr;
  }

  void ensure_homed(void (*cb)()) {
    Log::info() << F("Wait::ensure_homed") << Log::endl();
    if(ExtUI::isAxisPositionKnown(ExtUI::X) && ExtUI::isAxisPositionKnown(ExtUI::Y) && ExtUI::isAxisPositionKnown(ExtUI::Z)) {
      Log::info() << F("  Position known, called directly the event") << Log::endl();
      cb();
      return;
    }

    Log::info() << F("  Position not known") << Log::endl();
    homing(cb);
  }

  void not_busy() {
    Log::info() << F("Wait::not_busy") << Log::endl();
    wait_task.set([] () -> CALLBACK_RESULT {
      if(Core::is_busy()) return CALLBACK_RESULT::CONTINUE;
      Pages::back_all(Pages::BACK_ALL_OPTIONS::SHOW_MAIN);
      return CALLBACK_RESULT::STOP;
    });
    wait();
  }

  inline namespace internals {

    //! Show a simple wait page with a message
    void wait_() {
      Log::info() << F("Wait::wait_") << Log::endl();
      callback_ = nullptr;
      Pages::show(Page::Wait);
    }

    //! Handles the Back command
    void back_command() {
      Log::info() << F("Wait::back_command") << Log::endl();
      Pages::clear_temporaries(true);
      callback_(CALLBACK_SOURCE::BACK);
    }

    //! Handles the Save (Continue) command
    void save_command() {
      Log::info() << F("Wait::save_command") << Log::endl();
      Pages::clear_temporaries(true);
      callback_(CALLBACK_SOURCE::SAVE);
    }

  }
}
