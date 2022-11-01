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

#ifdef BLTOUCH

#include "../../parameters.h"
#include "../../core/core.h"
#include "../../core/dgus.h"
#include "../../core/status.h"
#include "../../core/wait.h"
#include "bltouch_testing.h"
#include "../../../feature/bltouch.h" // Use directly the BLTouch class

namespace ADVi3pp {

BLTouchTesting bltouch_testing;

inline bool triggered() { return Z_MIN_ENDSTOP_INVERTING != READ(Z_MIN_PIN); }

static void status_red_brown()   { status.set(F("Check red (VCC) and brown (GND) wires")); }
static void status_orange()      { status.set(F("Check the orange (Servo) wire")); }
static void status_white_black() { status.set(F("Check white (Z-stop) and black (GND) wires")); }
static void status_pin()         { status.set(F("Check the pin of the BLTouch")); }


//! Execute a command
//! @param key_value    The sub-action to handle
//! @return             True if the action was handled
bool BLTouchTesting::on_dispatch(KeyValue key_value) {
  if(Parent::on_dispatch(key_value))
    return true;

  switch(key_value) {
    case KeyValue::BLTouchTestingStep1aYes:   step_1a_yes(); break;
    case KeyValue::BLTouchTestingStep1aNo:    step_1a_no(); break;
    case KeyValue::BLTouchTestingStep1bSlow:  step_1b_slow(); break;
    case KeyValue::BLTouchTestingStep1bQuick: step_1b_quick(); break;
    case KeyValue::BLTouchTestingStep1bNo:    step_1b_no(); break;
    case KeyValue::BLTouchTestingStep2Yes:    step_2_yes(); break;
    case KeyValue::BLTouchTestingStep2No:     step_2_no(); break;
    default: return false;
  }

  return true;
}

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
void BLTouchTesting::on_enter() {
  pages.save_forward_page();
  step_1a();
}

void BLTouchTesting::on_back_command() {
  bltouch._reset();;
  Parent::on_back_command();
}

void BLTouchTesting::on_abort() {
  bltouch._reset();
}

void BLTouchTesting::on_save_command() {
  // Do not call parent, there nothing to save
  pages.show_forward_page();
}

//! Test if the BLTouch is powered
void BLTouchTesting::step_1a() {
  wait.wait(F("Initialize the BLTouch..."));
  tested_ = ok_ = Wires::None;
  set_bits(tested_, Wires::Brown | Wires::Red);
  bltouch._reset();
}

void BLTouchTesting::step_1a_yes() {
  step_1b();
}

void BLTouchTesting::step_1a_no() {
  status_red_brown();
  step_4();
}

//! Is the BLTouch blinking?
void BLTouchTesting::step_1b() {
  pages.show(Page::BLTouchTesting1B, ACTION);
}

void BLTouchTesting::step_1b_no() {
  set_bits(ok_, Wires::Brown | Wires::Red);
  step_2();
}

void BLTouchTesting::step_1b_slow() {
  set_bits(tested_, Wires::Brown | Wires::Red);
  status_red_brown();
  step_4();
}

void BLTouchTesting::step_1b_quick() {
  status_pin();
  step_4();
}

//! BLTouch deploy and stow (self test)
void BLTouchTesting::step_2() {
  set_bits(tested_, Wires::Orange);
  pages.show(Page::BLTouchTesting2, ACTION);
  bltouch._selftest();
}

void BLTouchTesting::step_2_yes() {
  set_bits(ok_, Wires::Orange);
  step_3();
}

void BLTouchTesting::step_2_no() {
  status_orange();
  step_4();
}

void BLTouchTesting::step_3() {
  set_bits(tested_, Wires::White | Wires::Black);

  wait.wait(F("Testing BLTouch triggering, please wait..."));

  // Adapted from M43 code
  // This code will try to detect a BLTouch probe
  bltouch._reset();
  bltouch._stow();
  if(triggered()) { status_white_black(); step_4(); return; }

  bltouch._set_SW_mode();
  if(!triggered()) { status_white_black(); step_4(); return; }

  bltouch._deploy();
  if(triggered()) { status_white_black(); step_4(); return; }

  bltouch._stow();

  // BLTouch Classic 1.2, 1.3, Smart 1.0, 2.0, 2.2, 3.0, 3.1 detected
  // Check for a 3.1 by letting the user trigger it, later

  bltouch._deploy();
  safe_delay(500);

  pages.show(Page::BLTouchTesting3, ACTION);

  // Wait 30 seconds for user to trigger probe
  for(uint16_t j = 0; j < 500 * 30; j++) {
    safe_delay(2);

    if(0 == j % (500 * 1)) gcode.reset_stepper_timeout();    // Keep steppers powered

    if(triggered()) {
      uint16_t probe_counter = 0; // Pulse width / 2
      for(probe_counter = 0; probe_counter < 15 && triggered(); ++probe_counter) safe_delay(2);
      Log::log() << F("BLTouch pulse width 0x") << probe_counter << Log::endl();

      wait.wait();
      if(probe_counter < 4) {
        status.set(F("Noise detected"));
        step_4();
        return;
      }

      status.set(probe_counter == 15 ? F("BLTouch 3.1 detected") : F("BLTouch V3.0 or lower detected"));
      bltouch._stow();
      set_bits(ok_, Wires::White | Wires::Black);
      step_4();
      return;
    }
  }

  status_white_black();
  step_4();
}

void BLTouchTesting::step_4() {
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

  pages.show(Page::BLTouchTesting4, ACTION);
}

#endif

}
