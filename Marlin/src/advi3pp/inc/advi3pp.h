/**
 * ADVi3++ Firmware For Wanhao Duplicator i3 Plus (based on Marlin)
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

#include "../../lcd/extui/ui_api.h"

namespace ADVi3pp {

#if ENABLED(ADVi3PP_X_TWIST)

#if ENABLED(ADVi3PP_PROBE)
    void twist(xyze_pos_t &pos);
    void untwist(xyze_pos_t &pos);
    float twist_offset_x0();
    float twist_offset_x2();
#else
    inline void twist(xyze_pos_t &pos) {}
    inline void untwist(xyze_pos_t &pos) {}
    inline float twist_offset_x0() { return 0; }
    inline float twist_offset_x2() { return 0; }
#endif

#endif

}
