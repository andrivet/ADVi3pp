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
#include "statistics.h"
#include "../../core/string.h"
#include "../../core/dgus.h"

namespace ADVi3pp {

Statistics statistics;

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
bool Statistics::on_enter() {
  send_stats();
  return true;
}

void Statistics::send_stats() {
  WriteRamRequest{Variable::Value0}.write_words(
    ExtUI::getTotalPrints(),
    ExtUI::getFinishedPrints(),
    freeMemory()
  );

  // Minimize the RAM used so send each value separately.
  char buffer[21];
  ADVString<16> value;

  value.set(ExtUI::getTotalPrintTime_str(buffer));
  WriteRamRequest{Variable::LongText0}.write_text(value);

  value.set(ExtUI::getLongestPrint_str(buffer));
  WriteRamRequest{Variable::LongText1}.write_text(value);

  value.set(ExtUI::getFilamentUsed_str(buffer));
  WriteRamRequest{Variable::LongText2}.write_text(value);
}

}
