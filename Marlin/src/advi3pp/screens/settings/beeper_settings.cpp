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
bool BeeperSettings::do_dispatch(KeyValue key_value) {
  if(Parent::do_dispatch(key_value))
    return true;

  switch(key_value)
  {
    case KeyValue::BuzzOnAction:        on_action_command(); break;
    case KeyValue::BuzzOnPress:         on_press_command(); break;
    default:                            return false;
  }

  return true;
}

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page BeeperSettings::do_prepare_page()
{
  buzz_on_action_ =  buzzer.is_buzz_on_action_enabled();
  buzz_on_press_ = buzzer.is_buzz_on_press_enabled();
  buzz_duration_ = buzzer.get_buzz_duration();

  send_values();
  return Page::BuzzerSettings;
}

void BeeperSettings::do_save_command() {
  get_values();
  buzzer.set_settings(buzz_on_action_, buzz_on_press_, buzz_duration_);
  Parent::do_save_command();
}

void BeeperSettings::send_values() const
{
  WriteRamRequest{Variable::Value0}.write_words(buzz_on_action_, buzz_on_press_);
  WriteRamRequest{Variable::BeepDuration}.write_words(buzz_duration_ * 10ul);
}

bool BeeperSettings::get_values() {
  ReadRam frame{Variable::Value0};
  if(!frame.send_receive(2))
  {
    Log::error() << F("Receiving Frame (Buzzer ExtendedSettings)") << Log::endl();
    return false;
  }

  buzz_on_action_ = frame.read_word();
  buzz_on_press_ = frame.read_word();

  ReadRam frame2{Variable::BeepDuration};
  if(!frame.send_receive(1))
  {
    Log::error() << F("Receiving Frame (Buzzer ExtendedSettings)") << Log::endl();
    return false;
  }

  buzz_duration_ = frame2.read_word() / 10;
}

//! Handle the change duration command.
void BeeperSettings::duration_command(uint16_t duration)
{
    buzzer.buzz(duration / 10);
}

void BeeperSettings::on_action_command() {
  buzz_on_action_ = !buzz_on_action_;
  send_values();
}

void BeeperSettings::on_press_command() {
  buzz_on_press_ = !buzz_on_press_;
  send_values();
}

}
