/**
 * ADVi3++ Firmware For Wanhao Duplicator i3 Plus (based on Marlin 2)
 *
 * Copyright (C) 2017-2021 Sebastien Andrivet [https://github.com/andrivet/]
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
    set_next_dimming_time();
}

//! Set the next dimming check time
void Dimming::set_next_checking_time()
{
    next_check_time_ = millis() + 200;
}

//! Set the next dimming delay time
void Dimming::set_next_dimming_time()
{
    next_dimming_time_ = millis() + 1000ul * dimming_delay;
}

//! Get adjusted brightness (depended of the active dimming or not)
//! @return The adjusted brightness
uint8_t Dimming::get_adjusted_brightness()
{
    int16_t brightness = ExtUI::get_lcd_contrast();
    if(dimmed_)
        brightness = brightness * dimming_ratio / 100;
    if(brightness < LCD_CONTRAST_MIN)
        brightness = LCD_CONTRAST_MIN;
    if(brightness > LCD_CONTRAST_MAX)
        brightness = LCD_CONTRAST_MAX;
    return static_cast<uint8_t>(brightness);
}


void Dimming::send()
{
    if(!settings.is_feature_enabled(Feature::Dimming) || !ELAPSED(millis(), next_check_time_))
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

        if(response.read_byte() == 0x5A) {
            no_log.allow();
            // Reset TouchPanelFlag
            WriteRegisterRequest{Register::TouchPanelFlag}.write_byte(0);

            Log::log() << F("Panel touched, reset dimming") << Log::endl();
            reset();
            return true;
        }
    }

    if(!dimmed_ && settings.is_feature_enabled(Feature::Dimming) && ELAPSED(millis(), next_dimming_time_))
    {
        no_log.allow();
        Log::log() << F("Delay elapsed, dim the panel") << Log::endl();
        dimmed_ = true;
        send_brightness();
    }

    return received;
}

//! Reset the dimming
//! @param force Change the brightness right now
void Dimming::reset(bool force)
{
    set_next_dimming_time();
    if(!force && !dimmed_) // Already reset, nothing more to do (unless force is true)
        return;

    dimmed_ = false;
    send_brightness();
}

//! Adjust the brigthness of the LCD panel
void Dimming::send_brightness()
{
    auto brightness = get_adjusted_brightness();

    WriteRegisterRequest{Register::Brightness}.write_byte(brightness);
}

//! Change the brightness of the LCD Panel.
//! @param brightness New brightness
void Dimming::change_brightness(int16_t brightness)
{
    ExtUI::set_lcd_contrast(brightness);
}

}

