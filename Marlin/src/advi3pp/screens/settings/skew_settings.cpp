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
#include "skew_settings.h"
#include "../../core/core.h"
#include "../../core/dgus.h"

namespace ADVi3pp {

namespace {
  const unsigned DEFAULT_AC = 1415;
  const unsigned DEFAULT_BD = 1415;
  const unsigned DEFAULT_AD = 1000;
}

SkewSettings skew_settings;

//! Execute a command
//! @param key_value    The sub-action to handle
//! @return             True if the action was handled
bool SkewSettings::do_dispatch(KeyValue key_value) {
  if(Parent::do_dispatch(key_value))
    return true;

  switch(key_value)
  {
    case KeyValue::SkewStep2:  step2(); break;
    case KeyValue::SkewStep3:  step3(); break;
    default: return false;
  }

  return true;
}

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page SkewSettings::do_prepare_page() {
  pages.save_forward_page();
  step1();
  return Page::Skew1Settings;
}

void SkewSettings::set_default_values() {
  WriteRamRequest{Variable::Value0}.write_words(DEFAULT_AC, DEFAULT_BD, DEFAULT_AD);
}

void SkewSettings::step1() {
  set_default_values();
}

void SkewSettings::step2() {
  xy_ = get_factor();
  set_default_values();
  pages.show(Page::Skew2Settings);
}

void SkewSettings::step3() {
  xz_ = get_factor();
  set_default_values();
  pages.show(Page::Skew3Settings);
}

float SkewSettings::get_factor() {
  ReadRam response{Variable::Value0};
  if(!response.send_receive(3)) {
    Log::error() << F("Receiving Frame (Acceleration Settings)") << Log::endl();
    return 0;
  }

  float ac = response.read_word() / 10.0;
  float bd = response.read_word() / 10.0;
  float ad = response.read_word() / 10.0;

  return _SKEW_FACTOR(ac, bd, ad);
}

//! Save the settings
void SkewSettings::do_save_command() {
  float yz = get_factor();
  if(abs(xy_) < 0.000001) xy_ = 0;
  if(abs(xz_) < 0.000001) xz_ = 0;
  if(abs(yz)  < 0.000001) yz = 0;

  if(WITHIN(xy_, SKEW_FACTOR_MIN, SKEW_FACTOR_MAX) &&
     WITHIN(xz_, SKEW_FACTOR_MIN, SKEW_FACTOR_MAX) &&
     WITHIN(yz,  SKEW_FACTOR_MIN, SKEW_FACTOR_MAX)
   )
    ExtUI::setSkewFactors(xy_, xz_, yz);

  Parent::do_save_command();
}

}
