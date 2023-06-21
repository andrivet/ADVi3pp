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
#include "beeper_settings.h"
#include "../../core/buzzer.h"
#include "../../core/dgus.h"

namespace ADVi3pp {

BeeperSettings beeper_settings;


//! Handle command
//! @param key_value    The sub-action to handle
//! @return             True if the action was handled
bool BeeperSettings::on_dispatch(KeyValue key_value) {
  if(Parent::on_dispatch(key_value))
    return true;

  switch(key_value) {
    case KeyValue::BuzzOnAction:        on_action_command(); break;
    case KeyValue::BuzzOnPress:         on_press_command(); break;
    default:                            return false;
  }

  return true;
}

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
bool BeeperSettings::on_enter() {
  buzz_on_action_ =  buzzer.is_buzz_on_action_enabled();
  buzz_on_press_ = buzzer.is_buzz_on_press_enabled();
  auto buzz_duration = buzzer.get_buzz_duration();

  send_values(buzz_on_action_, buzz_on_press_, buzz_duration);
  return true;
}

void BeeperSettings::on_save_command() {
  bool on_action, on_press; uint8_t duration;
  if(get_values(on_action, on_press, duration))
    buzzer.set_settings(on_action, on_press, duration);
  Parent::on_save_command();
}

void BeeperSettings::send_values(bool on_action, bool on_press, uint8_t duration) const {
  WriteRamRequest{Variable::Value0}.write_words(on_action, on_press);
  WriteRamRequest{Variable::BeepDuration}.write_words(duration * 10ul);
}

bool BeeperSettings::get_values(bool &on_action, bool &on_press, uint8_t &duration) {
  ReadRam frame{Variable::Value0};
  if(!frame.send_receive(2)) {
    Log::error() << F("Receiving Frame (Buzzer ExtendedSettings)") << Log::endl();
    return false;
  }

  on_action = frame.read_word();
  on_press = frame.read_word();

  ReadRam frame2{Variable::BeepDuration};
  if(!frame2.send_receive(1)) {
    Log::error() << F("Receiving Frame (Buzzer ExtendedSettings)") << Log::endl();
    return false;
  }

  duration = frame2.read_word() / 10;
  Log::log() << duration << Log::endl();
  return true;
}

//! Handle the change duration command.
void BeeperSettings::duration_command(uint16_t duration) {
    buzzer.buzz(duration / 10);
}

void BeeperSettings::on_action_command() {
  buzz_on_action_ = !buzz_on_action_;
  WriteRamRequest{Variable::Value0}.write_words(buzz_on_action_);
}

void BeeperSettings::on_press_command() {
  buzz_on_press_ = !buzz_on_press_;
  WriteRamRequest{Variable::Value1}.write_words(buzz_on_press_);
}

}
