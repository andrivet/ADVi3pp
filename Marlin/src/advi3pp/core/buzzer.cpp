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
#include "logging.h"
#include "dgus.h"
#include "buzzer.h"


namespace ADVi3pp {

Buzzer buzzer;


void Buzzer::buzz(uint8_t duration) {
  send_buzz_command_to_lcd(duration);
}

void Buzzer::set_settings(bool buzz_on_action, bool buzz_on_press, uint8_t buzz_duration) {
  buzz_on_action_ = buzz_on_action;
  buzz_on_press_ = buzz_on_press;
  buzz_duration_ = buzz_duration;
}

//! Store current data in permanent memory (EEPROM)
//! @param eeprom EEPROM writer
void Buzzer::do_write(EepromWrite& eeprom) const {
  eeprom.write(buzz_on_action_);
  eeprom.write(buzz_on_press_);
  eeprom.write(buzz_duration_);
}

//! Validate data from permanent memory (EEPROM).
//! @param eeprom EEPROM reader
bool Buzzer::do_validate(EepromRead &eeprom) {
  bool value;
  uint8_t buzz_duration;
  eeprom.read(value);
  eeprom.read(value);
  eeprom.read(buzz_duration);
  return true;
}

//! Restore data from permanent memory (EEPROM).
//! @param eeprom EEPROM reader
void Buzzer::do_read(EepromRead& eeprom) {
  eeprom.read(buzz_on_action_);
  eeprom.read(buzz_on_press_);
  eeprom.read(buzz_duration_);
}

//! Reset settings
void Buzzer::do_reset() {
  buzz_on_action_ = true;
  buzz_on_press_ = false;
  buzz_duration_ = 1;
}

//! Return the amount of data (in bytes) necessary to save settings in permanent memory (EEPROM).
//! @return Number of bytes
uint16_t Buzzer::do_size_of() const {
  return sizeof(buzz_on_action_) + sizeof(buzz_on_press_) + sizeof(buzz_duration_);
}

//! Send the buzz command to the LCD panel
//! @param duration Duration of the sound
void Buzzer::send_buzz_command_to_lcd()
{
  send_buzz_command_to_lcd(get_buzz_duration());
}

//! Send the buzz command to the LCD panel
//! @param duration Duration of the sound
void Buzzer::send_buzz_command_to_lcd(uint8_t duration)
{
  if(duration <= 0)
    duration = 1;
  WriteRegisterRequest{Register::BuzzerBeepingTime}.write_byte(duration);
}

//! Activate the LCD internal buzzer for the given duration.
//! Note: If the buzzer is disabled, does nothing.
void Buzzer::buzz_on_action()
{
  ui.refresh_screen_timeout();
  if(!is_buzz_on_action_enabled())
    return;

  send_buzz_command_to_lcd();
}

//! Activate the LCD internal buzzer for the given duration.
//! Note: If the buzzer is disabled, does nothing.
void Buzzer::buzz_on_action(uint8_t duration)
{
  ui.refresh_screen_timeout();
  if(!is_buzz_on_action_enabled())
    return;

  send_buzz_command_to_lcd(duration);
}

//! Buzz briefly when the LCD panel is pressed.
//! Note: If buzz on press is disabled, does nothing
void Buzzer::buzz_on_press()
{
    if(!is_buzz_on_press_enabled())
        return;
    send_buzz_command_to_lcd();
}

}