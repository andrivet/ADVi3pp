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

#include "../../core/bitmasks.h"
#include "../core/screen.h"

namespace ADVi3pp {

#ifdef BLTOUCH
//! BLTouch Testing Page
struct BLTouchTesting: Screen<BLTouchTesting>
{
private:
    enum class Wires: uint8_t
    {
        None    = 0b00000000,
        Brown   = 0b00000001,
        Red     = 0b00000010,
        Orange  = 0b00000100,
        Black   = 0b00001000,
        White   = 0b00010000
    };

private:
    bool do_dispatch(KeyValue key_value);
    Page do_prepare_page();
    void step1();
    void step1yes();
    void step1no();
    void step2();
    void step2yes();
    void step2no();
    void step3();
    void step3yes();
    void step3no();
    void step4();
    uint16_t wire_value(Wires wire);

private:
    Wires tested_ = Wires::None;
    Wires ok_ = Wires::None;

    friend Parent;
};

ENABLE_BITMASK_OPERATOR(BLTouchTesting::Wires);

#else
//! BLTouch Testing Page
struct BLTouchTesting: Screen<BLTouchTesting>
{
private:
    Page do_prepare_page();
    friend Parent;
};
#endif

extern BLTouchTesting bltouch_testing;

}
