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

#include "../../inc/MarlinConfig.h"
#include "dgus.h"
#include "buzzer.h"


namespace ADVi3pp {

Buzzer buzzer;

//! Send the buzz command to the LCD panel
//! @param duration Duration of the sound
void Buzzer::send_buzz_command_to_lcd() {
  send_buzz_command_to_lcd(ui.tone_duration);
}

//! Send the buzz command to the LCD panel
//! @param duration Duration of the sound. 0 for default duration.
void Buzzer::send_buzz_command_to_lcd(uint8_t duration) {
  if(duration <= 0) duration = ui.tone_duration;
  if(duration < 10) duration = 10;
  WriteRegisterRequest{Register::BuzzerBeepingTime}.write_byte(duration / 10);
}

//! Activate the LCD internal buzzer for the given duration.
//! Note: If the buzzer is disabled, does nothing.
void Buzzer::buzz_on_action() {
  ui.refresh_screen_timeout();
  if(!is_option_enabled(OPTIONS::ON_ACTION)) return;
  send_buzz_command_to_lcd();
}

//! Activate the LCD internal buzzer for the given duration.
//! Note: If the buzzer is disabled, does nothing.
void Buzzer::buzz_on_action(uint8_t duration) {
  ui.refresh_screen_timeout();
  if(!is_option_enabled(OPTIONS::ON_ACTION)) return;
  send_buzz_command_to_lcd(duration);
}

//! Activate the LCD internal buzzer for the given duration.
//! Note: Even if the buzzer is disabled, it does beep.
void Buzzer::buzz(uint8_t duration) {
  ui.refresh_screen_timeout();
  send_buzz_command_to_lcd(duration);
}

//! Buzz briefly when the LCD panel is pressed.
//! Note: If buzz on press is disabled, does nothing
void Buzzer::buzz_on_press() {
  if(!is_option_enabled(OPTIONS::ON_TOUCH)) return;
  send_buzz_command_to_lcd();
}

}