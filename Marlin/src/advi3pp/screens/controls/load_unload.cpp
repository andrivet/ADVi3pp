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

#include "../../../inc/MarlinConfig.h"
#include "load_unload.h"
#include "../../core/wait.h"


namespace ADVi3pp {

LoadUnload load_unload;


//! Handle Load & Unload actions.
//! @param key_value    The sub-action to handle
//! @return             True if the action was handled
bool LoadUnload::on_dispatch(KeyValue key_value) {
  if(Parent::on_dispatch(key_value))
    return true;

  switch(key_value){
    case KeyValue::Load:    load_command(); break;
    case KeyValue::Unload:  unload_command(); break;
    default:                return false;
  }

  return true;
}

void LoadUnload::send_data() {
  WriteRamRequest{Variable::Value0}.write_word(static_cast<uint16_t>(ExtUI::getDefaultTemp_celsius(ExtUI::E0)));
}

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
bool LoadUnload::on_enter() {
  send_data();
  previous_z_ = Core::ensure_z_enough_room();
  return true;
}

void LoadUnload::on_back_command() {
  ExtUI::setAxisPosition_mm(previous_z_, ExtUI::Z, 20);
  Parent::on_back_command();
}

//! Prepare Load or Unload step #1: set the target temperature, setup the next step and display a wait message
//! @param background Background task to detect if it is time for step #2
void LoadUnload::prepare(float length, feedRate_t feedrate) {
  ReadRam frame{Variable::Value0};
  if(!frame.send_receive(1)) {
    Log::error() << F("Receiving Frame (Target Temperature)") << Log::endl();
    return;
  }
  const auto target_temp = frame.read_word();
  length_ = length;
  feedrate_ = feedrate;

  ExtUI::setTargetTemp_celsius(target_temp, ExtUI::E0, true);
  ExtUI::setDefaultTemp_celsius(target_temp, ExtUI::E0);
  settings.save();

  wait.wait_back(F("Heating the extruder..."), WaitCallback{this, &LoadUnload::cancel_heating});
  background_task.set(Callback{this, &LoadUnload::heating_task}, 250);
  ExtUI::setHostKeepaliveState(GcodeSuite::IN_PROCESS);
}

void LoadUnload::heating_task() {
  if(ExtUI::getActualTemp_celsius(ExtUI::E0) < ExtUI::getTargetTemp_celsius(ExtUI::E0) - 2) return;
  background_task.clear();
  extrude();
}

bool LoadUnload::cancel_heating() {
  ExtUI::setHostKeepaliveState(GcodeSuite::NOT_BUSY);
  background_task.clear();
  ExtUI::setTargetTemp_celsius(0, ExtUI::E0);
  return true;
}

void LoadUnload::extrude() {
  ExtUI::setAxisPosition_mm(ExtUI::getAxisPosition_mm(ExtUI::E0) + length_, ExtUI::E0, feedrate_);
  wait.wait_back(length_ > 0 ? F("Load filament...") : F("Unload filament..."), WaitCallback{this, &LoadUnload::cancel_extrude});
  background_task.set(Callback{this, &LoadUnload::extrude_task});
}

void LoadUnload::extrude_task() {
  if(ExtUI::isMoving()) return;
  ExtUI::setHostKeepaliveState(GcodeSuite::NOT_BUSY);
  background_task.clear();
  ExtUI::setTargetTemp_celsius(0, ExtUI::E0);
  pages.clear_temporaries();
}

bool LoadUnload::cancel_extrude() {
  ExtUI::setHostKeepaliveState(GcodeSuite::NOT_BUSY);
  background_task.clear();
  ExtUI::setTargetTemp_celsius(0, ExtUI::E0);
  ExtUI::stopMove();
  pages.clear_temporaries();
  return false;
}

//! Start Load action.
void LoadUnload::load_command() {
  prepare(FILAMENT_CHANGE_SLOW_LOAD_LENGTH, FILAMENT_CHANGE_SLOW_LOAD_FEEDRATE);
}

//! Start Unload action.
void LoadUnload::unload_command() {
    prepare(-FILAMENT_CHANGE_UNLOAD_LENGTH, FILAMENT_CHANGE_UNLOAD_FEEDRATE);
}


}
