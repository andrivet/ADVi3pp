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

#include "logging.h"
#include "dgus.h"
#include "buzzer.h"
#include "dimming.h"

namespace ADVi3pp {

const uint8_t BUZZ_ON_PRESS_DURATION = 10; // x 1 ms

Buzzer buzzer;

//! Enable or disable the buzzer
void Buzzer::enable(bool enable)
{
    enabled_ = enable;
}

//! Enable or disable the buzzer when the LCD panel is presses
void Buzzer::enable_on_press(bool enable, bool buzz)
{
    on_press_enabled_ = buzz;
    if(enable && buzz)
        send_buzz_command_to_lcd(50);
}

//! Activate the LCD internal buzzer for the given duration.
//! Note: If the buzzer is disabled, does nothing.
void Buzzer::buzz(long duration)
{
    dimming.reset();
    if(!enabled_)
    {
        Log::log() << F("Silent Buzz") << Log::endl();
        return;
    }

    send_buzz_command_to_lcd(duration);
}

//! Send the buzz command to the LCD panel
//! @param duration Duration of the sound
void Buzzer::send_buzz_command_to_lcd(long duration)
{
    duration /= 10;

    WriteRegisterDataRequest request{Register::BuzzerBeepingTime};
    request << Uint8(static_cast<uint8_t>(duration > UINT8_MAX ? UINT8_MAX : duration));
    request.send();
}

//! Buzz briefly when the LCD panel is pressed.
//! Note: If buzz on press is disabled, does nothing
void Buzzer::buzz_on_press()
{
    if(!on_press_enabled_)
    {
        Log::log() << F("Silent Buzz") << Log::endl();
        return;
    }
    send_buzz_command_to_lcd(BUZZ_ON_PRESS_DURATION);
}

}