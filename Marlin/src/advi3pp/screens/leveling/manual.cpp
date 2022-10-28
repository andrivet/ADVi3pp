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
#include "manual.h"
#include "xtwist.h"
#include "../../core/core.h"
#include "../../screens/core/wait.h"

namespace ADVi3pp {

namespace {
    constexpr xyz_feedrate_t homing_feedrate_mm_m = HOMING_FEEDRATE_MM_M;
    constexpr float FEEDRATE_XY = MMM_TO_MMS(homing_feedrate_mm_m.x);
    constexpr float FEEDRATE_Z = MMM_TO_MMS(homing_feedrate_mm_m.z);
}

ManualLeveling manual_leveling;

//! Execute a Manual Leveling command
//! @param key_value    The sub-action to handle
//! @return             True if the action was handled
bool ManualLeveling::on_dispatch(KeyValue key_value) {
  if(Parent::on_dispatch(key_value))
      return true;

  switch(key_value) {
    case KeyValue::LevelingPoint1:  point1_command(); break;
    case KeyValue::LevelingPoint2:  point2_command(); break;
    case KeyValue::LevelingPoint3:  point3_command(); break;
    case KeyValue::LevelingPoint4:  point4_command(); break;
    case KeyValue::LevelingPoint5:  point5_command(); break;
    case KeyValue::LevelingPointA:  pointA_command(); break;
    case KeyValue::LevelingPointB:  pointB_command(); break;
    case KeyValue::LevelingPointC:  pointC_command(); break;
    case KeyValue::LevelingPointD:  pointD_command(); break;
    default:                        return false;
  }

  return true;
}

//! Execute the Back command
void ManualLeveling::on_back_command() {
#if HAS_LEVELING
  ExtUI::setLevelingActive(true); // Enable back compensation
#endif
  ExtUI::setAxisPosition_mm(Z_AFTER_HOMING, ExtUI::Z, FEEDRATE_Z);
  Parent::on_back_command();
}

void ManualLeveling::on_abort() {
#if HAS_LEVELING
  ExtUI::setLevelingActive(true); // Enable back compensation
#endif
}

void ManualLeveling::on_save_command() {
#if HAS_LEVELING
  ExtUI::setLevelingActive(false); // Disable ABL mesh (already disabled but prefer to be explicit)
#endif
  ExtUI::setAxisPosition_mm(Z_AFTER_HOMING, ExtUI::Z, FEEDRATE_Z);
  Parent::on_save_command();
}


//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
void ManualLeveling::on_enter() {
  pages.save_forward_page();

  if(ExtUI::getActualTemp_celsius(ExtUI::E0) > 50)
    wait.wait_back_continue(F("Warning: Hotend is hot"),
                            WaitCallback{this, &ManualLeveling::abort},
                            WaitCallback{this, &ManualLeveling::start});
  else
    start();
}

bool ManualLeveling::abort() {
  return true;
}

bool ManualLeveling::start() {
  wait.wait(F("Homing..."));
  core.inject_commands(F("G28 O")); // Homing
#if HAS_LEVELING
  ExtUI::setLevelingActive(false); // We do not want compensation during manual leveling
#endif
  background_task.set(Callback{this, &ManualLeveling::leveling_task}, 200);
  return false;
}

//! Leveling Background task.
void ManualLeveling::leveling_task() {
  if(!ExtUI::isMachineHomed())
    return;

  background_task.clear();
  pages.show(PAGE, ACTION);
}

void ManualLeveling::move(float x, float y) {
  ExtUI::setAxisPosition_mm(Z_HOMING_HEIGHT, ExtUI::Z, FEEDRATE_Z);

  float positions[2] = {x, y};
  ExtUI::axis_t axis[2] = { ExtUI::X, ExtUI::Y };
  ExtUI::setMultipleAxisPosition_mm(2, positions, axis, FEEDRATE_XY);

  ExtUI::setAxisPosition_mm(0, ExtUI::Z, FEEDRATE_Z);
}

//! Handle leveling point #1.
void ManualLeveling::point1_command() {
  move(30, 30);
}

//! Handle leveling point #2.
void ManualLeveling::point2_command() {
  move(30, 170);
}

//! Handle leveling point #3.
void ManualLeveling::point3_command() {
  move(170, 170);
}

//! Handle leveling point #4.
void ManualLeveling::point4_command() {
  move(170, 30);
}

//! Handle leveling point #5.
void ManualLeveling::point5_command() {
  move(100, 100);
}

//! Handle leveling point #A.
void ManualLeveling::pointA_command() {
  move(100, 30);
}

//! Handle leveling point #B.
void ManualLeveling::pointB_command() {
  move(30, 100);
}

//! Handle leveling point #C.
void ManualLeveling::pointC_command() {
  move(100, 170);
}

//! Handle leveling point #D.
void ManualLeveling::pointD_command() {
  move(170, 100);
}

}
