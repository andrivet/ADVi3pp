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
#include "baby_steps.h"
#include "../../core/dgus.h"

namespace ADVi3pp {

//! List of multipliers in Print Settings
const int BABYSTEPS_MULTIPLIERS[] = {8, 16, 32};

BabySteps baby_steps;


//! Handle Baby Steps command
//! @param key_value    The sub-action to handle
//! @return             True if the action was handled
bool BabySteps::on_dispatch(KeyValue key_value) {
  if(Parent::on_dispatch(key_value))
    return true;

  switch(key_value) {
    case KeyValue::Baby1:       multiplier_ = Multiplier::M1; break;
    case KeyValue::Baby2:       multiplier_ = Multiplier::M2; break;
    case KeyValue::Baby3:       multiplier_ = Multiplier::M3; break;
    default:                    return false;
  }

  send_multiplier();
  return true;
}

//! Get the value corresponding the the current multiplier.
//! @return The value of the current multiplier (in steps), or the first one in the case of an invalid multiplier
int BabySteps::get_multiplier_value() const {
  if(multiplier_ < Multiplier::M1 || multiplier_ > Multiplier::M3) {
    Log::error() << F("Invalid multiplier value: ") << static_cast<uint16_t >(multiplier_) << Log::endl();
    return BABYSTEPS_MULTIPLIERS[0];
  }

  return BABYSTEPS_MULTIPLIERS[static_cast<uint16_t>(multiplier_)];
}

//! Send the current data to the LCD panel.
void BabySteps::send_multiplier() const {
  WriteRamRequest{Variable::Value0}.write_words(multiplier_);
}

//! Send the current data to the LCD panel.
void BabySteps::send_z_offset() {
  WriteRamRequest{Variable::Value1}.write_words(lround(100.0 * ExtUI::getZOffset_mm()));
}


//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
bool BabySteps::on_enter() {
  save_ = false;
  send_multiplier();
  background_task.set(Callback{this, &BabySteps::send_z_offset}, 200);
  return true;
}

void BabySteps::on_back_command() {
  background_task.clear();
  if(save_) settings.save();
  Parent::on_back_command();
}

//! Handle the -Babystep command
void BabySteps::baby_minus_command() {
  save_= true;
  ExtUI::smartAdjustAxis_steps(-get_multiplier_value(), ExtUI::Z, true);
}

//! Handle the +Babystep command
void BabySteps::baby_plus_command() {
  save_= true;
  ExtUI::smartAdjustAxis_steps(get_multiplier_value(), ExtUI::Z, true);
}

}
