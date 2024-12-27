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

#include "../../../inc/MarlinConfig.h"
#include "../../../lcd/extui/ui_api.h"
#include "preheat.h"
#include "../../core/core.h"
#include "../../core/dgus.h"
#include "../../core/status.h"
#include "../../core/pool.h"

namespace ADVi3pp::Preheat {

  inline namespace internals {
    constexpr uint16_t KEY_CODE_PREVIOUS = 1;
    constexpr uint16_t KEY_CODE_NEXT = 2;
    constexpr uint16_t KEY_CODE_COOLDOWN = 3;
    constexpr Variable VAR_TEMP = Variable::Value0;

    struct Data {;
      size_t index_ = 0;
    };

    inline Data& pool() { return Pool::get<Data>(Page::Preheat); }

    void show_command();
    void save_command();
    void back_command();
    void previous_command();
    void next_command();
    void cooldown_command();

    void send_presets();
    void retrieve_presets();
  }

  //! Handle Preheat actions.
  //! @param key_value    Sub-action to handle
  //! @return             True if the action was handled
  bool handle_command(uint16_t key_code) {
    switch(key_code) {
      case KEY_CODE_SHOW: show_command(); break;
      case KEY_CODE_BACK: back_command(); break;
      case KEY_CODE_SAVE: save_command(); break;
      case KEY_CODE_PREVIOUS: previous_command(); break;
      case KEY_CODE_NEXT: next_command(); break;
      case KEY_CODE_COOLDOWN: cooldown_command(); break;
      default: return false;
    }
    return true;
  }

  inline namespace internals {

    void show_command() {
      if(!Core::check_not_busy()) return;
      Pool::reset<Data>(Page::Preheat);
      Status::reset();
      send_presets();
      Pages::show(Page::Preheat);
    }

    void back_command() {
      Pages::back(Pages::BACK_OPTIONS::NONE);
    }

    //! Send the presets t the LCD Panel
    void send_presets() {
      WriteRamRequest{VAR_TEMP}.write_words(
        ExtUI::getMaterialPresetHotendTemp_celsius(pool().index_),
        ExtUI::getMaterialPresetBedTemp_celsius(pool().index_),
        ExtUI::getMaterialPresetFanSpeed_percent(pool().index_)
      );

      ADVString<3> preset;
      preset << pool().index_ + 1 << F("/") << PREHEAT_COUNT;
      WriteRamRequest{Variable::ShortText0}.write_text(preset.get(), SHORT_TEXT_LENGTH);
    }

    //! Retrieve presets values from the LCD Panel
    void retrieve_presets() {
      ReadRam frame{VAR_TEMP};
      if(!frame.send_receive(3)) return;

      auto hotend = frame.read_word<int16_t>();
      auto bed = frame.read_word<int16_t>();
      auto fan = frame.read_word<uint8_t>();

      ExtUI::setMaterialPreset(pool().index_, hotend, bed, fan);
    }

    //! Handle Previous command
    void previous_command() {
      retrieve_presets();
      pool().index_ = pool().index_ > 0 ? pool().index_ - 1 : PREHEAT_COUNT - 1;
      send_presets();
    }

    //! Handle Next command
    void next_command() {
      retrieve_presets();
      pool().index_ = pool().index_ < PREHEAT_COUNT - 1 ? pool().index_ + 1 : 0;
      send_presets();
    }

    //! Handles the Save (Continue) command
    void save_command() {
      retrieve_presets();

      ExtUI::setTargetTemp_celsius(ExtUI::getMaterialPresetHotendTemp_celsius(pool().index_), ExtUI::E0, true);
      ExtUI::setTargetTemp_celsius(ExtUI::getMaterialPresetBedTemp_celsius(pool().index_), ExtUI::BED, true);
      ExtUI::setTargetFan_percent(ExtUI::getMaterialPresetFanSpeed_percent(pool().index_), ExtUI::FAN0);

      Status::set(GET_TEXT_F(ADVI3PP_MSG_PREHEAT), Status::STATUS_OPTIONS::RESET);
      Pages::save(Pages::SAVE_OPTIONS::SETTINGS, Pages::BACK_OPTIONS::NONE);
    }

    //! Cooldown the bed and the nozzle, turn off the fan
    void cooldown_command() {
      // If printing, do nothing
      if(Core::is_printing()) return;
      ExtUI::setTargetTemp_celsius(0, ExtUI::BED);
      ExtUI::setTargetTemp_celsius(0, ExtUI::E0);
      ExtUI::setTargetFan_percent(0, ExtUI::FAN0);
      Status::set(GET_TEXT_F(MSG_COOLDOWN), Status::STATUS_OPTIONS::RESET);
    }

  }
}
