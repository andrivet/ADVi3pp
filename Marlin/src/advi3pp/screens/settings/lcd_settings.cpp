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
#include "lcd_settings.h"
#include "../../core/dgus.h"
#include "../../core/dimming.h"

namespace ADVi3pp {

LcdSettings lcd_settings;


//! Handle LCD ExtendedSettings command
//! @param key_value    The sub-action to handle
//! @return             True if the action was handled
bool LcdSettings::do_dispatch(KeyValue key_value) {
  if(Parent::do_dispatch(key_value))
      return true;

  switch(key_value)
  {
      case KeyValue::LCDDimming:          dimming_command(); break;
      default:                            return false;
  }

  return true;
}

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page LcdSettings::do_prepare_page() {
  dimming_ = dimming.is_enabled();
  normal_brightness_ = dimming.get_normal_brightness();
  dimming_brightness_ = dimming.get_dimming_brightness();
  send_values();
  return Page::LCD;
}

void LcdSettings::do_back_command() {
  dimming.send_brightness_to_lcd(); // Restore previous brightness
  Parent::do_back_command();
}

void LcdSettings::do_save_command() {
  auto dimming_time = get_dimming_time();
  dimming.set_settings(dimming_, dimming_time, normal_brightness_, dimming_brightness_);
  dimming.send_brightness_to_lcd();
  Parent::do_save_command();
}

void LcdSettings::send_values() {
  auto dimming_time = dimming.get_dimming_time();
  WriteRamRequest{Variable::Value0}.write_words(dimming_, dimming_time);
  WriteRamRequest{Variable::NormalBrightness}.write_words(normal_brightness_, dimming_brightness_);
}

uint8_t LcdSettings::get_dimming_time() const {
  ReadRam frame{Variable::Value0};
  if(!frame.send_receive(2))
  {
    Log::error() << F("Receiving Frame (Dimming time)") << Log::endl();
    return 1;
  }

  return frame.read_word();
}

//! Handle the Dimming (On/Off) command
void LcdSettings::dimming_command() {
  dimming_ = !dimming_;
  WriteRamRequest{Variable::Value0}.write_words(dimming_);
}

//! Handle the change brightness command.
void LcdSettings::normal_brightness_command(uint16_t brightness) {
  dimming.send_brightness_to_lcd(brightness);
}

//! Handle the change brightness command.
void LcdSettings::dimming_brightness_command(uint16_t brightness) {
  dimming.send_brightness_to_lcd(brightness);
}


}
