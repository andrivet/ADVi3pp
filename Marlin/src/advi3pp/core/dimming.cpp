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

//! Get adjusted brightness (depended of the active dimming or not)
//! @return The adjusted brightness
uint8_t Dimming::get_adjusted_brightness()
{
    int16_t brightness = ui.brightness;
    if(dimmed_)
        brightness = brightness * dimming_ratio / 100;
    if(brightness < LCD_BRIGHTNESS_MIN)
        brightness = LCD_BRIGHTNESS_MIN;
    if(brightness > LCD_BRIGHTNESS_MAX)
        brightness = LCD_BRIGHTNESS_MAX;
    return static_cast<uint8_t>(brightness);
}


void Dimming::send()
{
    if(!settings.is_feature_enabled(Feature::Dimming) || !dimmed_ || !ELAPSED(millis(), next_check_time_))
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
      // Reset TouchPanelFlag
      WriteRegisterRequest{Register::TouchPanelFlag}.write_byte(0);
      ui.refresh_screen_timeout();
      return true;
    }
  }

  if(!dimmed_ && settings.is_feature_enabled(Feature::Dimming))
    ui.check_screen_timeout();

  return received;
}

//! Set the brightness of the LCD panel
void Dimming::send_brightness_to_lcd()
{
    WriteRegisterRequest{Register::Brightness}.write_byte(get_adjusted_brightness());
}

void Dimming::sleep_on() {
  if(dimmed_) return;
  dimmed_ = true;
  send_brightness_to_lcd();
}

void Dimming::sleep_off() {
  if(!dimmed_) return;
  dimmed_ = false;
  send_brightness_to_lcd();
}

}
