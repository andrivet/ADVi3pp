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
#include "../../core/string.h"
#include "../../screens/core/wait.h"
#include "grid.h"
#include "automatic.h"

namespace ADVi3pp {

AutomaticLeveling automatic_leveling;

bool AutomaticLeveling::do_dispatch(KeyValue key_value)
{
    if(Parent::do_dispatch(key_value))
        return true;

    switch(key_value)
    {
        case KeyValue::LevelingResetProbe:	reset_command(); break;
        default: return false;
    }

    return true;
}


//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page AutomaticLeveling::do_prepare_page()
{
#ifdef ADVi3PP_PROBE
    if(!core.ensure_not_printing())
        return Page::None;
    start();
    return Page::None;
#else
    return Page::NoSensor;
#endif
}

void AutomaticLeveling::reset_command()
{
#ifdef ADVi3PP_PROBE
    core.inject_commands(F("M280 P0 S160"));
#else
    pages.show(Page::NoSensor);
#endif
}

void AutomaticLeveling::start() {
  pages.save_forward_page();
  wait.wait(F("Homing..."));
  core.inject_commands(F("G28 O F6000"));
  background_task.set(Callback{this, &AutomaticLeveling::home_task}, 200);
}

//! Check if the printer is homed, and continue the Z Height Tuning process.
void AutomaticLeveling::home_task() {
  if(core.is_busy() || !ExtUI::isMachineHomed())
    return;

  background_task.clear();
  pages.show(Page::AutomaticLeveling);

  // homing, raise head, leveling, go back to corner, activate compensation
#ifdef ADVi3PP_PROBE
  core.inject_commands(F("G1 Z4 F1200\nG29 E\nG28 X Y F6000\nM420 S1"));
#else
  core.inject_commands(F("G1 Z4 F1200\nG29 S1\nG28 X Y F6000\nM420 S1"));
#endif
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
void AutomaticLeveling::on_done() {
  if(pages.get_current_page() != Page::AutomaticLeveling) {
    settings.save();
    ExtUI::setLevelingActive(true);
    return;
  }

  status.reset();
  leveling_grid.show();
}

}
