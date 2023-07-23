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
#include "change_temperature.h"
#include "../../core/core.h"
#include "../../core/dgus.h"
#include "../../core/wait.h"

namespace ADVi3pp {

  ChangeTemperature change_temperature;

  //! Handle Preheat actions.
  //! @param key_value    Sub-action to handle
  //! @return             True if the action was handled
  bool ChangeTemperature::on_dispatch(KeyValue key_value) {
    if(Parent::on_dispatch(key_value))
      return true;

    return false;
  }

  //! Prepare the page before being displayed and return the right Page value
  //! @return The index of the page to display
  bool ChangeTemperature::on_enter() {
    send_data();
    return true;
  }

  void ChangeTemperature::on_save_command() {
    Log::log() << F("resume_print_command") << Log::endl();

    ReadRam frame{Variable::Value0};
    if(!frame.send_receive(1)) {
      Log::error() << F("Receiving Frame (Target Temperature)") << Log::endl();
      return;
    }
    const auto temp = frame.read_word();
    ExtUI::setTargetTemp_celsius(temp, ExtUI::E0, true);
    ExtUI::setDefaultTemp_celsius(temp, ExtUI::E0);
    settings.save();

    ExtUI::setUserConfirmed();
    wait.wait();
  }

  void ChangeTemperature::send_data() {
    WriteRamRequest{Variable::Value0}.write_word(static_cast<uint16_t>(ExtUI::getTargetTemp_celsius(ExtUI::E0)));
  }

}

