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

//! Preheat Page
struct Preheat: Screen<Preheat>
{
    static const size_t NB_PRESETS = 5;

private:
    bool do_dispatch(KeyValue key_value);
    Page do_prepare_page();
    void do_save_command();
    void do_write(EepromWrite& eeprom) const;
    void do_read(EepromRead& eeprom);
    void do_reset();
    uint16_t do_size_of() const;

    void send_presets();
    void retrieve_presets();
    void previous_command();
    void next_command();
    void cooldown_command();

private:
    Preset presets_[NB_PRESETS] = {};
    size_t index_ = 0;

    friend Parent;
};

extern Preheat preheat;

}
