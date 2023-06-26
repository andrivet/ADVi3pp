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

#include "../../../inc/MarlinConfig.h"
#include "../../core/core.h"
#include "../../core/dgus.h"
#include "grid.h"

namespace ADVi3pp {

LevelingGrid leveling_grid;

#ifdef ADVi3PP_PROBE

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
bool LevelingGrid::on_enter() {
  ExtUI::bed_mesh_t& z_values = ExtUI::getMeshArray();

  adv::array<uint16_t, GRID_MAX_POINTS_Y * GRID_MAX_POINTS_X> data{};
  for(auto y = 0; y < GRID_MAX_POINTS_Y; y++)
    for(auto x = 0; x < GRID_MAX_POINTS_X; x++)
      data[y * GRID_MAX_POINTS_X + x] = static_cast<int16_t>(lround(z_values[x][y] * 100));

  WriteRamRequest{Variable::Value0}.write_words_data(data.data(), data.size());
  return true;
}

//! Handles the Save (Continue) command
void LevelingGrid::on_save_command() {
  ExtUI::setLevelingActive(true);
  Parent::on_save_command();
}

void LevelingGrid::on_back_command() {
  pages.back();
  Parent::on_back_command();
}

#endif

}
