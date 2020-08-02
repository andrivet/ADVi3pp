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
#include "bltouch_testing.h"

namespace ADVi3pp {

BLTouchTesting bltouch_testing;


#ifdef BLTOUCH

//! Execute acommand
//! @param key_value    The sub-action to handle
//! @return             True if the action was handled
bool BLTouchTesting::do_dispatch(KeyValue key_value)
{
    if(Parent::do_dispatch(key_value))
        return true;

    switch(key_value)
    {
        case KeyValue::BLTouchTestingStep2:     step2(); break;
        case KeyValue::BLTouchTestingStep2Yes:  step2yes(); break;
        case KeyValue::BLTouchTestingStep2No:   step2no(); break;
        case KeyValue::BLTouchTestingStep3:     step3(); break;
        case KeyValue::BLTouchTestingStep3Yes:  step3yes(); break;
        case KeyValue::BLTouchTestingStep3No:   step3no(); break;
        case KeyValue::BLTouchTestingStep4:     step4(); break;
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
    return Page::BLTouchTesting1;
}

void BLTouchTesting::step2()
{
    // TODO
}

void BLTouchTesting::step2yes()
{
    // TODO
}

void BLTouchTesting::step2no()
{
    // TODO
}

void BLTouchTesting::step3()
{
    // TODO
}

void BLTouchTesting::step3yes()
{
    // TODO
}

void BLTouchTesting::step3no()
{
    // TODO
}

void BLTouchTesting::step4()
{
    // TODO
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
