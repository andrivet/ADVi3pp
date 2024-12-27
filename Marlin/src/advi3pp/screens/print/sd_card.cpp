/**
 * ADVi3++ Firmware For Wanhao Duplicator i3 Plus (based on Marlin 2)
 *
 * Copyright (C) 2017-2025 Sebastien Andrivet [https://github.com/andrivet/]
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

#include "../../../lcd/extui/ui_api.h"
#include "sd_card.h"
#include "../../core/core.h"
#include "../../core/dgus.h"
#include "../../core/status.h"
#include "../../core/pool.h"
#include "../../core/progress.h"

namespace ADVi3pp::SdCard {

  inline namespace internals {
    constexpr uint16_t NB_VISIBLE_SD_FILES = 5; //!< Number of files per page on the SD screen
    constexpr uint16_t LONG_TEXT_VARIABLES_DIFF = static_cast<uint16_t>(Variable::LongText1) - static_cast<uint16_t>(Variable::LongText0);
    enum class FileType { None, File, Folder };
    constexpr uint16_t KEY_CODE_UP = 1;
    constexpr uint16_t KEY_CODE_DOWN = 2;
    constexpr uint16_t KEY_CODE_PARENT = 3;
    constexpr uint16_t KEY_CODE_LINE1 = 4;
    constexpr uint16_t KEY_CODE_LINE2 = 5;
    constexpr uint16_t KEY_CODE_LINE3 = 6;
    constexpr uint16_t KEY_CODE_LINE4 = 7;
    constexpr uint16_t KEY_CODE_LINE5 = 8;

    struct Data {
      uint16_t page_index_ = 0;
      ExtUI::FileList files_{};
    };

    inline Data& pool() { return Pool::get<Data>(Page::SdCard); }

    void show_command();
    void back_command();
    void up_command();
    void down_command();
    void parent_command();
    void select_command(uint16_t file_index);

    void show_initial();
    void show_folder_first_page();
    void show_folder_current_page();
    void show_empty();
    const char *get_file_name(uint8_t index_in_page, FileType &type);
    void select_file();
    void select_directory();
    bool check_media();
  }

  bool handle_command(uint16_t key_code) {
    switch(key_code) {
      case KEY_CODE_SHOW: show_command(); break;
      case KEY_CODE_BACK: back_command(); break;
      case KEY_CODE_UP: up_command(); break;
      case KEY_CODE_DOWN: down_command(); break;
      case KEY_CODE_PARENT: parent_command(); break;
      case KEY_CODE_LINE1:
      case KEY_CODE_LINE2:
      case KEY_CODE_LINE3:
      case KEY_CODE_LINE4:
      case KEY_CODE_LINE5: select_command(key_code - KEY_CODE_LINE1); break;
      default: return false;
    }
    return true;
  }

  void on_media_inserted() {
    Status::set(GET_TEXT_F(ADVI3PP_MSG_SD_CARD_DETECTED), Status::STATUS_OPTIONS::RESET);
    auto current = Pages::get_current_page();
    if(current == Page::SdCard)
      show_initial();
    else if(Pages::get_current_page() == Page::Main)
      Core::display(Page::SdCard);
  }

  void on_media_removed() {
    Status::set(GET_TEXT_F(ADVI3PP_MSG_SD_CARD_REMOVED), Status::STATUS_OPTIONS::RESET);
    if(Pages::get_current_page() != Page::SdCard) return;
    show_empty();
  }

  void on_media_error() {
    Status::set(GET_TEXT_F(ADVI3PP_MSG_SD_CARD_ERROR), Status::STATUS_OPTIONS::RESET | Status::STATUS_OPTIONS::PERSISTENT);
    if(Pages::get_current_page() != Page::SdCard) return;
    show_empty();
  }

  void on_media_open_error() {
    Status::set(GET_TEXT_F(ADVI3PP_MSG_SD_CARD_ERROR), Status::STATUS_OPTIONS::RESET | Status::STATUS_OPTIONS::PERSISTENT);
  }

  inline namespace internals {

    void show_command() {
      Pool::reset<Data>(Page::SdCard);
      Pages::show(Page::SdCard);
      show_initial();
    }

    void back_command() {
      Pages::back(Pages::BACK_OPTIONS::NONE);
    }

    //! Show first SD card page
    void show_initial() {
      Progress::set_animation(true);
      show_empty();
      ExtUI::mountMedia();

      if(!ExtUI::isMediaMounted()) {
        Progress::set_animation(false);
        return;
      }

      Status::set(GET_TEXT_F(ADVI3PP_MSG_SD_CARD_DETECTED), Status::STATUS_OPTIONS::RESET);
      pool().files_.refresh();
      show_folder_first_page();
      Progress::set_animation(false);
    }

    //! Show first SD card page
    void show_folder_first_page() {
      pool().page_index_ = 0;
      show_folder_current_page();
    }

    bool check_media() {
      if(ExtUI::isMediaMounted()) return true;
      show_empty();
      return false;
    }

    //! Handle Page Down command.
    void down_command() {
      if(!check_media()) return;
      if(pool().files_.count() <= pool().page_index_ * NB_VISIBLE_SD_FILES + NB_VISIBLE_SD_FILES) return;
      pool().page_index_ += 1;
      show_folder_current_page();
    }

    //! Handle Page Up command.
    void up_command() {
      if(!check_media()) return;
      if(pool().page_index_ <= 0) return;
      pool().page_index_ -= 1;
      show_folder_current_page();
    }

    //! Handle Page Up command.
    void parent_command() {
      if(!check_media()) return;
      if(pool().files_.isAtRootDir()) return;

      pool().files_.upDir();
      pool().files_.refresh();
      show_folder_current_page();
    }

    //! Show the list of files on SD (current page)
    void show_folder_current_page() {
      FileType file_type = FileType::None;

      for(uint8_t index = 0; index < NB_VISIBLE_SD_FILES; ++index) {
        auto file_name = get_file_name(index, file_type);
        auto var = static_cast<Variable>(static_cast<uint16_t>(Variable::LongText0) + LONG_TEXT_VARIABLES_DIFF * index);
        WriteRamRequest{var}.write_text(file_name, LONG_TEXT_LENGTH);
        var = static_cast<Variable>(static_cast<uint16_t>(Variable::Value0) + index);
        WriteRamRequest{var}.write_word(file_type);
      }

      auto nb_pages = (pool().files_.count() + NB_VISIBLE_SD_FILES - 1) / NB_VISIBLE_SD_FILES;
      WriteRamRequest{Variable::Value5}.write_words(pool().page_index_ + 1, nb_pages <= 0 ? 1 : nb_pages);
    }

    void show_empty() {
      pool().page_index_ = 0;
      FileType file_type = FileType::None;

      for(uint8_t index = 0; index < NB_VISIBLE_SD_FILES; ++index) {
        auto var = static_cast<Variable>(static_cast<uint16_t>(Variable::LongText0) + LONG_TEXT_VARIABLES_DIFF * index);
        WriteRamRequest{var}.write_text(F(""), LONG_TEXT_LENGTH);

        var = static_cast<Variable>(static_cast<uint16_t>(Variable::Value0) + index);
        WriteRamRequest{var}.write_word(file_type);
      }

      WriteRamRequest{Variable::Value5}.write_words(0, 0);
    }

    //! Get a filename with a given index.
    //! @param index    Index of the filename
    //! @param name     Copy the filename into this Chars
    const char *get_file_name(uint8_t index_in_page, FileType &type) {
      type = FileType::None;

      auto absolute_index = index_in_page + pool().page_index_ * NB_VISIBLE_SD_FILES;
      if(absolute_index < pool().files_.count()) {
        ExtUI::FileList files{};
        files.seek(absolute_index);
        type = files.isDir() ? FileType::Folder : FileType::File;
        return files.filename();
      }

      return "";
    }

    //! Select a filename as sent by the LCD screen.
    //! @param file_index    The index of the filename to select
    void select_command(uint16_t file_index) {
      if(!check_media()) return;

      auto absolute_index = file_index + pool().page_index_ * NB_VISIBLE_SD_FILES;
      if(absolute_index >= pool().files_.count()) return;

      pool().files_.seek(absolute_index);

      const char* filename = pool().files_.shortFilename();
      if(filename == nullptr) { // If the SD card is not readable
        ExtUI::onMediaOpenError(filename);
        return;
      }

      if(pool().files_.isDir())
        select_directory();
      else
        select_file();
    }

    void select_file() {
      ExtUI::printFile(pool().files_.shortFilename());
      Pages::clear_current(); // To avoid putting it onto the stack
      Core::display(Page::Print);
    }

    void select_directory() {
      pool().files_.changeDir(pool().files_.shortFilename());
      pool().files_.refresh();
      show_folder_first_page();
    }

  }
}
