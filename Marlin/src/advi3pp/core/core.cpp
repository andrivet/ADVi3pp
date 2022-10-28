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

#include "../parameters.h"
#include "../versions.h"
#include "core.h"
#include "graphs.h"
#include "dimming.h"
#include "status.h"
#include "pages.h"
#include "task.h"
#include "dgus.h"
#include "buzzer.h"
#include "reentrant.h"
#include "../screens/core/no_sensor.h"
#include "../screens/controls/controls.h"
#include "../screens/core/wait.h"
#include "../screens/controls/load_unload.h"
#include "../screens/controls/preheat.h"
#include "../screens/controls/move.h"
#include "../screens/leveling/manual.h"
#include "../screens/leveling/automatic.h"
#include "../screens/leveling/grid.h"
#include "../screens/print/print.h"
#include "../screens/print/pause_options.h"
#include "../screens/print/sd_card.h"
#include "../screens/print/temperatures.h"
#include "../screens/tuning/extruder_tuning.h"
#include "../screens/tuning/pid_tuning.h"
#include "../screens/tuning/bltouch_testing.h"
#include "../screens/leveling/z_height.h"
#include "../screens/tuning/io.h"
#include "../screens/tuning/setup.h"
#include "../screens/leveling/xtwist.h"
#include "../screens/tuning/vibrations.h"
#include "../screens/settings/eeprom_mismatch.h"
#include "../screens/settings/factory_reset.h"
#include "../screens/settings/sensor_settings.h"
#include "../screens/settings/lcd_settings.h"
#include "../screens/settings/beeper_settings.h"
#include "../screens/print/print_settings.h"
#include "../screens/print/baby_steps.h"
#include "../screens/settings/pid_settings.h"
#include "../screens/settings/step_settings.h"
#include "../screens/settings/feedrate_settings.h"
#include "../screens/settings/acceleration_settings.h"
#include "../screens/settings/linear_advance_settings.h"
#include "../screens/settings/runout_settings.h"
#include "../screens/settings/skew_settings.h"
#include "../screens/info/versions.h"
#include "../screens/info/statistics.h"
#include "../screens/info/copyrights.h"
#include "../screens/info/killed.h"

namespace ADVi3pp {

static constexpr unsigned int FROM_LCD_DELAY = 0; // ms
static constexpr unsigned int TO_LCD_DELAY = 250; // ms
static constexpr float Z_ROOM = 20; // mm
static constexpr unsigned int ANTI_BOUNCE_DELAY = 20; // ms

Core core;

Task background_task;
Task from_lcd_task;
Task to_lcd_task;

// ----------------------------------------------------------------------------

Once::operator bool() {
  if(!once_)
    return false;
  once_ = false;
  return true;
}

// ----------------------------------------------------------------------------

void Core::startup() {
#ifdef ADVi3PP_DEBUG
  pinMode(LED_PIN, OUTPUT); // To help debugging when serial is not available
#endif
  dgus.open();
}

bool Core::init() {
  if(!once_)
    return false;

  dgus.setup();
  status.set(GET_TEXT_F(WELCOME_MSG)); // This welcome message is not displayed on power-on
  send_gplv3_7b_notice(); // You are not authorized to remove or alter this notice
  graphs.clear();
  versions.send_versions();

#if HAS_LEVELING
  ExtUI::setLevelingActive(true);
#endif

  from_lcd_task.set(Callback{this, &Core::from_lcd}, FROM_LCD_DELAY);
  to_lcd_task.set(Callback{this, &Core::to_lcd}, TO_LCD_DELAY);

  if(settings.does_eeprom_mismatch())
    eeprom_mismatch.show();
  else
    pages.show(Page::Boot, Action::None);

  return true;
}

void Core::idle() {
  static Reentrant reentrant;
  ReentrantScope scope{reentrant};

  if(!scope.reentrant()) {
    init();
    from_lcd_task.execute();
  }

  to_lcd_task.execute();

  if(!scope.reentrant())
    background_task.execute();
}

void Core::to_lcd() {
  update_progress();
  send_lcd_data();
  graphs.update();
  send_lcd_touch_request();
}

void Core::killed(float temp, const FlashChar* error, const FlashChar* component) {
  status.set(error);
  send_lcd_data();
  dimming.sleep_off();
  killed_page.show(temp, component);
}

//! Note to forks author:
//! Under GPLv3 provision 7(b), you are not authorized to remove or alter this notice.
void Core::send_gplv3_7b_notice() {
  SERIAL_ECHO_START();
  SERIAL_ECHOLNPGM("Based on ADVi3++, Copyright (C) 2017-2022 Sebastien Andrivet");
}

//! Update the progress bar if the printer is printing for the SD card
void Core::update_progress() {
    // TODO Not sure it is necessary
}

//! Read a frame from the LCD and act accordingly.
void Core::from_lcd() {
  if(dimming.receive())
    return;

  ReadAction frame{};
  if(!frame.receive())
    return;

  buzzer.buzz_on_press();
  ui.refresh_screen_timeout();

  Action action = frame.get_parameter();
  auto key_code = frame.read_key_value();
  millis_t now = millis();
  bool bounce = action == last_action_ && (now - last_action_time_) < ANTI_BOUNCE_DELAY;
  last_action_ = action;
  last_action_time_ = now;

  if(bounce) {
    Log::log() << "Bounce detected" << Log::endl();
    return;
  }

  Log::frame(LogState::Start) << F("=R==> Action =") << static_cast<uint16_t>(action)
    << F("KeyValue =") << static_cast<uint16_t>(key_code) << Log::endl();

   process_action(action, key_code);
}

void Core::process_action(Action action, KeyValue key_code) {
  uint16_t raw_value = static_cast<int16_t>(key_code);

  switch(action) {
    case controls.ACTION:                 controls.handle(key_code); break;
    case print.ACTION:                    print.handle(key_code); break;
    case wait.ACTION:                     wait.handle(key_code); break;
    case load_unload.ACTION:              load_unload.handle(key_code); break;
    case preheat.ACTION:                  preheat.handle(key_code); break;
    case move.ACTION:                     move.handle(key_code); break;
    case sd_card.ACTION:                  sd_card.handle(key_code); break;
    case factory_reset.ACTION:            factory_reset.handle(key_code); break;
    case manual_leveling.ACTION:          manual_leveling.handle(key_code); break;
    case extruder_tuning.ACTION:          extruder_tuning.handle(key_code); break;
    case pid_tuning.ACTION:               pid_tuning.handle(key_code); break;
    case sensor_settings.ACTION:          sensor_settings.handle(key_code); break;
    case vibrations.ACTION:               vibrations.handle(key_code); break;
    case no_sensor.ACTION:                no_sensor.handle(key_code); break;
    case lcd_settings.ACTION:             lcd_settings.handle(key_code); break;
    case statistics.ACTION:               statistics.handle(key_code); break;
    case versions.ACTION:                 versions.handle(key_code); break;
    case print_settings.ACTION:           print_settings.handle(key_code); break;
    case baby_steps.ACTION:               baby_steps.handle(key_code); break;
    case pid_settings.ACTION:             pid_settings.handle(key_code); break;
    case steps_settings.ACTION:           steps_settings.handle(key_code); break;
    case feedrates_settings.ACTION:       feedrates_settings.handle(key_code); break;
    case accelerations_settings.ACTION:   accelerations_settings.handle(key_code); break;
    case pause_options.ACTION:            pause_options.handle(key_code); break;
    case copyrights.ACTION:               copyrights.handle(key_code); break;
    case automatic_leveling.ACTION:       automatic_leveling.handle(key_code); break;
    case leveling_grid.ACTION:            leveling_grid.handle(key_code); break;
    case sensor_z_height.ACTION:          sensor_z_height.handle(key_code); break;
    case eeprom_mismatch.ACTION:          eeprom_mismatch.handle(key_code); break;
    case bltouch_testing.ACTION:          bltouch_testing.handle(key_code); break;
    case linear_advance_settings.ACTION:  linear_advance_settings.handle(key_code); break;
    case io.ACTION:                       io.handle(key_code); break;
    case temperatures.ACTION:             temperatures.handle(key_code); break;
    case setup.ACTION:                    setup.handle(key_code); break;
    case xtwist.ACTION:                   xtwist.handle(key_code); break;
    case runout_settings.ACTION:          runout_settings.handle(key_code); break;
    case skew_settings.ACTION:            skew_settings.handle(key_code); break;
    case beeper_settings.ACTION:          beeper_settings.handle(key_code); break;

    case Action::MoveXPlus:               move.x_plus_command(); break;
    case Action::MoveXMinus:              move.x_minus_command(); break;
    case Action::MoveYPlus:               move.y_plus_command(); break;
    case Action::MoveYMinus:              move.y_minus_command(); break;
    case Action::MoveZPlus:               move.z_plus_command(); break;
    case Action::MoveZMinus:              move.z_minus_command(); break;
    case Action::MoveEPlus:               move.e_plus_command(); break;
    case Action::MoveEMinus:              move.e_minus_command(); break;
    case Action::BabyMinus:               baby_steps.baby_minus_command(); break;
    case Action::BabyPlus:                baby_steps.baby_plus_command(); break;
    case Action::ZHeightMinus:            sensor_z_height.minus(); break;
    case Action::ZHeightPlus:             sensor_z_height.plus(); break;
    case Action::FeedrateMinus:           print_settings.feedrate_minus_command(); break;
    case Action::FeedratePlus:            print_settings.feedrate_plus_command(); break;
    case Action::FlowrateMinus:           print_settings.flowrate_minus_command(); break;
    case Action::FlowratePlus:            print_settings.flowrate_plus_command(); break;
    case Action::FanMinus:                print_settings.fan_minus_command(); break;
    case Action::FanPlus:                 print_settings.fan_plus_command(); break;
    case Action::HotendMinus:             print_settings.hotend_minus_command(); break;
    case Action::HotendPlus:              print_settings.hotend_plus_command(); break;
    case Action::BedMinus:                print_settings.bed_minus_command(); break;
    case Action::BedPlus:                 print_settings.bed_plus_command(); break;
    case Action::XTwistMinus:             xtwist.minus(); break;
    case Action::XTwistPlus:              xtwist.plus(); break;
    case Action::BeepDuration:            beeper_settings.duration_command(raw_value); break;
    case Action::NormalBrightness:        lcd_settings.normal_brightness_command(raw_value); break;
    case Action::DimmingBrightness:       lcd_settings.dimming_brightness_command(raw_value); break;

    default:                              Log::error() << F("Invalid action ") << static_cast<uint16_t>(action) << Log::endl(); break;
  }
}

void Core::send_lcd_touch_request() {
  NoFrameLogging no_log{};
  dimming.send();
}

//! Update the status of the printer on the LCD.
void Core::send_lcd_data() {
  // The progress bar is split into two parts because of a limitation of the DWIN panel
  // so compute the progress of each part.
  int16_t progress_bar_low  = ExtUI::getProgress_percent() >= 50 ? 10 : ExtUI::getProgress_percent() / 5;
  int16_t progress_var_high = ExtUI::getProgress_percent() < 50 ? 0 : (ExtUI::getProgress_percent() - 50) / 5;

#ifdef ADVi3PP_PROBE
  uint16_t probe_state = ExtUI::getLevelingActive() ? 2 : 1;
#else
  uint16_t probe_state = 0;
#endif

  NoFrameLogging no_logging{};
  // Send the current status in one frame
  WriteRamRequest{Variable::TargetBed}.write_words(
    ExtUI::getTargetTemp_celsius(ExtUI::BED),
    ExtUI::getActualTemp_celsius(ExtUI::BED),
    ExtUI::getTargetTemp_celsius(ExtUI::E0),
    ExtUI::getActualTemp_celsius(ExtUI::E0),
    ExtUI::getActualFan_percent(ExtUI::FAN0),
    lround(ExtUI::getAxisPosition_mm(ExtUI::Z) * 100.0),
    progress_bar_low,
    progress_var_high,
    0, // Reserved
    probe_state,
    ExtUI::getFeedrate_percent(),
    ExtUI::getFlow_percent(ExtUI::E0)
  );

  status.send();
}

bool Core::is_busy()  {
  return
    ExtUI::isMoving() ||
    ExtUI::commandsInQueue() ||
    ExtUI::getHostKeepaliveState() == GcodeSuite::MarlinBusyState::IN_HANDLER ||
    ExtUI::getHostKeepaliveState() == GcodeSuite::MarlinBusyState::IN_PROCESS;
}

void Core::inject_commands(const FlashChar* commands) {
  ExtUI::injectCommands_P(from_flash(commands));
}

void Core::inject_commands(const char *commands) {
  ExtUI::injectCommands(commands);
}

//! Get current digital pin state (adapted from Arduino source code).
//! @param pin  Pin number to check.
//! @return     The current state: On (input), Off (input), Output
Core::PinState Core::get_pin_state(uint8_t pin) {
  uint8_t mask = digitalPinToBitMask(pin);
  uint8_t port = digitalPinToPort(pin);
  if(port == NOT_A_PIN)
    return PinState::Off;

  volatile uint8_t* reg = portModeRegister(port);
  if(*reg & mask)
    return PinState::Output;

  uint8_t timer = digitalPinToTimer(pin);
  if(timer != NOT_ON_TIMER)
    return PinState::Output;

  return (*portInputRegister(port) & mask) ? PinState::On : PinState::Off;
}

float Core::ensure_z_enough_room() {
  auto previous_z = ExtUI::getAxisPosition_mm(ExtUI::Z);
  if(previous_z < 10)
    ExtUI::setAxisPosition_mm(Z_ROOM, ExtUI::Z, 20);
  return previous_z;
}

void Core::media_inserted() {
  status.set(F("SD card detected."));
  if(pages.get_current_page() != Page::SdCard)
    return;
  sd_card.on_media_inserted();
}

void Core::media_removed() {
  status.set(F("SD card removed."));
  if(pages.get_current_page() != Page::SdCard)
    return;
  sd_card.on_media_removed();
}

void Core::media_error() {
  status.set(F("Error reading SD card."));
  if(pages.get_current_page() != Page::SdCard)
    return;
  sd_card.on_media_error();
}


}
