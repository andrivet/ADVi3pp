/**
 * ADVi3++ Firmware For Wanhao Duplicator i3 Plus (based on Marlin 2)
 *
 * Copyright (C) 2017-2020 Sebastien Andrivet [https://github.com/andrivet/]
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

#include <Arduino.h>
#include "../../lcd/extui/ui_api.h"
#include "../../lcd/ultralcd.h"
#include "dimming.h"
#include "dgus.h"
#include "logging.h"

namespace ADVi3pp {

Dimming dimming;

//! Constructor. Initialize dimming check time and dimming delay time
Dimming::Dimming()
{
    set_next_checking_time();
    set_next_dimmming_time();
}

//! Enable or disable dimming of the LCD Panel
//! @param enable Enable or disable dimming
//! @param doIt Chnage the dimming right now or not
void Dimming::enable(bool enable, bool doIt)
{
    enabled_ = enable;
    if(doIt)
        reset(true);
}

//! Set the next dimming check time
void Dimming::set_next_checking_time()
{
    next_check_time_ = millis() + 200;
}

//! Set the next dimming delay time
void Dimming::set_next_dimmming_time()
{
    next_dimming_time_ = millis() + 1000ul * dimming_delay;
}

//! Get adjusted brightness (depended of the active dimming or not)
//! @return The adjusted brightness
uint8_t Dimming::get_adjusted_brightness()
{
    int16_t brightness = ui.contrast;
    if(dimming_)
        brightness = brightness * dimming_ratio / 100;
    if(brightness < LCD_CONTRAST_MIN)
        brightness = LCD_CONTRAST_MIN;
    if(brightness > LCD_CONTRAST_MAX)
        brightness = LCD_CONTRAST_MAX;
    return static_cast<uint8_t>(brightness);
}

//! Check the dimming state: LCD Panel was touched? Dimming delay was elapsed?
void Dimming::check()
{
    if(!enabled_ || !ELAPSED(millis(), next_check_time_))
        return;
    set_next_checking_time();

    ReadRegister read{Register::TouchPanelFlag, 1};
    if(!read.send_and_receive(false))
    {
        Log::error() << F("Reading TouchPanelFlag") << Log::endl();
        return;
    }
    Uint8 value; read >> value;

    // Reset TouchPanelFlag
    WriteRegisterDataRequest request{Register::TouchPanelFlag};
    request << 00_u8;
    request.send(false);

    if(value.byte == 0x5A)
    {
        Log::log() << F("Panel touched, reset dimming") << Log::endl();
        reset();
        return;
    }

    if(!dimming_ && ELAPSED(millis(), next_dimming_time_))
    {
        Log::log() << F("Delay elapsed, dim the panel") << Log::endl();
        dimming_ = true;
        send_brightness();
    }
}

//! Reset the dimming
//! @param force Change the brightness right now
void Dimming::reset(bool force)
{
    set_next_dimmming_time();
    if(!force && !dimming_) // Already reset, nothing more to do (unless force is true)
        return;

    dimming_ = false;
    send_brightness();
}

//! Adjust the brigthness of the LCD panel
void Dimming::send_brightness()
{
    auto brightness = get_adjusted_brightness();

    WriteRegisterDataRequest frame{Register::Brightness};
    frame << Uint8{brightness};
    frame.send(true);
}

//! Change the brightness of the LCD Panel.
//! @param brightness New brightness
void Dimming::change_brightness(int16_t brightness)
{
    ui.set_contrast(brightness);;
}

}

int16_t MarlinUI::contrast = DEFAULT_LCD_CONTRAST;

void MarlinUI::set_contrast(const int16_t value) {
    MarlinUI::contrast = constrain(value, LCD_CONTRAST_MIN, LCD_CONTRAST_MAX);
    ADVi3pp::dimming.reset();
}

