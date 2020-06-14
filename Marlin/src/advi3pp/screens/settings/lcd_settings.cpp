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

#include "../../parameters.h"
#include "lcd_settings.h"
#include "../../core/dimming.h"
#include "../../core/buzzer.h"

namespace ADVi3pp {

LcdSettings lcd_settings;


//! Handle LCD Settings command
//! @param key_value    The sub-action to handle
//! @return             True if the action was handled
bool LcdSettings::do_dispatch(KeyValue key_value)
{
    if(Parent::do_dispatch(key_value))
        return true;

    switch(key_value)
    {
        case KeyValue::LCDDimming:          dimming_command(); break;
        case KeyValue::BuzzerOnAction:      buzz_on_action_command(); break;
        case KeyValue::BuzzOnPress:         buzz_on_press_command(); break;
        default:                            return false;
    }

    return true;
}

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page LcdSettings::do_prepare_page()
{
    settings.send_lcd_values(Variable::Value0);
    return Page::LCD;
}

void LcdSettings::do_back_command()
{
    settings.save();
    Parent::do_back_command();
}

//! Handle the Dimming (On/Off) command
void LcdSettings::dimming_command()
{
    settings.flip_features(Feature::Dimming);
    settings.send_lcd_values(Variable::Value0);
}

//! Handle the change brightness command.
void LcdSettings::change_brightness(uint16_t brightness)
{
    dimming.change_brightness(brightness);
    settings.send_lcd_values(Variable::Value0);
}

//! Handle the Buzz on Action command
void LcdSettings::buzz_on_action_command()
{
    settings.flip_features(Feature::BuzzOnAction);
    settings.send_lcd_values(Variable::Value0);
}

//! Handle the Buzz on Press command
void LcdSettings::buzz_on_press_command()
{
    Feature feature = settings.flip_features(Feature::BuzzOnPress);
    if(feature == Feature::BuzzOnPress)
        buzzer.buzz_on_press();
    settings.send_lcd_values(Variable::Value0);
}

}
