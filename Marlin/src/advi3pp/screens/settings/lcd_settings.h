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

#pragma once

#include "../core/screen.h"

namespace ADVi3pp {

//! LCD Setting Page
struct LcdSettings: Screen<LcdSettings>
{
    void normal_brightness_command(uint16_t brightness);
    void dimming_brightness_command(uint16_t brightness);

    bool receive();
    void send();
    void sleep_on();
    void sleep_off();

private:
    friend Parent;

    bool do_dispatch(KeyValue key_value);
    Page do_prepare_page();
    void do_back_command();
    void do_save_command();
    void dimming_command();
    void send_values(uint16_t time, uint8_t normal, uint8_t dimmed);
    bool get_values(uint16_t &time, uint8_t &normal, uint8_t &dimmed) const;

    bool dimming_ = false;
};

extern LcdSettings lcd_settings;

}
