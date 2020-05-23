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

//! Move Page
struct Move: Screen<Move>
{
    void x_plus_command();
    void x_minus_command();
    void x_home_command();
    void y_plus_command();
    void y_minus_command();
    void y_home_command();
    void z_plus_command();
    void z_minus_command();
    void z_home_command();
    void e_plus_command();
    void e_minus_command();
    void all_home_command();
    void disable_motors_command();

private:
    bool do_dispatch(KeyValue key_value);
    Page do_prepare_page();
    void move(const char* command, millis_t delay);

private:
    millis_t last_move_time_ = 0;

    friend Parent;
};

}
