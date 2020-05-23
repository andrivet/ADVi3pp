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

//! Firmware Setting Page
struct FirmwareSettings: Screen<FirmwareSettings>
{
private:
    bool do_dispatch(KeyValue key_value);
    Page do_prepare_page();
    void do_save_command();
    void thermal_protection_command();
    void runout_sensor_command();
    void send_usb_baudrate() const;
    void send_features() const;
    void baudrate_minus_command();
    void baudrate_plus_command();
    size_t usb_baudrate_index(uint32_t baudrate);

    uint32_t usb_baudrate_ = BAUDRATE;
    Feature features_ = Feature::BuzzOnPress | Feature::Dimming | Feature::ThermalProtection;

    friend Parent;
};

}
