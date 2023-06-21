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
#include "io.h"
#include "../../core/core.h"
#include "../../core/task.h"
#include "../../core/dgus.h"

namespace ADVi3pp {

//! List of digital pins for the Diagnosis page
const uint8_t diagnosis_digital_pins[] = {
  54,     // PF0 / ADC0 - A0
  24,     // PA2 / AD2
  23,     // PA1 / AD1
   6,     // PH3 / OC4A
  25,     // PA3 / AD3

  40,     // PG1 / !RD
  56,     // PF2 / ADC2 - A2
  36,     // PC1 / A9
  37,     // PC0 / A8

  34,     // PC3 / A11
  35,     // PC2 / A10
  32,     // PC5 / A13
  33,     // PC4 / A12
};

//! List of analog pins for the Diagnosis page
const uint8_t diagnosis_analog_pins[] = {55, 68, 54, 56}; // A1, A14, A0, A2

IO io;


//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
bool IO::on_enter() {
  background_task.set(Callback{this, &IO::send_data}, 250);
  return true;
}

//! Execute the Back command
void IO::on_back_command() {
  background_task.clear();
  Parent::on_back_command();
}


//! Send the current data to the LCD panel.
void IO::send_data() {
  WriteRamRequest request{Variable::Value0};

  for(size_t i = 0; i < adv::count_of(diagnosis_digital_pins); ++i) {
    auto var = static_cast<Variable>(static_cast<uint16_t>(Variable::Value0) + i);
    WriteRamRequest{var}.write_word(static_cast<uint16_t>(Core::get_pin_state(diagnosis_digital_pins[i])));
  }

  for(size_t i = 0; i < adv::count_of(diagnosis_analog_pins); ++i) {
    auto var = static_cast<Variable>(static_cast<uint16_t>(Variable::Value0) + 0x20 + i);
    WriteRamRequest{var}.write_word(static_cast<uint16_t>(analogRead(diagnosis_analog_pins[i])));
  }
}

}
