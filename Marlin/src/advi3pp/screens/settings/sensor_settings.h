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

#ifdef ADVi3PP_PROBE
//! Sensor Settings Page
struct SensorSettings: Handler<SensorSettings>
{
    static const size_t NB_SENSOR_POSITIONS = 3;

    SensorSettings();

    int x_probe_offset_from_extruder() const;
    int y_probe_offset_from_extruder() const;
    int left_probe_bed_position();
    int right_probe_bed_position();
    int front_probe_bed_position();
    int back_probe_bed_position();

private:
    bool do_dispatch(KeyValue value);
    Page do_prepare_page();
    void do_save_command();
    void do_write(EepromWrite& eeprom) const;
    void do_read(EepromRead& eeprom);
    void do_reset();
    uint16_t do_size_of() const;
    void previous_command();
    void next_command();
    void send_data() const;
    void get_data();

private:
    uint16_t index_ = 0;
    SensorPosition positions_[NB_SENSOR_POSITIONS];

    friend Parent;
};
#else
//! Sensor Settings Page
struct SensorSettings: Screen<SensorSettings>
{
    void send_z_height_to_lcd(double) {}
    void save_lcd_z_height() {}

private:
    Page do_prepare_page();
    friend Parent;
};
#endif

extern SensorSettings sensor_settings;

}
