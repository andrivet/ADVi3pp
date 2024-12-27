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
#include "../../core/core.h"
#include "../../core/dgus.h"
#include "../../core/status.h"
#include "grid.h"

namespace ADVi3pp::Grid {

  inline namespace internals {
    void show_command();
    void back_command();
    void save_command();
  }

  bool handle_command(uint16_t key_code) {
    switch(key_code) {
      case KEY_CODE_SHOW: show_command(); break;
      case KEY_CODE_BACK: back_command(); break;
      case KEY_CODE_SAVE: save_command(); break;
      default: return false;
    }
    return true;
  }

  inline namespace internals {

    void show_command() {
      ExtUI::bed_mesh_t& z_values = ExtUI::getMeshArray();

      adv::array<uint16_t, GRID_MAX_POINTS_Y * GRID_MAX_POINTS_X> data{};
      for(auto y = 0; y < GRID_MAX_POINTS_Y; y++)
        for(auto x = 0; x < GRID_MAX_POINTS_X; x++)
          data[y * GRID_MAX_POINTS_X + x] = static_cast<int16_t>(lround(z_values[x][y] * 100));

      WriteRamRequest{Variable::Value0}.write_words_data(data.data(), data.size());
      if(!ExtUI::getLevelingIsValid()) Status::set(GET_TEXT_F(ADVI3PP_MSG_LEVELING_NO_MESH), Status::STATUS_OPTIONS::RESET);

      Pages::show(Page::SensorGrid);
    }

    void back_command() {
      Pages::back(Pages::BACK_OPTIONS::NONE);
    }

    //! Handles the Save (Continue) command
    void save_command() {
      ExtUI::setLevelingActive(true);
      Pages::save(Pages::SAVE_OPTIONS::SETTINGS, Pages::BACK_OPTIONS::NONE);
    }

  }
}
