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

#include "../../../inc/MarlinConfig.h"

#ifdef BLTOUCH

#include "../../../feature/bltouch.h"
#include "../../../lcd/marlinui.h"
#include "../../../gcode/gcode.h"
#include "../../lib/ADVstd/bitmasks.h"
#include "../../core/status.h"
#include "../../core/core.h"
#include "../../core/dgus.h"
#include "../../core/pool.h"
#include "../common/wait.h"
#include "bltouch_testing.h"

namespace ADVi3pp {
  enum class Wires: uint8_t {
    None    = 0b00000000,
    Brown   = 0b00000001,
    Red     = 0b00000010,
    Orange  = 0b00000100,
    Black   = 0b00001000,
    White   = 0b00010000
  };
}
ENABLE_BITMASK_OPERATOR(ADVi3pp::Wires);

namespace ADVi3pp::BlTouchTesting {

  inline namespace internals {
    constexpr uint16_t KEY_CODE_STEP1_YES = 1;
    constexpr uint16_t KEY_CODE_STEP1_NO = 2;
    constexpr uint16_t KEY_CODE_STEP2_SLOW = 3;
    constexpr uint16_t KEY_CODE_STEP2_QUICK = 4;
    constexpr uint16_t KEY_CODE_STEP2_NO = 5;
    constexpr uint16_t KEY_CODE_STEP3_YES = 6;
    constexpr uint16_t KEY_CODE_STEP3_NO = 7;

    struct Data {
      Wires tested_ = Wires::None;
      Wires ok_ = Wires::None;
    };

    inline Data& pool() { return Pool::get<Data>(Page::BlTouchTesting1); }

    void show_command();
    void back_command();
    void save_command();

    void step_1_yes();
    void step_1_no();
    void step_2();
    void step_2_slow();
    void step_2_quick();
    void step_2_no();
    void step_3();
    void step_3_yes();
    void step_3_no();
    void step_4();
    void step_5();
    uint16_t wire_value(Wires wire);
  }

  bool handle_command(uint16_t key_code) {
    switch(key_code) {
      case KEY_CODE_SHOW: show_command(); break;
      case KEY_CODE_BACK: back_command(); break;
      case KEY_CODE_SAVE: save_command(); break;
      case KEY_CODE_STEP1_YES: step_1_yes(); break;
      case KEY_CODE_STEP1_NO: step_1_no(); break;
      case KEY_CODE_STEP2_SLOW: step_2_slow(); break;
      case KEY_CODE_STEP2_QUICK: step_2_quick(); break;
      case KEY_CODE_STEP2_NO: step_2_no(); break;
      case KEY_CODE_STEP3_YES: step_3_yes(); break;
      case KEY_CODE_STEP3_NO: step_3_no(); break;
      default: return false;
    }
    return true;
  }

  inline namespace internals {

    const Status::STATUS_OPTIONS OPTIONS = Status::STATUS_OPTIONS::RESET | Status::STATUS_OPTIONS::PERSISTENT;
    inline void status_red_brown()   { Status::set(GET_TEXT_F(ADVI3PP_MSG_BLTOUCH_RED_BROWN), OPTIONS); }
    inline void status_orange()      { Status::set(GET_TEXT_F(ADVI3PP_MSG_BLTOUCH_ORANGE), OPTIONS); }
    inline void status_white_black() { Status::set(GET_TEXT_F(ADVI3PP_MSG_BLTOUCH_WHITE_BLACK), OPTIONS); }
    inline void status_pin()         { Status::set(GET_TEXT_F(ADVI3PP_MSG_BLTOUCH_PIN), OPTIONS); }

    inline uint16_t wire_value(Wires wire) {
      return test_one_bit(pool().ok_, wire) ? 1 : test_one_bit(pool().tested_, wire) ? 2 : 0;
    }

    void show_command() {
      if(!Core::check_not_busy()) return;
      Pool::reset<Data>(Page::BlTouchTesting1);
      Status::set(GET_TEXT_F(ADVI3PP_MSG_BLTOUCH_TESTING), OPTIONS);
      Pages::save_forward_page();
      pool().tested_ = pool().ok_ = Wires::None;
      set_bits(pool().tested_, Wires::Brown | Wires::Red);
      Pages::show(Page::BlTouchTesting1);
      bltouch._reset();
    }

    void back_command() {
      bltouch._reset();
      Pages::back(Pages::BACK_OPTIONS::FINISH_MOVE);
    }

    void save_command() {
      Pages::save(Pages::SAVE_OPTIONS::NOTHING, Pages::BACK_OPTIONS::FINISH_MOVE);
    }

    void step_1_yes() {
      step_2();
    }

    void step_1_no() {
      status_red_brown();
      step_5();
    }

    //! Is the BLTouch blinking?
    void step_2() {
      Pages::show(Page::BlTouchTesting2);
    }

    void step_2_no() {
      set_bits(pool().ok_, Wires::Brown | Wires::Red);
      step_3();
    }

    void step_2_slow() {
      status_red_brown();
      step_5();
    }

    void step_2_quick() {
      status_pin();
      step_5();
    }

    //! BLTouch deploy and stow (self test)
    void step_3() {
      set_bits(pool().tested_, Wires::Orange);
      Pages::show(Page::BlTouchTesting3);
      bltouch._selftest();
    }

    void step_3_yes() {
      set_bits(pool().ok_, Wires::Orange);
      step_4();
    }

    void step_3_no() {
      status_orange();
      step_5();
    }

    void step_4() {
      set_bits(pool().tested_, Wires::White | Wires::Black);

      Wait::wait(GET_TEXT_F(ADVI3PP_MSG_BLTOUCH_TRIGGER));

      // Adapted from M43 code
      // This code will try to detect a BLTouch probe
      SET_INPUT_PULLUP(Z_MIN_PROBE_PIN);
      bltouch._reset();
      bltouch._stow();
      if(READ(Z_MIN_PROBE_PIN) == Z_MIN_ENDSTOP_HIT_STATE) { status_white_black(); step_5(); return; }

      bltouch._set_SW_mode();
      if(READ(Z_MIN_PROBE_PIN) != Z_MIN_ENDSTOP_HIT_STATE) { status_white_black(); step_5(); return; }

      bltouch._deploy();
      if(READ(Z_MIN_PROBE_PIN) == Z_MIN_ENDSTOP_HIT_STATE) { status_white_black(); step_5(); return; }

      bltouch._stow();

      // BLTouch Classic 1.2, 1.3, Smart 1.0, 2.0, 2.2, 3.0, 3.1 detected
      // Check for a 3.1 by letting the user trigger it, later

      bltouch._deploy();
      safe_delay(500);

      Pages::show(Page::BlTouchTesting4);

      // Wait 30 seconds for user to trigger probe
      for(uint16_t j = 0; j < 500 * 30; j++) {
        safe_delay(2);

        if(0 == j % (500 * 1)) gcode.reset_stepper_timeout();    // Keep steppers powered

        if(READ(Z_MIN_PROBE_PIN) == Z_MIN_ENDSTOP_HIT_STATE) {
          uint16_t probe_counter = 0; // Pulse width / 2
          for(probe_counter = 0; probe_counter < 15 && READ(Z_MIN_PROBE_PIN); ++probe_counter) safe_delay(2);
          Log::info() << F("BLTouch pulse width 0x") << probe_counter << Log::endl();

          if(probe_counter < 4) {
            Status::set(GET_TEXT_F(ADVI3PP_MSG_BLTOUCH_NOISE), OPTIONS);
            step_5();
            return;
          }

          if(probe_counter == 15)
            Status::set(GET_TEXT_F(ADVI3PP_MSG_BLTOUCH_3_1), Status::STATUS_OPTIONS::RESET);
          else
            Status::set(GET_TEXT_F(ADVI3PP_MSG_BLTOUCH_3_0), Status::STATUS_OPTIONS::RESET);

          bltouch._stow();
          set_bits(pool().ok_, Wires::White | Wires::Black);
          step_5();
          return;
        }
      }

      status_white_black();
      step_5();
    }

    void step_5() {
      bltouch._reset();

      auto brown = wire_value(Wires::Brown);
      auto red = wire_value(Wires::Red);
      auto orange = wire_value(Wires::Orange);
      auto black = wire_value(Wires::Black);
      auto white = wire_value(Wires::White);

      WriteRamRequest{Variable::Value0}.write_words(
          brown,
          red,
          orange,
          black,
          white
      );

      Pages::show(Page::BlTouchTesting5);
    }

  }
}

#endif
