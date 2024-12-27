/**
 * ADVi3++ Firmware For Wanhao Duplicator i3 Plus (based on Marlin 2)
 *
 * Copyright (C) 2017-2025 Sebastien Andrivet [https://github.com/andrivet/]
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version
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
#include "../core/logging.h"
#include "../core/dgus.h"
#include "../core/progress.h"
#include "status.h"

namespace ADVi3pp::Status {

  inline namespace internals {

    constexpr size_t MESSAGE_LENGTH = 32; //!< Size of messages to be displayed on the LCD Panel

    millis_t status_message_expire_ms = 0;

    void send_status(FSTR_P message);
    void send_status(const char* message);
  }

  void set(FSTR_P message, STATUS_OPTIONS options) {
    //Log::info() << F("Status::set#1") << message << static_cast<uint16_t>(options) << Log::endl();
    if(test_one_bit(options, STATUS_OPTIONS::RESET)) reset();
    bool persistent = test_one_bit(options, STATUS_OPTIONS::PERSISTENT);
    // If a persistent message is already displayed, do not override it my a non-persistent one
    if(status_message_expire_ms == 0 && !persistent) return;
    status_message_expire_ms = persistent ? 0 : millis() + (STATUS_MESSAGE_TIMEOUT_SEC) * 1000UL;
    send_status(message);
  }

  void set(const char* message, STATUS_OPTIONS options) {
    //Log::info() << F("Status::set#2") << message << static_cast<uint16_t>(options) << Log::endl();
    if(test_one_bit(options, STATUS_OPTIONS::RESET)) reset();
    bool persistent = test_one_bit(options, STATUS_OPTIONS::PERSISTENT);
    // If a persistent message is already displayed, do not override it my a non-persistent one
    if(status_message_expire_ms == 0 && !persistent) return;
    status_message_expire_ms = persistent ? 0 : millis() + (STATUS_MESSAGE_TIMEOUT_SEC) * 1000UL;
    send_status(message);
  }

  void set_default() {
    set(GET_TEXT_F(WELCOME_MSG), Status::STATUS_OPTIONS::RESET);
  }

  void reset() {
    //Log::info() << F("Status::reset") << Log::endl();
    status_message_expire_ms = millis() + (STATUS_MESSAGE_TIMEOUT_SEC) * 1000UL;
  }

  void init() {
    status_task.set([] () -> CALLBACK_RESULT {
      if(status_message_expire_ms && ELAPSED(millis(), status_message_expire_ms))
        LCD_RESET_STATUS();
      return CALLBACK_RESULT::CONTINUE;
    });
  }

  inline namespace internals {

    void send_status(FSTR_P message) {
      WriteRamRequest{Variable::Message}.write_text(message, MESSAGE_LENGTH);
      WriteRamRequest{Variable::CenteredMessage}.write_centered_text(message, MESSAGE_LENGTH);
    }

    void send_status(const char* message) {
      WriteRamRequest{Variable::Message}.write_text(message, MESSAGE_LENGTH);
      WriteRamRequest{Variable::CenteredMessage}.write_centered_text(message, MESSAGE_LENGTH);
    }

  }
}
