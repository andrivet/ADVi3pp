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

//! Automatic Leveling Page
struct AutomaticLeveling: Screen<AutomaticLeveling>
{
    void leveling_finished(bool success);

private:
    bool do_dispatch(KeyValue key_value);
    Page do_prepare_page();
    void reset_command();
    void start();
    bool leveling_failed();

private:
    bool sensor_interactive_leveling_ = false;

    friend Parent;
};

extern AutomaticLeveling automatic_leveling;

}
