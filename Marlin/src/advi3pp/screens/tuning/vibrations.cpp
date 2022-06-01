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
#include "../../core/core.h"
#include "../../core/dgus.h"
#include "../core/wait.h"
#include "vibrations.h"

namespace ADVi3pp {

Vibrations vibrations;

//! Execute command
//! @param key_value    The sub-action to handle
//! @return             True if the action was handled
bool Vibrations::do_dispatch(KeyValue key_value)
{
    if(Parent::do_dispatch(key_value))
        return true;

    switch(key_value)
    {
        case KeyValue::VibrationsX:     x_command(); break;
        case KeyValue::VibrationsY:     y_command(); break;
        case KeyValue::VibrationsXY:    xy_command(); break;
        case KeyValue::VibrationsYX:    yx_command(); break;
        case KeyValue::VibrationsZ:     z_command(); break;
        default: return false;
    }

    return true;
}

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page Vibrations::do_prepare_page()
{
    if(!core.ensure_not_printing())
        return Page::None;

    return Page::VibrationsTuning;
}

//! Execute the Back command
void Vibrations::do_back_command()
{
    wait.wait(F("Please wait for the move to finish..."));
    background_task.set(Callback{this, &Vibrations::move_finished}, 500);
}

void Vibrations::move_finished() {
    if(core.is_busy()) return;
    background_task.clear();
    pages.show_back_page();
    pages.show_back_page();
}

void Vibrations::move_x() {
    if(core.is_busy()) return;

    int min, max;
    if(!get_min_max(min, max))
        return;

    if(min < X_MIN_BED) min = X_MIN_BED;
    if(max > X_MAX_BED) max = X_MAX_BED;

    int new_position = (ExtUI::getAxisPosition_mm(ExtUI::X) == max) ? min : max;

    ADVString<20> cmd;
    cmd.format(F("G1 X%i F6000"), new_position);
    core.inject_commands(cmd.get());
}

void Vibrations::move_y() {
    if(core.is_busy()) return;

    int min, max;
    if(!get_min_max(min, max))
        return;

    if(min < Y_MIN_BED) min = Y_MIN_BED;
    if(max > Y_MAX_BED) max = Y_MAX_BED;

    int new_position = (ExtUI::getAxisPosition_mm(ExtUI::Y) == max) ? min : max;

    ADVString<20> cmd;
    cmd.format(F("G1 Y%i F6000"), new_position);
    core.inject_commands(cmd.get());
}

void Vibrations::move_z() {
    if(core.is_busy()) return;

    int min, max;
    if(!get_min_max(min, max))
        return;

    if(min < Z_MIN_POS) min = Z_MIN_POS;
    if(max > Z_MAX_POS) max = Z_MAX_POS;

    int new_position = (ExtUI::getAxisPosition_mm(ExtUI::Z) == max) ? min : max;

    ADVString<20> cmd;
    cmd.format(F("G1 Z%i F1200"), new_position);
    core.inject_commands(cmd.get());
}

void Vibrations::move_start_xy() {
    ADVString<20> cmd;
    cmd.format(F("G1 X%i F6000\nG1 Y%i F6000"), X_MIN_BED, Y_MIN_BED);
    core.inject_commands(cmd.get());
    background_task.set(Callback{this, &Vibrations::move_xy}, 500);
}

void Vibrations::move_start_yx() {
    ADVString<20> cmd;
    cmd.format(F("G1 X%i F6000\nG1 Y%i F6000"), X_MIN_BED, Y_MAX_BED);
    core.inject_commands(cmd.get());
    background_task.set(Callback{this, &Vibrations::move_xy}, 500);
}

void Vibrations::move_xy() {
    if(core.is_busy()) return;

    int min, max;
    if(!get_min_max(min, max))
        return;

    int min_x = (min < X_MIN_BED) ? X_MIN_BED : min;
    int max_x = (max > X_MAX_BED) ? X_MAX_BED : max;
    int min_y = (min < Y_MIN_BED) ? Y_MIN_BED : min;
    int max_y = (max > Y_MAX_BED) ? Y_MAX_BED : max;

    int new_x_position = (ExtUI::getAxisPosition_mm(ExtUI::X) == max_x) ? min_x : max_x;
    int new_y_position = (ExtUI::getAxisPosition_mm(ExtUI::Y) == max_y) ? min_y : max_y;

    ADVString<20> cmd;
    cmd.format(F("G1 X%i Y%i F6000"), new_x_position, new_y_position);
    core.inject_commands(cmd.get());
}

bool Vibrations::get_min_max(int &min, int &max) {
    ReadRam frame{Variable::Value0};
    if(!frame.send_receive(2))
    {
        Log::error() << F("Receiving Frame (Min Max)") << Log::endl();
        return false;
    }

    min = frame.read_word();
    max = frame.read_word();
    if(min < 0) min = 0;
    if(min >= max) return false;

    return true;
}

void Vibrations::x_command() {
    background_task.set(Callback{this, &Vibrations::move_x}, 500);
}

void Vibrations::y_command() {
    background_task.set(Callback{this, &Vibrations::move_y}, 500);
}

void Vibrations::xy_command() {
    background_task.set(Callback{this, &Vibrations::move_start_xy}, 500);
}

void Vibrations::yx_command() {
    background_task.set(Callback{this, &Vibrations::move_start_yx}, 500);
}

void Vibrations::z_command() {
    background_task.set(Callback{this, &Vibrations::move_z}, 500);
}

}
