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
#include "../../core/core.h"
#include "../core/wait.h"
#include "xtwist.h"
#include "../../../module/probe.h"

namespace ADVi3pp {

XTwist x_twist;

#ifdef ADVi3PP_PROBE

const double SENSOR_Z_HEIGHT_MULTIPLIERS[] = {0.01, 0.10, 1.0};


//! Store current data in permanent memory (EEPROM)
//! @param eeprom EEPROM writer
void XTwist::do_write(EepromWrite& eeprom) const
{
    for(auto offset : offsets_)
        eeprom.write(offset);
}

//! Validate data from permanent memory (EEPROM).
//! @param eeprom EEPROM reader
bool XTwist::do_validate(EepromRead &eeprom)
{
    int dummy{};
    for(const auto& offset : offsets_)
        UNUSED(offset), eeprom.read(dummy);
    return true;
}

//! Restore data from permanent memory (EEPROM).
//! @param eeprom EEPROM reader
void XTwist::do_read(EepromRead& eeprom)
{
    for(auto& offset : offsets_)
        eeprom.read(offset);
}

//! Reset settings
void XTwist::do_reset()
{
    for(auto& offset : offsets_)
        offset = 0;
}

//! Return the amount of data (in bytes) necessary to save settings in permanent memory (EEPROM).
//! @return Number of bytes
uint16_t XTwist::do_size_of() const
{
    return sizeof(offsets_);
}

//! Handle Sensor Z Height command
//! @param key_value    The sub-action to handle
//! @return             True if the action was handled
bool XTwist::do_dispatch(KeyValue key_value)
{
    if(Parent::do_dispatch(key_value))
        return true;

    switch(key_value)
    {
        case KeyValue::Multiplier1:     multiplier1_command(); break;
        case KeyValue::Multiplier2:     multiplier2_command(); break;
        case KeyValue::Multiplier3:     multiplier3_command(); break;
        case KeyValue::Point_L:         point_L_command(); break;
        case KeyValue::Point_M:         point_M_command(); break;
        case KeyValue::Point_R:         point_R_command(); break;
        default:                        return false;
    }

    return true;
}

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page XTwist::do_prepare_page()
{
    if(!core.ensure_not_printing())
        return Page::None;
    pages.save_forward_page();

    wait.show(F("Homing..."));
    core.inject_commands(F("G28 F6000"));  // homing
    task.set_background_task(BackgroundTask(this, &XTwist::post_home_task), 200);
    return Page::None;
}

//! Check if the printer is homed, and continue the Z Height Tuning process.
void XTwist::post_home_task()
{
    if(!ExtUI::isMachineHomed() || core.is_busy())
        return;

    task.clear_background_task();
    send_data();
    status.reset();

    pages.show_page(Page::XTwist, ShowOptions::None);

    ExtUI::setSoftEndstopState(false);
    reset_mesh();
    point_M_command();
}

//! Execute the Back command
void XTwist::do_back_command()
{
    update_mesh(); // Put back the mesh like it was before calling X Twist

    // enable enstops, raise head
    ExtUI::setSoftEndstopState(true);
    ExtUI::setFeedrate_mm_s(HOMING_FEEDRATE_Z);
    ExtUI::setAxisPosition_mm(4, ExtUI::Z);

    Parent::do_back_command();
}

void XTwist::update_mesh()
{
    Log::log() << F("Update mesh") << Log::endl();

    xy_uint8_t xy{};
    for(uint8_t y = 0; y < GRID_MAX_POINTS_Y; ++y)
    {
        auto log = Log::log();
        for(uint8_t x = 0; x < GRID_MAX_POINTS_X; ++x)
        {
            xy.set(x, y);
            auto z = ExtUI::getMeshPoint(xy) + offsets_[x] / 100.0;
            log << z << F(" ");
            ExtUI::setMeshPoint(xy, z);
        }
        log << Log::endl();
    }
}

void XTwist::reset_mesh()
{
    Log::log() << F("Reset mesh") << Log::endl();

    xy_uint8_t xy{};
    for(uint8_t y = 0; y < GRID_MAX_POINTS_Y; ++y)
    {
        auto log = Log::log();
        for(uint8_t x = 0; x < GRID_MAX_POINTS_X; ++x)
        {
            xy.set(x, y);
            auto z = ExtUI::getMeshPoint(xy) - offsets_[x] / 100.0;
            log << z << F(" ");
            ExtUI::setMeshPoint(xy, z);
        }
        log << Log::endl();
    }
}

//! Handles the Save (Continue) command
void XTwist::do_save_command()
{
    update_mesh();

    // enable enstops, raise head
    ExtUI::setSoftEndstopState(true);
    ExtUI::setFeedrate_mm_s(HOMING_FEEDRATE_Z);
    ExtUI::setAxisPosition_mm(4, ExtUI::Z);

    Parent::do_save_command();
}

void XTwist::on_mesh_updated(const int8_t xpos, const int8_t ypos, const float zval)
{
    xy_uint8_t xy{};
    xy.set(static_cast<uint8_t>(xpos), static_cast<uint8_t>(ypos));
    auto z = zval + offsets_[xy.x] / 100.0;
    Log::log() << F("Update mesh @")
               << static_cast<uint8_t>(xpos) << F(", ")
               << static_cast<uint8_t>(ypos) << F(" = ")
               << z << Log::endl();
    ExtUI::setMeshPoint(xy, z);
}

//! Change the multiplier.
void XTwist::multiplier1_command()
{
    multiplier_ = Multiplier::M1;
    send_data();
}

//! Change the multiplier.
void XTwist::multiplier2_command()
{
    multiplier_ = Multiplier::M2;
    send_data();
}

//! Change the multiplier.
void XTwist::multiplier3_command()
{
    multiplier_ = Multiplier::M3;
    send_data();
}

void XTwist::move_x(Point x)
{
    int x_mm = probe.min_x() + static_cast<int>(x) * (probe.max_x() - probe.min_x()) / (GRID_MAX_POINTS_X - 1);
    int y_mm = probe.min_y() + (probe.max_y() - probe.min_y()) / (GRID_MAX_POINTS_Y - 1);

    ExtUI::setFeedrate_mm_s(HOMING_FEEDRATE_Z);
    ExtUI::setAxisPosition_mm(4, ExtUI::Z);

    ExtUI::setFeedrate_mm_s(HOMING_FEEDRATE_XY);
    ExtUI::setAxisPosition_mm(x_mm, ExtUI::X);

    ExtUI::setFeedrate_mm_s(HOMING_FEEDRATE_XY);
    ExtUI::setAxisPosition_mm(y_mm, ExtUI::Y);

    ExtUI::setFeedrate_mm_s(HOMING_FEEDRATE_Z);
    ExtUI::setAxisPosition_mm(offset(x) / 100.0, ExtUI::Z);

    point_ = x;
}

void XTwist::point_M_command()
{
    move_x(Point::M);
}

void XTwist::point_L_command()
{
    move_x(Point::L);
}

void XTwist::point_R_command()
{
    move_x(Point::R);
}

//! Change the position of the nozzle (-Z).
void XTwist::minus()
{
    adjust_height(-get_multiplier_value());
}

//! Change the position of the nozzle (+Z).
void XTwist::plus()
{
    adjust_height(+get_multiplier_value());
}

//! Get the current multiplier value on the LCD panel.
double XTwist::get_multiplier_value() const
{
    if(multiplier_ < Multiplier::M1 || multiplier_ > Multiplier::M3)
    {
        Log::error() << F("Invalid multiplier value: ") << static_cast<uint16_t >(multiplier_) << Log::endl();
        return SENSOR_Z_HEIGHT_MULTIPLIERS[0];
    }

    return SENSOR_Z_HEIGHT_MULTIPLIERS[static_cast<uint16_t>(multiplier_)];
}

//! Adjust the Z height.
//! @param offset Offset for the adjustment.
void XTwist::adjust_height(double offset_value)
{
    ExtUI::setFeedrate_mm_s(HOMING_FEEDRATE_Z);
    ExtUI::setAxisPosition_mm(ExtUI::getAxisPosition_mm(ExtUI::Z) + offset_value, ExtUI::Z);
    offset(point_) = lround(ExtUI::getAxisPosition_mm(ExtUI::Z) * 100);
}

//! Send the current data (i.e. multiplier) to the LCD panel.
void XTwist::send_data() const
{
    WriteRamDataRequest frame{Variable::Value0};
    frame << Uint16(static_cast<uint16_t>(multiplier_));
    frame.send();
}

#else

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page XTwist::do_prepare_page()
{
    return Page::NoSensor;
}

#endif

}
