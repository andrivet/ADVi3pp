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

//! Manual Leveling Page
struct ManualLeveling: Screen<ManualLeveling>
{
private:
    bool do_dispatch(KeyValue value);
    Page do_prepare_page();
    void do_back_command();
    void point1_command();
    void point2_command();
    void point3_command();
    void point4_command();
    void point5_command();
    void pointA_command();
    void pointB_command();
    void pointC_command();
    void pointD_command();
    void leveling_task();
    void move(int x, int y);

    friend Parent;
};
extern ManualLeveling manual_leveling;

}
