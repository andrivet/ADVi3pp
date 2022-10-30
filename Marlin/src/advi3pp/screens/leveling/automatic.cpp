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
#include "../../core/core.h"
#include "../../core/status.h"
#include "../../core/wait.h"
#include "grid.h"
#include "automatic.h"

namespace ADVi3pp {

AutomaticLeveling automatic_leveling;

bool AutomaticLeveling::on_dispatch(KeyValue key_value) {
  if(Parent::on_dispatch(key_value))
    return true;

  switch(key_value) {
    case KeyValue::LevelingResetProbe:	reset_command(); break;
    default: return false;
  }

  return true;
}


//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
void AutomaticLeveling::on_enter() {
#ifdef ADVi3PP_PROBE
  start();
#endif
}

void AutomaticLeveling::reset_command() {
#ifdef ADVi3PP_PROBE
  core.inject_commands(F("M280 P0 S160"));
#else
  pages.show(PAGE, ACTION);
#endif
}

void AutomaticLeveling::on_back_command() {
  status.set(F("Canceling leveling..."));
  ExtUI::cancelLeveling();
  Parent::on_back_command();
}

void AutomaticLeveling::on_abort() {
  ExtUI::cancelLeveling();
}

void AutomaticLeveling::start() {
  lcd_leveling_ = true;
  pages.save_forward_page();
  wait.home_and_wait(Callback{this, &AutomaticLeveling::home_task}, F("G28\nG1 Z4 F1200")); // Homing, always
}

//! Check if the printer is homed, and continue the Z Height Tuning process.
void AutomaticLeveling::home_task() {
  if(!wait.check_homed())
    return;

  adv::array<uint16_t, GRID_MAX_POINTS_Y * GRID_MAX_POINTS_X> data{};
  WriteRamRequest{Variable::Value0}.write_words_data(data.data(), data.size());

  // homing, raise head, leveling, go back to corner, activate compensation
  core.inject_commands(ExtUI::isLevelingHighSpeed() ? F("G29") : F("G29 E"));
}

void AutomaticLeveling::on_progress(uint8_t index, uint8_t x, uint8_t y) {
  status.format(F("Probing #%i at %i x %i mm"), index, x, y);

  if(pages.get_current_page() != Page::AutomaticLeveling)
    return;

  adv::array<uint16_t, GRID_MAX_POINTS_Y * GRID_MAX_POINTS_X> data{};
  data[index - 1] = 2; // index starts at 1

  WriteRamRequest{Variable::Value0}.write_words_data(data.data(), data.size());
}

//! Called by Marlin when G29 (automatic bed leveling) is finished.
//! @param success Boolean indicating if the leveling was successful or not.
void AutomaticLeveling::on_done(bool success) {
  Log::log() << "on_done" << success << Log::endl();
  if(!success)
    status.set(F("Leveling failure or aborted, please wait..."));

  if(!lcd_leveling_) {
    if(success) {
      settings.save();
      ExtUI::setLevelingActive(true);
    }
    return;
  }

  lcd_leveling_ = false;
  if(!success)
    return;

  status.reset();
  leveling_grid.show();
}

}
