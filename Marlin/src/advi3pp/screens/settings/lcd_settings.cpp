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
bool LcdSettings::on_dispatch(KeyValue key_value) {
  if(Parent::on_dispatch(key_value))
      return true;

  switch(key_value) {
    case KeyValue::LCDDimming:          dimming_command(); break;
    default:                            return false;
  }

  return true;
}

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
bool LcdSettings::on_enter() {
  dimming_ = dimming.is_enabled();
  auto dimming_time = dimming.get_dimming_time();
  auto normal_brightness = dimming.get_normal_brightness();
  auto dimming_brightness = dimming.get_dimming_brightness();
  send_values(dimming_time, normal_brightness, dimming_brightness);
  return true;
}

void LcdSettings::on_back_command() {
  dimming.send_brightness_to_lcd(); // Restore previous brightness
  Parent::on_back_command();
}

void LcdSettings::on_abort() {
  dimming.send_brightness_to_lcd(); // Restore previous brightness
}

void LcdSettings::on_save_command() {
  uint16_t dimming_time; uint8_t normal_brightness, dimming_brightness;
  if(get_values(dimming_time, normal_brightness, dimming_brightness))
    dimming.set_settings(dimming_, dimming_time, normal_brightness, dimming_brightness);
  dimming.send_brightness_to_lcd();
  Parent::on_save_command();
}

void LcdSettings::send_values(uint16_t time, uint8_t normal, uint8_t dimmed) {
  WriteRamRequest{Variable::Value0}.write_words(dimming_, time);
  WriteRamRequest{Variable::NormalBrightness}.write_words(normal, dimmed);
}

bool LcdSettings::get_values(uint16_t &time, uint8_t &normal, uint8_t &dimmed) const {
  ReadRam frame{Variable::Value1};
  if(!frame.send_receive(1)) {
    Log::error() << F("Receiving Frame (Dimming time)") << Log::endl();
    return false;
  }
  time = frame.read_word();

  ReadRam frame2{Variable::NormalBrightness};
  if(!frame2.send_receive(2)) {
    Log::error() << F("Receiving Frame (brightness)") << Log::endl();
    return false;
  }
  normal = frame2.read_word();
  dimmed = frame2.read_word();
  return true;
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
