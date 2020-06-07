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
#include "manual_leveling.h"
#include "../../core/core.h"
#include "../core/wait.h"

namespace ADVi3pp {

ManualLeveling manual_leveling;

//! Execute a Manual Leveling command
//! @param key_value    The sub-action to handle
//! @return             True if the action was handled
bool ManualLeveling::do_dispatch(KeyValue key_value)
{
    if(Parent::do_dispatch(key_value))
        return true;

    switch(key_value)
    {
        case KeyValue::LevelingPoint1:  point1_command(); break;
        case KeyValue::LevelingPoint2:  point2_command(); break;
        case KeyValue::LevelingPoint3:  point3_command(); break;
        case KeyValue::LevelingPoint4:  point4_command(); break;
        case KeyValue::LevelingPoint5:  point5_command(); break;
        case KeyValue::LevelingPointA:  pointA_command(); break;
        case KeyValue::LevelingPointB:  pointB_command(); break;
        case KeyValue::LevelingPointC:  pointC_command(); break;
        case KeyValue::LevelingPointD:  pointD_command(); break;
        default:                        return false;
    }

    return true;
}

//! Execute the Back command
void ManualLeveling::do_back_command()
{
    ExtUI::injectCommands_P(PSTR("G1 Z30 F1200"));
    Parent::do_back_command();
}

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page ManualLeveling::do_prepare_page()
{
    if(!core.ensure_not_printing())
        return Page::None;
    wait.show(F("Homing..."));
    ExtUI::setAllAxisUnhomed();
    ExtUI::setAllAxisPositionUnknown();
    ExtUI::injectCommands_P(PSTR("G90")); // absolute mode
    ExtUI::injectCommands_P((PSTR("G28 F6000"))); // homing
    task.set_background_task(BackgroundTask(this, &ManualLeveling::leveling_task), 200);
    return Page::None;
}

//! Leveling Background task.
void ManualLeveling::leveling_task()
{
    if(!ExtUI::isMachineHomed())
        return;

    Log::log() << F("Leveling Homed, start process") << Log::endl();
    task.clear_background_task();
    pages.show_page(Page::ManualLeveling, ShowOptions::None);
}

//! Handle leveling point #1.
void ManualLeveling::point1_command()
{
    Log::log() << F("Level point 1") << Log::endl();
    ExtUI::injectCommands_P(PSTR("G1 Z4 F1200"));
    ExtUI::injectCommands_P(PSTR("G1 X30 Y30 F6000"));
    ExtUI::injectCommands_P(PSTR("G1 Z0 F1200"));
}

//! Handle leveling point #2.
void ManualLeveling::point2_command()
{
    Log::log() << F("Level point 2") << Log::endl();
    ExtUI::injectCommands_P(PSTR("G1 Z4 F1200"));
    ExtUI::injectCommands_P(PSTR("G1 X30 Y170 F6000"));
    ExtUI::injectCommands_P(PSTR("G1 Z0 F1200"));
}

//! Handle leveling point #3.
void ManualLeveling::point3_command()
{
    Log::log() << F("Level point 3") << Log::endl();
    ExtUI::injectCommands_P(PSTR("G1 Z4 F1200"));
    ExtUI::injectCommands_P(PSTR("G1 X170 Y170 F6000"));
    ExtUI::injectCommands_P(PSTR("G1 Z0 F1200"));
}

//! Handle leveling point #4.
void ManualLeveling::point4_command()
{
    Log::log() << F("Level point 4") << Log::endl();
    ExtUI::injectCommands_P(PSTR("G1 Z4 F1200"));
    ExtUI::injectCommands_P(PSTR("G1 X170 Y30 F6000"));
    ExtUI::injectCommands_P(PSTR("G1 Z0 F1200"));
}

//! Handle leveling point #5.
void ManualLeveling::point5_command()
{
    Log::log() << F("Level point 5") << Log::endl();
    ExtUI::injectCommands_P(PSTR("G1 Z4 F1200"));
    ExtUI::injectCommands_P(PSTR("G1 X100 Y100 F6000"));
    ExtUI::injectCommands_P(PSTR("G1 Z0 F1200"));
}

//! Handle leveling point #A.
void ManualLeveling::pointA_command()
{
    Log::log() << F("Level point A") << Log::endl();
    ExtUI::injectCommands_P(PSTR("G1 Z4 F1200"));
    ExtUI::injectCommands_P(PSTR("G1 X100 Y30 F6000"));
    ExtUI::injectCommands_P(PSTR("G1 Z0 F1200"));
}

//! Handle leveling point #B.
void ManualLeveling::pointB_command()
{
    Log::log() << F("Level point B") << Log::endl();
    ExtUI::injectCommands_P(PSTR("G1 Z4 F1200"));
    ExtUI::injectCommands_P(PSTR("G1 X30 Y100 F6000"));
    ExtUI::injectCommands_P(PSTR("G1 Z0 F1200"));
}

//! Handle leveling point #C.
void ManualLeveling::pointC_command()
{
    Log::log() << F("Level point C") << Log::endl();
    ExtUI::injectCommands_P(PSTR("G1 Z4 F1200"));
    ExtUI::injectCommands_P(PSTR("G1 X100 Y170 F6000"));
    ExtUI::injectCommands_P(PSTR("G1 Z0 F1200"));
}

//! Handle leveling point #D.
void ManualLeveling::pointD_command()
{
    Log::log() << F("Level point D") << Log::endl();
    ExtUI::injectCommands_P(PSTR("G1 Z4 F1200"));
    ExtUI::injectCommands_P(PSTR("G1 X170 Y100 F6000"));
    ExtUI::injectCommands_P(PSTR("G1 Z0 F1200"));
}

}
