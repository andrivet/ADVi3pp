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
#include "pid_settings.h"
#include "../../core/dgus.h"
#include "../../core/pid.h"
#include "../tuning/pid_tuning.h"

namespace ADVi3pp {

PidSettings pid_settings;


//! Handle PID Settings command
//! @param key_value    The sub-action to handle
//! @return             True if the action was handled
bool PidSettings::on_dispatch(KeyValue key_value) {
  if(Parent::on_dispatch(key_value))
    return true;

  switch(key_value) {
    case KeyValue::PidSettingsHotend:   hotend_command(); break;
    case KeyValue::PidSettingsBed:      bed_command(); break;
    case KeyValue::PidSettingPrevious:  previous_command(); break;
    case KeyValue::PidSettingNext:      next_command(); break;
    default:                            return false;
  }

  return true;
}

//! Handle the select Hotend PID command
void PidSettings::hotend_command() {
  from_lcd();
  kind_ = TemperatureKind::Hotend;
  to_lcd();
}

//! Handle the select Bed PID command
void PidSettings::bed_command() {
  from_lcd();
  kind_ = TemperatureKind::Bed;
  to_lcd();
}

//! Handle the show previous PID values command
void PidSettings::previous_command() {
  if(index_ <= 0)
    return;
  from_lcd();
  index_ -= 1;
  to_lcd();
}

//! Handle the show next PID values command
void PidSettings::next_command() {
  if(index_ >= Pid::NB_PIDs - 1)
    return;
  from_lcd();
  index_ += 1;
  to_lcd();
}

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
bool PidSettings::on_enter() {
  to_lcd();
  return true;
}

//! Save the PID settings
void PidSettings::on_save_command() {
  from_lcd();
  assert(kind_ <= TemperatureKind::Hotend);
  pid.set_marlin_pid(kind_, index_);
  Parent::on_save_command();
}

//! Execute the Back command
void PidSettings::on_back_command() {
  settings.restore();
  Parent::on_back_command();
}


//! Send the current data to the LCD panel.
void PidSettings::to_lcd() const {
  const PidValue& value = pid.get_pid(kind_, index_);
  WriteRamRequest{Variable::Value0}.write_words(
    kind_ == TemperatureKind::Hotend ? 0u : 1u,
    value.temperature_,
    value.Kp_ * 100,
    value.Ki_ * 100,
    value.Kd_ * 100
  );

  ADVString<8> indexes;
  indexes << index_ + 1 << F(" / ") << Pid::NB_PIDs;
  WriteRamRequest{Variable::ShortText0}.write_text(indexes);
}

//! Save the settings from the LCD Panel.
void PidSettings::from_lcd() {
  ReadRam response{Variable::Value0};
  if(!response.send_receive(5))
    return;

  uint16_t kind = response.read_word();
  uint16_t temperature = response.read_word();
  uint16_t p = response.read_word();
  uint16_t i = response.read_word();
  uint16_t d = response.read_word();

  kind_ = kind ? TemperatureKind::Bed : TemperatureKind::Hotend;
  PidValue& value = pid.get_pid(kind_, index_);

  value.Kp_ = static_cast<float>(p) / 100;
  value.Ki_ = static_cast<float>(i) / 100;
  value.Kd_ = static_cast<float>(d) / 100;
  value.temperature_ = temperature;

  pid.set_marlin_pid(kind_, index_);
}


}
