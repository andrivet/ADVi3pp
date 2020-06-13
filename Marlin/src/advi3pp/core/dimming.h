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

#pragma once

#include <stdint.h>
#include "../../core/millis_t.h"

namespace ADVi3pp {

//! LCD screen brightness and dimming
struct Dimming
{
    Dimming();

    void check();
    void reset(bool force = false);
    void change_brightness(int16_t brightness);

private:
    void set_next_checking_time();
    void set_next_dimming_time();
    void send_brightness();
    uint8_t get_adjusted_brightness();

private:
    static constexpr uint8_t dimming_ratio = 5; //!< Ratio (in percent) between normal and dimmed LCD panel
    static constexpr uint16_t dimming_delay = 5 * 60; //!< Delay before dimming the LCD panel (5 minutes)

    bool dimmed_ = false;
    millis_t next_check_time_ = 0;
    millis_t next_dimming_time_ = 0;
};

extern Dimming dimming;

}
