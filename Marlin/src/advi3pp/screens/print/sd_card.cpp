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
#include "sd_card.h"
#include "../../core/dgus.h"
#include "../../core/status.h"

namespace ADVi3pp {

SdCard sd_card;


//! Execute a SD Card command
//! @param key_value    The sub-action to handle
//! @return             True if the action was handled
bool SdCard::do_dispatch(KeyValue key_value)
{
    if(Parent::do_dispatch(key_value))
        return true;

    switch(key_value)
    {
        case KeyValue::SDUp:	up_command(); break;
        case KeyValue::SDDown:	down_command(); break;
        case KeyValue::SDLine1:
        case KeyValue::SDLine2:
        case KeyValue::SDLine3:
        case KeyValue::SDLine4:
        case KeyValue::SDLine5:	select_file_command(static_cast<uint16_t>(key_value) - 1); break;
        default:                return false;
    }

    return true;
}

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page SdCard::do_prepare_page()
{
    show_first_page();
    return Page::SdCard;
}

//! Show first SD card page
void SdCard::show_first_page()
{
    if(!ExtUI::isMediaInserted())
        return;

    page_index_ = 0;
    ExtUI::FileList files{};
    nb_files_ = files.count();
    last_file_index_ = nb_files_ > 0 ? nb_files_ - 1 : 0;

    show_current_page();
}

//! Handle Page Down command.
void SdCard::down_command()
{
    if(!ExtUI::isMediaInserted())
        return;

    if(last_file_index_ >= nb_visible_sd_files)
    {
        page_index_ += 1;
        last_file_index_ -= nb_visible_sd_files;
        show_current_page();
    }
}

//! Handle Page Up command.
void SdCard::up_command()
{
    if(!ExtUI::isMediaInserted())
        return;

    if(last_file_index_ + nb_visible_sd_files < nb_files_)
    {
        page_index_ -= 1;
        last_file_index_ += nb_visible_sd_files;
        show_current_page();
    }
}

//! Show the list of files on SD (current page)
void SdCard::show_current_page()
{
    WriteRamDataRequest frame{Variable::LongText0};

    ADVString<sd_file_length> name;
    ADVString<48> aligned_name;

    for(uint8_t index = 0; index < nb_visible_sd_files; ++index)
    {
        get_file_name(index, name);
        aligned_name.set(name, Alignment::Left);
        frame << aligned_name;
    }
    frame.send(true);

    frame.reset(Variable::Value0);
    frame << Uint16(page_index_ + 1);
    frame.send();
}

//! Get a filename with a given index.
//! @param index    Index of the filename
//! @param name     Copy the filename into this Chars
void SdCard::get_file_name(uint8_t index_in_page, ADVString<sd_file_length>& name)
{
    name.reset();
    if(last_file_index_ >= index_in_page)
    {
        ExtUI::FileList files{};
        files.seek(last_file_index_ - index_in_page);
        if(files.isDir()) name = "[";
            name += files.filename();
        if(files.isDir()) name += "]";
    }
}

//! Select a filename as sent by the LCD screen.
//! @param file_index    The index of the filename to select
void SdCard::select_file_command(uint16_t file_index)
{
    if(!ExtUI::isMediaInserted())
        return;

    if(file_index > last_file_index_)
        return;

    ExtUI::FileList files{};
    files.seek(last_file_index_ - file_index);
    if(files.isDir())
        return;

    const char* filename = files.filename();
    if(filename == nullptr) // If the SD card is not readable
        return;
    status.set_progress_name(filename);
    ExtUI::printFile(filename);

    pages.show_page(Page::Print, ShowOptions::None);
}

}
