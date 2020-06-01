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

#include "diagnosis.h"
#include "../../core/task.h"
#include "../../core/dgus.h"

namespace ADVi3pp {

//! List of digital pins for the Diagnosis page
const uint8_t diagnosis_digital_pins[] =
        {
                54,     // PF0 / ADC0 - A0
                24,     // PA2 / AD2
                23,     // PA1 / AD1
                6,     // PH3 / OC4A
                25,     // PA3 / AD3

                40,     // PG1 / !RD
                56,     // PF2 / ADC2 - A2
                36,     // PC1 / A9
                37,     // PC0 / A8

                34,     // PC3 / A11
                35,     // PC2 / A10
                32,     // PC5 / A13
                33,     // PC4 / A12
        };

//! List of analog pins for the Diagnosis page
const uint8_t diagnosis_analog_pins[] = {55, 68, 54, 56}; // A1, A14, A0, A2

Diagnosis diagnosis;


//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page Diagnosis::do_prepare_page()
{
    task.set_background_task(BackgroundTask{this, &Diagnosis::send_data}, 250);
    return Page::Diagnosis;
}

//! Execute the Back command
void Diagnosis::do_back_command()
{
    task.clear_background_task();
    Parent::do_back_command();
}

//! Get current digital pin state (adapted from Arduino source code).
//! @param pin  Pin number to check.
//! @return     The current state: On (input), Off (input), Output
Diagnosis::State Diagnosis::get_pin_state(uint8_t pin)
{
    uint8_t mask = digitalPinToBitMask(pin);
    uint8_t port = digitalPinToPort(pin);
    if(port == NOT_A_PIN)
        return State::Off;

    volatile uint8_t* reg = portModeRegister(port);
    if(*reg & mask)
        return State::Output;

    uint8_t timer = digitalPinToTimer(pin);
    if(timer != NOT_ON_TIMER)
        return State::Output;

    return (*portInputRegister(port) & mask) ? State::On : State::Off;
}

//! Send the current data to the LCD panel.
void Diagnosis::send_data()
{
    WriteRamDataRequest request{Variable::Value0};

    for(size_t i = 0; i < adv::count_of(diagnosis_digital_pins); ++i)
    {
        request.reset(static_cast<Variable>(static_cast<uint16_t>(Variable::Value0) + i));
        request << Uint16{static_cast<uint16_t>(get_pin_state(diagnosis_digital_pins[i]))};
        request.send(false);
    }

    for(size_t i = 0; i < adv::count_of(diagnosis_analog_pins); ++i)
    {
        request.reset(static_cast<Variable>(static_cast<uint16_t>(Variable::Value0) + 0x20 + i));
        request << Uint16{static_cast<uint16_t>(analogRead(diagnosis_analog_pins[i]))};
        request.send(false);
    }
}

}
