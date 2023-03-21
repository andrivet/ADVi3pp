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

#include "../../core/string.h"
#include "../../core/screen.h"

namespace ADVi3pp {

//! SD Card Page
struct SdCard: Screen<SdCard> {
  static constexpr Page PAGE = Page::SdCard;
  static constexpr Action ACTION = Action::SdCard;

  enum class FileType { None, File, Folder };

  void on_media_inserted();
  void on_media_removed();
  void on_media_error();

private:
  bool on_dispatch(KeyValue value);
  bool on_enter();

  void show_initial();
  void show_folder_first_page();
  void show_folder_current_page();
  void show_empty();
  void get_file_name(uint8_t index_in_page, ADVString<48>& name, FileType &type);
  void up_command();
  void down_command();
  void parent_command();
  void select_command(uint16_t file_index);
  void select_file();
  void select_directory();
  bool check_media();

private:
  static constexpr uint16_t NB_VISIBLE_SD_FILES = 5; //!< Number of files per page on the SD screen

  uint16_t page_index_ = 0;
  ExtUI::FileList files_{};

  friend Parent;
};

extern SdCard sd_card;

}
