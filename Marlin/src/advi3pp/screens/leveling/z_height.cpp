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
#include "../../core/core.h"
#include "../../core/wait.h"
#include "z_height.h"

namespace ADVi3pp {

namespace {
  constexpr xyz_feedrate_t homing_feedrate_mm_m = HOMING_FEEDRATE_MM_M;
  constexpr float FEEDRATE_XY = MMM_TO_MMS(homing_feedrate_mm_m.x);
  constexpr float FEEDRATE_Z = MMM_TO_MMS(homing_feedrate_mm_m.z);
  constexpr unsigned MINIMAL_CLICK_DELAY = 200; // ms
}

SensorZHeight sensor_z_height;

#ifdef ADVi3PP_PROBE

const double SENSOR_Z_HEIGHT_MULTIPLIERS[] = {0.02, 0.10, 1.0};

//! Handle Sensor Z Height command
//! @param key_value    The sub-action to handle
//! @return             True if the action was handled
bool SensorZHeight::on_dispatch(KeyValue key_value) {
  if(Parent::on_dispatch(key_value))
    return true;

  switch(key_value) {
    case KeyValue::Multiplier1:     multiplier1_command(); break;
    case KeyValue::Multiplier2:     multiplier2_command(); break;
    case KeyValue::Multiplier3:     multiplier3_command(); break;
    default:                        return false;
  }

  return true;
}

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
bool SensorZHeight::on_enter() {
  pages.save_forward_page();

  old_offset_ = ExtUI::getZOffset_mm();
  last_click_time_ = 0;
  ExtUI::setZOffset_mm(0); // Before homing otherwise, Marlin is lost
  ExtUI::setAbsoluteZAxisPosition_mm(ExtUI::getAxisPosition_mm(ExtUI::Z) + old_offset_);

  wait.homing(WaitCallback{this, &SensorZHeight::on_homed});
  return false;
}

//! Reset Sensor Z Height data.
void SensorZHeight::reset() {
  multiplier_ = Multiplier::M1;
}

//! Check if the printer is homed, and continue the Z Height Tuning process.
bool SensorZHeight::on_homed() {
  pages.show(PAGE, ACTION);
  reset();

  float positions[2] = {X_CENTER, Y_CENTER};
  ExtUI::axis_t axis[2] = { ExtUI::X, ExtUI::Y };
  ExtUI::setMultipleAxisPosition_mm(2, positions, axis, FEEDRATE_XY);
  ExtUI::setAxisPosition_mm(0, ExtUI::Z, FEEDRATE_Z);
  ExtUI::setSoftEndstopState(false);

  send_data();
  return true;
}

//! Execute the Back command
void SensorZHeight::on_back_command() {
  ExtUI::setSoftEndstopState(true);
  ExtUI::setZOffset_mm(old_offset_);
  ExtUI::setAbsoluteZAxisPosition_mm(ExtUI::getAxisPosition_mm(ExtUI::Z) - old_offset_);
  ExtUI::setAxisPosition_mm(Z_AFTER_HOMING, ExtUI::Z, FEEDRATE_Z);
  Parent::on_back_command();
}

void SensorZHeight::on_abort() {
  ExtUI::setSoftEndstopState(true);
  ExtUI::setZOffset_mm(old_offset_);
}


//! Handles the Save (Continue) command
void SensorZHeight::on_save_command() {
  // Current Z position becomes Z offset
  ExtUI::setSoftEndstopState(true);
  ExtUI::setZOffset_mm(ExtUI::getAxisPosition_mm(ExtUI::Z));
  ExtUI::setAbsoluteZAxisPosition_mm(0);
  ExtUI::setAxisPosition_mm(Z_AFTER_HOMING, ExtUI::Z, FEEDRATE_Z);
  Parent::on_save_command();
}

//! Change the multiplier.
void SensorZHeight::multiplier1_command() {
  multiplier_ = Multiplier::M1;
  send_data();
}

//! Change the multiplier.
void SensorZHeight::multiplier2_command() {
  multiplier_ = Multiplier::M2;
  send_data();
}

//! Change the multiplier.
void SensorZHeight::multiplier3_command() {
  multiplier_ = Multiplier::M3;
  send_data();
}

//! Change the position of the nozzle (-Z).
void SensorZHeight::minus() {
  adjust_height(-get_multiplier_value());
}

//! Change the position of the nozzle (+Z).
void SensorZHeight::plus() {
  adjust_height(+get_multiplier_value());
}

//! Get the current multiplier value on the LCD panel.
double SensorZHeight::get_multiplier_value() const {
  if(multiplier_ < Multiplier::M1 || multiplier_ > Multiplier::M3)     {
    Log::error() << F("Invalid multiplier value: ") << static_cast<uint16_t >(multiplier_) << Log::endl();
    return SENSOR_Z_HEIGHT_MULTIPLIERS[0];
  }

  return SENSOR_Z_HEIGHT_MULTIPLIERS[static_cast<uint16_t>(multiplier_)];
}

//! Adjust the Z height.
//! @param offset Offset for the adjustment.
void SensorZHeight::adjust_height(double offset) {
  if(!ELAPSED(millis(), last_click_time_))
    return;
  last_click_time_ = millis();
  ExtUI::setAxisPosition_mm(ExtUI::getAxisPosition_mm(ExtUI::Z) + offset, ExtUI::Z, FEEDRATE_Z);
  send_data();
}

//! Send the current data (i.e. multiplier) to the LCD panel.
void SensorZHeight::send_data() const {
  WriteRamRequest{Variable::Value0}.write_word(static_cast<uint16_t>(multiplier_));
}

#endif

}
