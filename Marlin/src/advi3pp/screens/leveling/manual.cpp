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
#include "manual.h"
#include "xtwist.h"
#include "../../core/core.h"
#include "../../screens/core/wait.h"

namespace ADVi3pp {

namespace {

    constexpr xyz_feedrate_t homing_feedrate_mm_m = HOMING_FEEDRATE_MM_M;
    constexpr float FEEDRATE_X = MMM_TO_MMS(homing_feedrate_mm_m.x);
    constexpr float FEEDRATE_Y = MMM_TO_MMS(homing_feedrate_mm_m.y);
    constexpr float FEEDRATE_Z = MMM_TO_MMS(homing_feedrate_mm_m.z);

}

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
#if HAS_LEVELING
    ExtUI::setLevelingActive(true); // Enable back compensation
#endif
    ExtUI::setFeedrate_mm_s(FEEDRATE_Z);
    ExtUI::setAxisPosition_mm(Z_AFTER_HOMING, ExtUI::Z);
    Parent::do_back_command();
}

void ManualLeveling::do_save_command()
{
#if HAS_LEVELING
    xtwist.reset();
    ExtUI::setLevelingActive(false); // Disable ABL mesh (already disabled but prefer to be explicit)
#endif
    ExtUI::setFeedrate_mm_s(FEEDRATE_Z);
    ExtUI::setAxisPosition_mm(Z_AFTER_HOMING, ExtUI::Z);
    Parent::do_save_command();
}


//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page ManualLeveling::do_prepare_page()
{
    if(!core.ensure_not_printing())
        return Page::None;
    pages.save_forward_page();
    wait.wait(F("Homing..."));
    ExtUI::setAllAxisUnhomed();
    ExtUI::setAllAxisPositionUnknown();
    core.inject_commands(F("G28 F6000")); // Homing
#if HAS_LEVELING
    ExtUI::setLevelingActive(false); // We do not want compensation during manual leveling
#endif
    background_task.set(Callback{this, &ManualLeveling::leveling_task}, 200);
    return Page::None;
}

//! Leveling Background task.
void ManualLeveling::leveling_task()
{
    if(!ExtUI::isMachineHomed())
        return;

    Log::log() << F("Leveling Homed, start process") << Log::endl();
    background_task.clear();
    pages.show(Page::ManualLeveling);
}

void ManualLeveling::move(int x, int y)
{
    ExtUI::setFeedrate_mm_s(FEEDRATE_Z);
    ExtUI::setAxisPosition_mm(Z_HOMING_HEIGHT, ExtUI::Z);

    ExtUI::setFeedrate_mm_s(FEEDRATE_X);
    ExtUI::setAxisPosition_mm(x, ExtUI::X);
    ExtUI::setFeedrate_mm_s(FEEDRATE_Y);
    ExtUI::setAxisPosition_mm(y, ExtUI::Y);

    ExtUI::setFeedrate_mm_s(FEEDRATE_Z);
    ExtUI::setAxisPosition_mm(0, ExtUI::Z);
}

//! Handle leveling point #1.
void ManualLeveling::point1_command()
{
    Log::log() << F("Level point 1") << Log::endl();
    move(30, 30);
}

//! Handle leveling point #2.
void ManualLeveling::point2_command()
{
    Log::log() << F("Level point 2") << Log::endl();
    move(30, 170);
}

//! Handle leveling point #3.
void ManualLeveling::point3_command()
{
    Log::log() << F("Level point 3") << Log::endl();
    move(170, 170);
}

//! Handle leveling point #4.
void ManualLeveling::point4_command()
{
    Log::log() << F("Level point 4") << Log::endl();
    move(170, 30);
}

//! Handle leveling point #5.
void ManualLeveling::point5_command()
{
    Log::log() << F("Level point 5") << Log::endl();
    move(100, 100);
}

//! Handle leveling point #A.
void ManualLeveling::pointA_command()
{
    Log::log() << F("Level point A") << Log::endl();
    move(100, 30);
}

//! Handle leveling point #B.
void ManualLeveling::pointB_command()
{
    Log::log() << F("Level point B") << Log::endl();
    move(30, 100);
}

//! Handle leveling point #C.
void ManualLeveling::pointC_command()
{
    Log::log() << F("Level point C") << Log::endl();
    move(100, 170);
}

//! Handle leveling point #D.
void ManualLeveling::pointD_command()
{
    Log::log() << F("Level point D") << Log::endl();
    move(170, 100);
}

}
