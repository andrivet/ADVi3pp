/**
 * ADVi3++ Firmware For Wanhao Duplicator i3 Plus (based on Marlin 2)
 *
 * Copyright (C) 2017-2022 Sebastien Andrivet [https://github.com/andrivet/]
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

#include "../parameters.h"
#include "../core/buzzer.h"
#include "../core/dimming.h"
#include "../../lcd/marlinui.h"

using namespace ADVi3pp;


void MarlinUI::buzz(const long duration, const uint16_t freq) {
    buzzer.buzz_on_action(duration);
}

void MarlinUI::sleep_on() {
    dimming.sleep_on();
}

void MarlinUI::sleep_off() {
    dimming.sleep_off();
}

void MarlinUI::_set_brightness() {
    dimming.send_brightness_to_lcd();
}
