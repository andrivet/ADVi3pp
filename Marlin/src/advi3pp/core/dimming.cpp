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

#include "../parameters.h"
#include <Arduino.h>
#include "../../lcd/extui/ui_api.h"
#include "settings.h"
#include "dimming.h"
#include "dgus.h"
#include "logging.h"

namespace ADVi3pp {

Dimming dimming;

//! Constructor. Initialize dimming check time and dimming delay time
Dimming::Dimming()
{
    set_next_checking_time();
}

//! Set the next dimming check time
void Dimming::set_next_checking_time()
{
    next_check_time_ = millis() + 200;
}

void Dimming::set_settings(bool dimming, uint8_t dimming_time, uint8_t normal_brightness, uint8_t dimming_brightness) {
  enabled_ = dimming;
  dimming_time_ = dimming_time;
  ui.set_brightness(normal_brightness);
  dimming_brightness_ = dimming_brightness;
}

//! Store current data in permanent memory (EEPROM)
//! @param eeprom EEPROM writer
void Dimming::do_write(EepromWrite& eeprom) const {
  eeprom.write(enabled_);
  eeprom.write(dimming_brightness_);
  eeprom.write(dimming_time_);
}

//! Validate data from permanent memory (EEPROM).
//! @param eeprom EEPROM reader
bool Dimming::do_validate(EepromRead &eeprom) {
  bool enabled;
  uint8_t brightness, time;
  eeprom.read(enabled);
  eeprom.read(brightness);
  eeprom.read(time);
  return true;
}

//! Restore data from permanent memory (EEPROM).
//! @param eeprom EEPROM reader
void Dimming::do_read(EepromRead& eeprom) {
  eeprom.read(enabled_);
  eeprom.read(dimming_brightness_);
  eeprom.read(dimming_time_);
}

//! Reset settings
void Dimming::do_reset() {
  enabled_ = true;
  dimming_brightness_ = 5;
  dimming_time_ = 2;
}

//! Return the amount of data (in bytes) necessary to save settings in permanent memory (EEPROM).
//! @return Number of bytes
uint16_t Dimming::do_size_of() const {
  return sizeof(enabled_) + sizeof(dimming_brightness_) + sizeof(dimming_time_);
}


void Dimming::send()
{
    if(!is_enabled() || !dimmed_ || !ELAPSED(millis(), next_check_time_))
        return;
    set_next_checking_time();
    ReadRegisterRequest{Register::TouchPanelFlag}.write(1);
}

bool Dimming::receive()
{
  NoFrameLogging no_log{};
  bool received = false;

  ReadRegisterResponse response{Register::TouchPanelFlag};
  if(response.receive(false)) {
    received = true;

    // 0x5A means the panel was touched, we have to write 0 to clear the flag
    if(response.read_byte() == 0x5A) {
      no_log.allow();
      reset_touch();
      ui.refresh_screen_timeout();
      return true;
    }
  }

  if(!dimmed_ && is_enabled())
    ui.check_screen_timeout();

  return received;
}

void Dimming::reset_touch() {
  WriteRegisterRequest{Register::TouchPanelFlag}.write_byte(0);
}

//! Set the brightness of the LCD panel
void Dimming::send_brightness_to_lcd(uint8_t brightness)
{
  WriteRegisterRequest{Register::Brightness}.write_byte(brightness);
}

//! Set the brightness of the LCD panel
void Dimming::send_brightness_to_lcd()
{
  send_brightness_to_lcd(dimmed_ ? dimming_brightness_ : get_normal_brightness());
}

void Dimming::sleep_on() {
  if(dimmed_) return;
  dimmed_ = true;
  reset_touch();
  send_brightness_to_lcd();
}

void Dimming::sleep_off() {
  if(!dimmed_) return;
  dimmed_ = false;
  send_brightness_to_lcd();
}

}
