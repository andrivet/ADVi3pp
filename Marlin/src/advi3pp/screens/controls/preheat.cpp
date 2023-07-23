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
#include "preheat.h"
#include "../../core/core.h"
#include "../../core/dgus.h"
#include "../../core/status.h"
#include "../print/temperatures.h"

namespace ADVi3pp {

Preheat preheat;

//! Handle Preheat actions.
//! @param key_value    Sub-action to handle
//! @return             True if the action was handled
bool Preheat::on_dispatch(KeyValue key_value) {
  if(Parent::on_dispatch(key_value))
    return true;

  switch(key_value) {
    case KeyValue::PresetPrevious:  previous_command(); break;
    case KeyValue::PresetNext:      next_command(); break;
    case KeyValue::Cooldown:        cooldown_command(); break;
    default:                        return false;
  }

  return true;
}

//! Send the presets t the LCD Panel
void Preheat::send_presets() {
  WriteRamRequest{Variable::Value0}.write_words(
    ExtUI::getMaterialPresetHotendTemp_celsius(index_),
    ExtUI::getMaterialPresetBedTemp_celsius(index_),
    ExtUI::getMaterialPresetFanSpeed_percent(index_)
  );

  ADVString<8> preset;
  preset << index_ + 1 << F(" / ") << NB_PRESETS;
  WriteRamRequest{Variable::ShortText0}.write_text(preset);
}

//! Retrieve presets values from the LCD Panel
void Preheat::retrieve_presets() {
  ReadRam frame{Variable::Value0};
  if(!frame.send_receive(3)) {
    Log::error() << F("Error receiving presets") << Log::endl();
    return;
  }

  uint16_t hotend = frame.read_word();
  uint16_t bed = frame.read_word();
  uint16_t fan = frame.read_word();

  ExtUI::setMaterialPreset(index_, hotend, bed, fan);
}

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
bool Preheat::on_enter() {
  send_presets();
  return true;
}

//! Handle Previous command
void Preheat::previous_command() {
  if(index_ <= 0)
    return;
  retrieve_presets();
  --index_;
  send_presets();
}

//! Handle Next command
void Preheat::next_command() {
  if(index_ >= NB_PRESETS - 1)
      return;
  retrieve_presets();
  ++index_;
  send_presets();
}

//! Handles the Save (Continue) command
void Preheat::on_save_command() {
  retrieve_presets();

  ExtUI::setTargetTemp_celsius(ExtUI::getMaterialPresetHotendTemp_celsius(index_), ExtUI::E0, true);
  ExtUI::setTargetTemp_celsius(ExtUI::getMaterialPresetBedTemp_celsius(index_), ExtUI::BED, true);
  ExtUI::setTargetFan_percent(ExtUI::getMaterialPresetFanSpeed_percent(index_), ExtUI::FAN0);

  settings.save();
  status.set(F("Preheat..."));
  temperatures.show();
}

//! Cooldown the bed and the nozzle, turn off the fan
void Preheat::cooldown_command() {
  // If printing, do nothing
  if(ExtUI::isPrinting()) return;
  ExtUI::setTargetTemp_celsius(0, ExtUI::BED);
  ExtUI::setTargetTemp_celsius(0, ExtUI::E0);
  ExtUI::setTargetFan_percent(0, ExtUI::FAN0);
  status.set(F("Cool down..."));
}

}
