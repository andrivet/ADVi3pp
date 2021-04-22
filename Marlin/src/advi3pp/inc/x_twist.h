/**
 * ADVi3++ Firmware For Wanhao Duplicator i3 Plus (based on Marlin 2)
 *
 * Copyright (C) 2017-2021 Sebastien Andrivet [https://github.com/andrivet/]
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

#if ENABLED(ADVi3PP_X_TWIST)

#include "src/lcd/extui/ui_api.h"

namespace ADVi3pp
{
    struct x_twist_factors_t
    {
        float a_ = 200.0f;
        float b_ = 0.0f;
    };

    extern x_twist_factors_t x_twist_factors;

    inline void x_twist(xyze_pos_t &pos)   { pos.z += x_twist_factors.b_ * pos.x / x_twist_factors.a_; }
    inline void x_untwist(xyze_pos_t &pos) { pos.z -= x_twist_factors.b_ * pos.x / x_twist_factors.a_; }
}

#endif
