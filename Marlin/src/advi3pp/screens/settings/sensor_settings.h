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

struct SensorPosition { int16_t x, y; };

//! Sensor Settings Page
struct SensorSettings: Screen<SensorSettings>
{
    static const size_t NB_SENSOR_POSITIONS = 3;

#ifdef ADVi3PP_PROBE
    SensorSettings();

    int x_probe_offset_from_extruder() const;
    int y_probe_offset_from_extruder() const;
    int left_probe_bed_position();
    int right_probe_bed_position();
    int front_probe_bed_position();
    int back_probe_bed_position();
#endif

private:
    Page do_prepare_page();

#ifdef ADVi3PP_PROBE
    bool do_dispatch(KeyValue value);
    void do_save_command();
    void previous_command();
    void next_command();
    void send_data() const;
    void get_data();
#endif

    void do_write(EepromWrite& eeprom) const;
    void do_read(EepromRead& eeprom);
    void do_reset();
    uint16_t do_size_of() const;

private:
    uint16_t index_ = 0;
    SensorPosition positions_[NB_SENSOR_POSITIONS];

    friend Parent;
};

extern SensorSettings sensor_settings;

}
