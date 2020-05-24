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

#include "../../core/string.h"
#include "../core/screen.h"

namespace ADVi3pp {

const uint8_t sd_file_length = 26; //!< This is the maximum length handled by the SD layer (FILENAME_LENGTH)

//! SD Card Page
struct SdCard: Screen<SdCard>
{
    void show_first_page();

private:
    bool do_dispatch(KeyValue value);
    Page do_prepare_page();
    void show_current_page();
    void get_file_name(uint8_t index_in_page, ADVString<sd_file_length>& name);
    void up_command();
    void down_command();
    void select_file_command(uint16_t file_index);

private:
    static constexpr uint16_t nb_visible_sd_files = 5; //!< Number of files per page on the SD screen

    uint16_t nb_files_ = 0;
    uint16_t last_file_index_ = 0;
    uint16_t page_index_ = 0;

    friend Parent;
};

extern SdCard sd_card;

}
