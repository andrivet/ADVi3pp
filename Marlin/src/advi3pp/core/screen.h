/**
 * ADVi3++ Firmware For Wanhao Duplicator i3 Plus (based on Marlin)
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

#include "../../lcd/extui/ui_api.h"
#include "../lib/ADVstd/ADVcrtp.h"
#include "../core/settings.h"
#include "../core/enums.h"
#include "../core/eeprom.h"
#include "../core/pages.h"

namespace ADVi3pp {

//! Handle inputs from the LCD Panel
template<typename Self>
struct Screen: adv::Crtp<Self, Screen> {
  void handle(KeyValue value);
  void show();

protected:
  bool on_dispatch(KeyValue value);
  bool on_enter();
  void on_save_command();
  void on_back_command();
  void on_abort();
};

// --------------------------------------------------------------------
// Screen implementation
// --------------------------------------------------------------------

template<typename Self>
void Screen<Self>::handle(KeyValue value) {
  if(!this->self().on_dispatch(value))
    Log::error() << F("Invalid key value ") << static_cast<uint16_t>(value) << Log::endl();
}

template<typename Self>
bool Screen<Self>::on_enter() { return true; }

template<typename Self>
bool Screen<Self>::on_dispatch(KeyValue value) {
  switch(value) {
    case KeyValue::Show: show(); break;
    case KeyValue::Abort: this->self().on_abort(); break;
    case KeyValue::Save:  this->self().on_save_command(); break;
    case KeyValue::Back:  this->self().on_back_command(); break;
    default: return false;
  }

  return true;
}

template<typename Self>
void Screen<Self>::show() {
  Page page = this->self().PAGE;
  if(!pages.check_no_print(page)) return;

  const bool show = this->self().on_enter();
  if(show && page != Page::None)
    pages.show(page, this->self().ACTION);
}

template<typename Self>
void Screen<Self>::on_save_command() {
    pages.save();
}

template<typename Self>
void Screen<Self>::on_back_command() {
    pages.back();
}

template<typename Self>
void Screen<Self>::on_abort() {}

}
