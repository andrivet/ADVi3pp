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

#include "../../lcd/extui/ui_api.h"

namespace ADVi3pp
{
    struct x_twist_factors_t
    {
        void twist(xyze_pos_t &pos) const   { pos.z += get_z(pos.x); }
        void untwist(xyze_pos_t &pos) const { pos.z -= get_z(pos.x); }
        void reset() { a_ = DEFAULT_X_TWIST_A; b_ = DEFAULT_X_TWIST_B; }
        void compute(float x0, float z0, float x1, float z1) { a_ = x1 - x0; b_ = z1 - z0; }
        float get_a() const { return a_; }
        float get_b() const { return b_; }
        void set_a_b(float a, float b) { a_ = a; b_ = b; }

    private:
        const float DEFAULT_X_TWIST_A = 100.0;
        const float DEFAULT_X_TWIST_B = 0.0;

        float get_z(float x) const { return b_ * (x - X_BED_SIZE / 2) / a_; }

        float a_ = DEFAULT_X_TWIST_A;
        float b_ = DEFAULT_X_TWIST_B;
    };

    extern x_twist_factors_t x_twist_factors;
}

#endif
