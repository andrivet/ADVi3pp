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
#include "xtwist.h"

namespace ADVi3pp {

namespace {
    constexpr xyz_feedrate_t homing_feedrate_mm_m = HOMING_FEEDRATE_MM_M;
    constexpr float FEEDRATE_X = MMM_TO_MMS(homing_feedrate_mm_m.x);
    constexpr float FEEDRATE_Y = MMM_TO_MMS(homing_feedrate_mm_m.y);
    constexpr float FEEDRATE_Z = MMM_TO_MMS(homing_feedrate_mm_m.z);
}

XTwist xtwist;

#ifdef ADVi3PP_PROBE

const double SENSOR_Z_HEIGHT_MULTIPLIERS[] = {0.02, 0.10, 1.0};
const int MARGIN = 10;


//! Handle Sensor Z Height command
//! @param key_value    The sub-action to handle
//! @return             True if the action was handled
bool XTwist::on_dispatch(KeyValue key_value) {
  if(Parent::on_dispatch(key_value))
    return true;

  switch(key_value) {
    case KeyValue::Multiplier1:     multiplier1_command(); break;
    case KeyValue::Multiplier2:     multiplier2_command(); break;
    case KeyValue::Multiplier3:     multiplier3_command(); break;
    case KeyValue::Point_L:         point_L_command(); break;
    case KeyValue::Point_M:         point_M_command(); break;
    case KeyValue::Point_R:         point_R_command(); break;
    default:                        return false;
  }

  return true;
}

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
void XTwist::on_enter() {
  if(!ExtUI::getLevelingActive()) {
    wait.wait_back(F("Please do an automated bed leveling."));
    return;
  }

  z_offsets_.fill(0);
  pages.save_forward_page();
  wait.home_and_wait(Callback{this, &XTwist::post_home_task});
}

//! Check if the printer is homed, and continue the process.
void XTwist::post_home_task() {
  if(!wait.check_homed())
    return;

  send_data();
  status.reset();

  ExtUI::setSoftEndstopState(false);
  point_M_command();
}

//! Execute the Back command
void XTwist::on_back_command() {
  // enable enstops, raise head
  ExtUI::setSoftEndstopState(true);
  ExtUI::setAxisPosition_mm(4, ExtUI::Z, FEEDRATE_Z);

  Parent::on_back_command();
}

//! Execute the Back command
void XTwist::on_abort() {
  // enable enstops, raise head
  ExtUI::setSoftEndstopState(true);
}

//! Handles the Save (Continue) command
void XTwist::on_save_command() {
  ExtUI::setXTwistStartSpacing(MARGIN, (X_BED_SIZE - MARGIN) / 2.0f);
  for(size_t i = 0; i < ExtUI::xTwistPoints; ++i) ExtUI::setXTwistZOffset(i, z_offsets_[i]);

  // enable enstops, raise head
  ExtUI::setSoftEndstopState(true);;
  ExtUI::setAxisPosition_mm(4, ExtUI::Z, FEEDRATE_Z);

  Parent::on_save_command();
}

//! Change the multiplier.
void XTwist::multiplier1_command() {
  multiplier_ = Multiplier::M1;
  send_data();
}

//! Change the multiplier.
void XTwist::multiplier2_command() {
  multiplier_ = Multiplier::M2;
  send_data();
}

//! Change the multiplier.
void XTwist::multiplier3_command() {
  multiplier_ = Multiplier::M3;
  send_data();
}

float XTwist::get_x_mm(Point x) const {
  return (x == Point::L ? MARGIN : (x == Point::R ? X_BED_SIZE - MARGIN : (X_BED_SIZE / 2.0f)));
}

void XTwist::move_x(Point x) {
  ExtUI::setAxisPosition_mm(4, ExtUI::Z, FEEDRATE_Z);
  ExtUI::setAxisPosition_mm(get_x_mm(x), ExtUI::X, FEEDRATE_X);
  ExtUI::setAxisPosition_mm(Y_BED_SIZE / 2.0f, ExtUI::Y, FEEDRATE_X);
  ExtUI::setAxisPosition_mm(z_offsets_[static_cast<size_t>(x)], ExtUI::Z, FEEDRATE_Z);

  point_ = x;
}

void XTwist::point_M_command() {
  move_x(Point::M);
}

void XTwist::point_L_command() {
  move_x(Point::L);
}

void XTwist::point_R_command() {
  move_x(Point::R);
}

//! Change the position of the nozzle (-Z).
void XTwist::minus() {
  adjust_height(-get_multiplier_value());
}

//! Change the position of the nozzle (+Z).
void XTwist::plus() {
  adjust_height(+get_multiplier_value());
}

//! Get the current multiplier value on the LCD panel.
double XTwist::get_multiplier_value() const {
  if(multiplier_ < Multiplier::M1 || multiplier_ > Multiplier::M3) {
    Log::error() << F("Invalid multiplier value: ") << static_cast<uint16_t >(multiplier_) << Log::endl();
    return SENSOR_Z_HEIGHT_MULTIPLIERS[0];
  }

  return SENSOR_Z_HEIGHT_MULTIPLIERS[static_cast<uint16_t>(multiplier_)];
}

//! Adjust the Z height.
//! @param offset Offset for the adjustment.
void XTwist::adjust_height(double offset_value) {
  ExtUI::setAxisPosition_mm(ExtUI::getAxisPosition_mm(ExtUI::Z) + offset_value, ExtUI::Z, FEEDRATE_Z);
  z_offsets_[static_cast<size_t>(point_)] = ExtUI::getAxisPosition_mm(ExtUI::Z);
}

//! Send the current data (i.e. multiplier) to the LCD panel.
void XTwist::send_data() const {
  WriteRamRequest{Variable::Value0}.write_word(static_cast<uint16_t>(multiplier_));
}

#endif

}
