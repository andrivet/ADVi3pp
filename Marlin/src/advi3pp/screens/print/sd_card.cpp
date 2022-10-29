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

#include "../../parameters.h"
#include "sd_card.h"
#include "../../core/dgus.h"
#include "../../core/status.h"

namespace ADVi3pp {

SdCard sd_card;


//! Execute a SD Card command
//! @param key_value    The sub-action to handle
//! @return             True if the action was handled
bool SdCard::on_dispatch(KeyValue key_value) {
  if(Parent::on_dispatch(key_value))
    return true;

  switch(key_value) {
    case KeyValue::SDUp: up_command(); break;
    case KeyValue::SDDown: down_command(); break;
    case KeyValue::SDParent: parent_command(); break;
    case KeyValue::SDLine1:
    case KeyValue::SDLine2:
    case KeyValue::SDLine3:
    case KeyValue::SDLine4:
    case KeyValue::SDLine5:
      select_command(static_cast<uint16_t>(key_value) - 1); break;
    default: return false;
  }

  return true;
}

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
void SdCard::on_enter() {
  show_initial();
}

//! Show first SD card page
void SdCard::show_initial() {
  ExtUI::mountMedia();
  status.reset();

  if(!ExtUI::isMediaInserted() || !ExtUI::isMediaMounted()) {
    show_empty();
    return;
  }

  files_.refresh();
  show_folder_first_page();
}

//! Show first SD card page
void SdCard::show_folder_first_page() {
  page_index_ = 0;
  show_folder_current_page();
}

bool SdCard::check_media() {
  if(ExtUI::isMediaInserted() && ExtUI::isMediaMounted())
    return true;
  show_empty();
  return false;
}

//! Handle Page Down command.
void SdCard::down_command() {
  if(!check_media())
      return;

  if(files_.count() <= page_index_ * NB_VISIBLE_SD_FILES + NB_VISIBLE_SD_FILES)
    return;

  page_index_ += 1;
  show_folder_current_page();
}

//! Handle Page Up command.
void SdCard::up_command() {
  if(!check_media())
    return;

  if(page_index_ <= 0)
    return;

  page_index_ -= 1;
  show_folder_current_page();
}

//! Handle Page Up command.
void SdCard::parent_command() {
  if(!check_media())
    return;

  if( files_.isAtRootDir())
    return;

  files_.upDir();
  files_.refresh();
  show_folder_current_page();
}

//! Show the list of files on SD (current page)
void SdCard::show_folder_current_page() {
  ADVString<48> name;
  FileType file_type = FileType::None;

  for(uint8_t index = 0; index < NB_VISIBLE_SD_FILES; ++index) {
    get_file_name(index, name, file_type);
    Log::log() << index << static_cast<uint16_t>(file_type) << name.get() << Log::endl();

    auto var = static_cast<Variable>(static_cast<uint16_t>(Variable::LongText0) + 24 * index);
    WriteRamRequest{var}.write_text(name);

    var = static_cast<Variable>(static_cast<uint16_t>(Variable::Value0) + index);
    WriteRamRequest{var}.write_word(static_cast<uint16_t>(file_type));
  }

  auto nb_pages = (files_.count() + NB_VISIBLE_SD_FILES - 1) / NB_VISIBLE_SD_FILES;
  WriteRamRequest{Variable::Value5}.write_words(page_index_ + 1, nb_pages <= 0 ? 1 : nb_pages);
}

void SdCard::show_empty() {
  page_index_ = 0;

  ADVString<48> name;
  FileType file_type = FileType::None;

  for(uint8_t index = 0; index < NB_VISIBLE_SD_FILES; ++index) {
    auto var = static_cast<Variable>(static_cast<uint16_t>(Variable::LongText0) + 24 * index);
    WriteRamRequest{var}.write_text(name);

    var = static_cast<Variable>(static_cast<uint16_t>(Variable::Value0) + index);
    WriteRamRequest{var}.write_word(static_cast<uint16_t>(file_type));
  }

  WriteRamRequest{Variable::Value5}.write_words(0, 0);
}

//! Get a filename with a given index.
//! @param index    Index of the filename
//! @param name     Copy the filename into this Chars
void SdCard::get_file_name(uint8_t index_in_page, ADVString<48>& name, FileType &type) {
  name.reset();
  type = FileType::None;

  auto absolute_index = index_in_page + page_index_ * NB_VISIBLE_SD_FILES;
  if(absolute_index < files_.count()) {
    ExtUI::FileList files{};
    files.seek(absolute_index);
    name += files.filename();
    type = files.isDir() ? FileType::Folder : FileType::File;
  }
}

//! Select a filename as sent by the LCD screen.
//! @param file_index    The index of the filename to select
void SdCard::select_command(uint16_t file_index) {
  if(!check_media())
    return;

  auto absolute_index = file_index + page_index_ * NB_VISIBLE_SD_FILES;
  if(absolute_index >= files_.count())
    return;

  files_.seek(absolute_index);

  const char* filename = files_.shortFilename();
  if(filename == nullptr) { // If the SD card is not readable
    ExtUI::onMediaOpenError(filename);
    return;
  }

  if(files_.isDir())
      select_directory();
  else
      select_file();
}

void SdCard::select_file() {
  status.set_filename(files_.filename());
  ExtUI::printFile(files_.shortFilename());
  pages.show(Page::Print, Action::Print);
}

void SdCard::select_directory() {
  files_.changeDir(files_.shortFilename());
  files_.refresh();
  show_folder_first_page();
}

void SdCard::on_media_inserted() {
  show_initial();
}

void SdCard::on_media_removed() {
  show_empty();
}

void SdCard::on_media_error() {
  show_empty();
}

}