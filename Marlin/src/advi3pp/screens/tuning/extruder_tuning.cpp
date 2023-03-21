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
#include "extruder_tuning.h"
#include "../../core/core.h"
#include "../../core/dgus.h"
#include "../../core/wait.h"
#include "../settings/step_settings.h"

namespace ADVi3pp {

namespace {
  constexpr uint16_t FILAMENT_TO_EXTRUDE = 100; //!< Filament to extrude (10 cm)
  constexpr uint16_t REMAINING_FILAMENT = 20; //!< Amount of filament supposes tp remain after extruding (2 cm)
}

ExtruderTuning extruder_tuning;

//! Handle Extruder Tuning command
//! @param key_value    The sub-action to handle
//! @return             True if the action was handled
bool ExtruderTuning::on_dispatch(KeyValue key_value) {
  switch(key_value) {
    case KeyValue::TuningStart:     start_command(); return true;
    case KeyValue::TuningSettings:  settings_command(); return true;
    default:                        break;
  }

  // Do this after since we handle Save before
  if(Parent::on_dispatch(key_value))
    return true;

  return false;
}

void ExtruderTuning::send_data() {
  WriteRamRequest{Variable::Value0}.write_word(settings.get_last_used_temperature(TemperatureKind::Hotend));
}

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
bool ExtruderTuning::on_enter() {
  pages.save_forward_page();
  send_data();
  previous_z_ = Core::ensure_z_enough_room();
  return true;
}

void ExtruderTuning::on_back_command() {
  ExtUI::setAxisPosition_mm(previous_z_, ExtUI::Z, 20);
  Parent::on_back_command();
}

void ExtruderTuning::on_save_command() {
  ExtUI::setAxisPosition_mm(previous_z_, ExtUI::Z, 20);
  Parent::on_save_command();
}

//! Start extruder tuning.
void ExtruderTuning::start_command() {
  ReadRam frame{Variable::Value0};
  if (!frame.send_receive(1)) {
    Log::error() << F("Receiving Frame (Target Temperature)") << Log::endl();
    return;
  }

  uint16_t temperature = frame.read_word();
  ExtUI::setTargetTemp_celsius(temperature, ExtUI::E0);
  wait.wait_back(F("Heating the extruder..."), WaitCallback{this, &ExtruderTuning::cancel_heating});
  background_task.set(Callback{this, &ExtruderTuning::heating}, 500);
}

void ExtruderTuning::heating() {
  if(ExtUI::getActualTemp_celsius(ExtUI::E0) < ExtUI::getTargetTemp_celsius(ExtUI::E0) - 2)
    return;
  background_task.clear();
  extrude();
}

bool ExtruderTuning::cancel_heating() {
  background_task.clear();
  ExtUI::setTargetTemp_celsius(0, ExtUI::E0);
  return true;
}

void ExtruderTuning::extrude() {
  extruded_ = ExtUI::getAxisPosition_mm(ExtUI::E0);
  wait.wait_back(F("Extrude filament..."), WaitCallback{this, &ExtruderTuning::cancel_extrude});
  background_task.set(Callback{this, &ExtruderTuning::extruding}, 100);
  ExtUI::setAxisPosition_mm(extruded_ + FILAMENT_TO_EXTRUDE, ExtUI::E0);
}

void ExtruderTuning::extruding() {
  if(ExtUI::isMoving())
    return;
  background_task.clear();
  ExtUI::setTargetTemp_celsius(0, ExtUI::E0);

  extruded_ = ExtUI::getAxisPosition_mm(ExtUI::E0) - extruded_;
  // Always set default to 20mm
  WriteRamRequest{Variable::Value0}.write_word(200);

  pages.show(Page::ExtruderTuningMeasure, ACTION);
}

bool ExtruderTuning::cancel_extrude() {
  background_task.clear();
  ExtUI::setTargetTemp_celsius(0, ExtUI::E0);
  ExtUI::stopMove();
  return true;
}

//! Compute the extruder (E axis) new value and show the steps settings.
void ExtruderTuning::settings_command() {
  ReadRam frame{Variable::Value0};
  if(!frame.send_receive(1)) {
    Log::error() << F("Receiving Frame (Measures)") << Log::endl();
    return;
  }

  uint16_t e = frame.read_word();
  // Note: e is divided by 10 because the LCD panel gives a value in 0.1 mm unit
  // Formula: new_value = old_value * theoretical_extruded / actual_extruded
  auto new_value = ExtUI::getAxisSteps_per_mm(ExtUI::E0) * extruded_ / (extruded_ + REMAINING_FILAMENT - e / 10.0);

  ExtUI::setAxisSteps_per_mm(new_value, ExtUI::E0);
  steps_settings.show();
}

}
