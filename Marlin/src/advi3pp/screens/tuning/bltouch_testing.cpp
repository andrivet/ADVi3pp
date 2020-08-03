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

#include "../../parameters.h"
#include "../../core/core.h"
#include "../../core/dgus.h"
#include "../../core/status.h"
#include "bltouch_testing.h"
#ifdef BLTOUCH
#include "../../../feature/bltouch.h"
#endif

namespace ADVi3pp {

BLTouchTesting bltouch_testing;


#ifdef BLTOUCH

void reset_bltouch_command()
{
    core.inject_commands(F("M280 P0 S160"));
}

void selftest_bltouch_command()
{
    core.inject_commands(F("M280 P0 S120"));
}


//! Execute acommand
//! @param key_value    The sub-action to handle
//! @return             True if the action was handled
bool BLTouchTesting::do_dispatch(KeyValue key_value)
{
    if(Parent::do_dispatch(key_value))
        return true;

    switch(key_value)
    {
        case KeyValue::BLTouchTestingStep1Yes:  step1yes(); break;
        case KeyValue::BLTouchTestingStep1No:   step1no(); break;
        case KeyValue::BLTouchTestingStep2Yes:  step2yes(); break;
        case KeyValue::BLTouchTestingStep2No:   step2no(); break;
        case KeyValue::BLTouchTestingStep3Yes:  step3yes(); break;
        case KeyValue::BLTouchTestingStep3No:   step3no(); break;
        default: return false;
    }

    return true;
}

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page BLTouchTesting::do_prepare_page()
{
    if(!core.ensure_not_printing())
        return Page::None;
    step1();
    return Page::BLTouchTesting1;
}

void BLTouchTesting::step1()
{
    status.reset();
    tested_ = ok_ = Wires::None;
    set_bits(tested_, Wires::Brown | Wires::Red);
    reset_bltouch_command();
}

void BLTouchTesting::step1yes()
{
    set_bits(ok_, Wires::Brown | Wires::Red);
    step2();
}

void BLTouchTesting::step1no()
{
    status.set(F("Problem with Red/Brown wiring"));
    step4();
}

void BLTouchTesting::step2()
{
    set_bits(tested_, Wires::Orange);
    selftest_bltouch_command();
    pages.show_page(Page::BLTouchTesting2, ShowOptions::None);
}

void BLTouchTesting::step2yes()
{
    set_bits(ok_, Wires::Orange);
    reset_bltouch_command();
    step3();
}

void BLTouchTesting::step2no()
{
    reset_bltouch_command();
    status.set(F("Problem with Orange wiring"));
    step4();
}

void BLTouchTesting::step3()
{
    set_bits(tested_, Wires::White | Wires::Black);
    pages.show_page(Page::BLTouchTesting3, ShowOptions::None);
    if(bltouch.deploy())
    {
        status.set(F("Deployment of BLTouch failed"));
        if(bltouch.triggered())
            clear_bits(ok_, Wires::Orange);
        step4();
    }
}

void BLTouchTesting::step3yes()
{
    set_bits(ok_, Wires::White | Wires::Black);
    step4();
}

void BLTouchTesting::step3no()
{
    clear_bits(ok_, Wires::Orange);
    bltouch.stow();
    status.set(F("Problem with White/Black wiring"));
    step4();
}

uint16_t BLTouchTesting::wire_value(Wires wire)
{
    return test_one_bit(ok_, wire) ? 1 : test_one_bit(tested_, wire) ? 2 : 0;
}

void BLTouchTesting::step4()
{
    auto brown = wire_value(Wires::Brown);
    auto red = wire_value(Wires::Red);
    auto orange = wire_value(Wires::Orange);
    auto black = wire_value(Wires::Black);
    auto white = wire_value(Wires::White);

    if(brown == 1 && red == 1 && orange == 1 && black == 1 && white == 1)
        status.set(F("No problem detected with BLTouch"));

    WriteRamDataRequest frame{Variable::Value0};
    frame << Uint16{brown}
          << Uint16{red}
          << Uint16{orange}
          << Uint16{black}
          << Uint16{white};
    frame.send();

    pages.show_page(Page::BLTouchTesting4, ShowOptions::None);
}

#else

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page BLTouchTesting::do_prepare_page()
{
    return Page::NoSensor;
}

#endif

}
