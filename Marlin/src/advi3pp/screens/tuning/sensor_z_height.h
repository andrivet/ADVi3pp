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

#include "../core/screen.h"

namespace ADVi3pp {

#ifdef ADVi3PP_PROBE
//! Sensor Z Height Tuning Page
struct SensorZHeight: Screen<SensorZHeight>
{
    void minus();
    void plus();

    enum class Multiplier: uint8_t
    {
        M1 = 0,
        M2 = 1,
        M3 = 2
    };

private:
    bool do_dispatch(KeyValue key_value);
    Page do_prepare_page();
    void do_save_command();
    void do_back_command();
    void post_home_task();
    void multiplier1_command();
    void multiplier2_command();
    void multiplier3_command();
    double get_multiplier_value() const;
    void adjust_height(double offset);
    void send_data() const;
    void reset();

private:
    Multiplier multiplier_ = Multiplier::M1;
    friend Parent;
};
#else
//! Sensor Z Height Tuning Page
struct SensorZHeight: Screen<SensorZHeight>
{
    void minus() {}
    void plus() {}

private:
    Page do_prepare_page();
    friend Parent;
};
#endif

extern SensorZHeight sensor_z_height;

}
