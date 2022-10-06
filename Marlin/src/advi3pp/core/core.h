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

#pragma once

#include "string.h"
#include "task.h"
#include "enums.h"

class GCodeParser;


namespace ADVi3pp {

// ----------------------------------------------------------------------------
// Once - Execute only one time
// ----------------------------------------------------------------------------

struct Once {
    operator bool();
private:
    bool once_ = true;
};

// ----------------------------------------------------------------------------
// Core
// ----------------------------------------------------------------------------

struct Core
{
    enum class PinState: uint8_t { Off = 0, On = 1, Output = 2};

    void startup();
    void idle();
    void killed(float temp, const FlashChar* error, const FlashChar* component);

    bool ensure_not_printing();
    bool is_busy();
    void inject_commands(const FlashChar* commands);
    void inject_commands(const char *commands);

    template<size_t L> ADVString<L>& convert_version(ADVString<L>& version, uint16_t hex_version);

    static PinState get_pin_state(uint8_t pin);
    static float ensure_z_enough_room();

    void media_inserted();
    void media_removed();
    void media_error();

private:
    bool init();
    void send_gplv3_7b_notice();
    void update_progress();
    void from_lcd();
    void to_lcd();
    void send_lcd_data();
    void send_lcd_touch_request();

private:
    Once once_{};
    Action last_action_ = Action::None;
    millis_t last_action_time_ = 0;
};

extern Core core;

//! Convert a version from its hexadecimal representation.
//! @param hex_version  Hexadecimal representation of the version
//! @return             Version as a string
template<size_t L>
ADVString<L>& Core::convert_version(ADVString<L>& version, uint16_t hex_version)
{
    version << hex_version / 0x0100 << '.' << (hex_version % 0x100) / 0x10 << '.' << hex_version % 0x10;
    return version;
}


}