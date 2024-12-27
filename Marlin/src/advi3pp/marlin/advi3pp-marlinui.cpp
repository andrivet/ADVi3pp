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

#include "../../inc/MarlinConfig.h"
#include "../../MarlinCore.h" // for printingIsPaused
#include "../../lcd/marlinui.h"
#include "../../lcd/extui/ui_api.h"
#include "../../gcode/queue.h"
#include "../../feature/host_actions.h"
#include "../core/buzzer.h"
#include "../core/dimming.h"
#include "../core/logging.h"
#include "src/module/planner.h"

// Simple and customized implementation of MarlinUI for ADVi3++
// The motivations for writing this implementation are:
// * ADVi3++ deviates slightly from Marlin, especially for Pause/Resume (it is possible to stop during a pause)
// * ADVi3++ uses only a small subset of this gigantic class. So it is possible to remove some fields and save memory.
// * Marlin's implementation is good for its own menu system, not so good for ADVi3++'s one
// * This small implementation is easier to understand and debug

using namespace ADVi3pp;

MarlinUI ui;

bool MarlinUI::status = false;
uint8_t MarlinUI::alert_level; // = 0
millis_t MarlinUI::status_message_expire_ms; // = 0
statusResetFunc_t MarlinUI::status_reset_callback; // = nullptr
MarlinUI::progress_t MarlinUI::progress_override; // = 0
uint32_t MarlinUI::remaining_time;
uint8_t MarlinUI::brightness = LCD_BRIGHTNESS_DEFAULT;
uint8_t MarlinUI::sound_on = SOUND_ON_DEFAULT;
preheat_t MarlinUI::material_preset[PREHEAT_COUNT];  // Initialized by settings.load
bool MarlinUI::lcd_clicked;
bool MarlinUI::sleep_timeout_enabled;
uint8_t MarlinUI::sleep_timeout_minutes; // Initialized by settings.load
uint8_t MarlinUI::sleep_timeout_brightness;
millis_t MarlinUI::screen_timeout_ms = 0;
uint16_t MarlinUI::tone_duration = TONE_DURATION_DEFAULT;


void MarlinUI::set_brightness(const uint8_t value) {
  brightness = constrain(value, LCD_BRIGHTNESS_MIN, LCD_BRIGHTNESS_MAX);
  Dimming::send_brightness_to_lcd();
}

void MarlinUI::set_tone(uint16_t duration, uint8_t options) {
  tone_duration = duration;
  sound_on = options;
}

void MarlinUI::apply_preheat(const uint8_t m, const uint8_t pmask, const uint8_t e/*=active_extruder*/) { }

void MarlinUI::refresh_screen_timeout() {
  screen_timeout_ms = sleep_timeout_minutes ? millis() + MIN_TO_MS(sleep_timeout_minutes) : 0;
  wake_display();
}

void MarlinUI::check_screen_timeout() {
  millis_t ms = millis();
  if (sleep_timeout_enabled && screen_timeout_ms && ELAPSED(ms, screen_timeout_ms)) // @advi3++
    sleep_display(true);
}

void MarlinUI::buzz(const long, const uint16_t) {
  Buzzer::buzz_on_action();
}

void MarlinUI::sleep_display(const bool sleep) {
  if(sleep) Dimming::sleep_on(); else Dimming::sleep_off();
}

void MarlinUI::completion_feedback(const bool good/*=true*/) {
  wake_display(); // Wake the screen for all audio feedback
  if (good) OKAY_BUZZ(); else ERR_BUZZ();
}

void MarlinUI::host_notify_P(PGM_P const pstr) { }
void MarlinUI::host_notify(const char * const cstr) { }

void MarlinUI::init() {
  ExtUI::onStartup();
}

void MarlinUI::reset_status(const bool no_welcome) {
  FSTR_P msg;
  if (printingIsPaused())
    msg = GET_TEXT_F(MSG_PRINT_PAUSED);
#if ENABLED(SDSUPPORT)
  else if (IS_SD_PRINTING())
    return set_status(card.longest_filename());
#endif
  else if (print_job_timer.isRunning())
    msg = GET_TEXT_F(MSG_PRINTING);
  else if (!no_welcome)
    msg = GET_TEXT_F(WELCOME_MSG);
  else
    return;

  set_min_status(msg);
}

bool MarlinUI::set_alert_level(int8_t &level) {
  if (level < 0) level = alert_level = 0;
  if (level < alert_level) return true;
  alert_level = level;
  return false;
}

void MarlinUI::_set_status_and_level(const char * const ustr, int8_t level, const bool pgm) {
  if (set_alert_level(level)) return;
  pgm ? host_notify_P(ustr) : host_notify(ustr);
  finish_status(ustr, pgm, level > 0);
}

void MarlinUI::_set_status(const char * const ustr, const bool persist, const bool pgm) {
  if (alert_level) return;
  pgm ? host_notify_P(ustr) : host_notify(ustr);
  finish_status(ustr, pgm, persist);
}

void MarlinUI::_set_alert(const char * const ustr, const int8_t level, const bool pgm) {
  pgm ? set_status_and_level_P(ustr, level) : set_status_and_level(ustr, level);
  wake_display();
}

void MarlinUI::status_printf_P(int8_t level, PGM_P const pfmt, ...) {
  if (set_alert_level(level)) return;

  MString<MAX_MESSAGE_SIZE> status_message;

  va_list args;
  va_start(args, pfmt);
  vsnprintf_P(status_message, MAX_MESSAGE_SIZE, pfmt, args);
  va_end(args);

  host_notify(status_message);

  finish_status(status_message, false, level > 0);
}

void MarlinUI::finish_status(const char* msg, const bool pgm, const bool persist) { // @advi3++
  set_status_reset_fn();
  status_message_expire_ms = persist ? 0 : millis() + (STATUS_MESSAGE_TIMEOUT_SEC) * 1000UL;
  pgm ? ExtUI::onStatusChanged(FPSTR(msg), persist) : ExtUI::onStatusChanged(msg, persist);
}

void MarlinUI::abort_print() {
  wait_for_heatup = false; wait_for_user = WAIT_FOR_USER::ABORT; // @advi3++
  did_pause_print = 0; // @advi3++
#if ENABLED(SDSUPPORT)
  if (IS_SD_PRINTING())
    card.abortFilePrintSoon();
  else if (card.isMounted())
    card.closefile();
#endif
  hostui.cancel();
  print_job_timer.stop();
  LCD_MESSAGE(MSG_PRINT_ABORTED);
}

void MarlinUI::pause_print() {
  wake_display();
  LCD_MESSAGE(MSG_PRINT_PAUSED);
  pause_show_message(PAUSE_MESSAGE_PARKING, PAUSE_MODE_PAUSE_PRINT); // Show message immediately to let user know about pause in progress
  queue.inject(F("M25 P")); // Do not resume here (like it is in Marlin's MarlinUI)
}

void MarlinUI::resume_print() {
  reset_status();
  wait_for_heatup = false; wait_for_user = WAIT_FOR_USER::CONTINUE; // @advi3++
#if ENABLED(SDSUPPORT)
  if (IS_SD_PAUSED()) queue.inject_P(M24_STR);
#endif
  hostui.resume();
  print_job_timer.start();
}

bool MarlinUI::is_printing_from_media_paused() {
  return IS_SD_PAUSED();
}

bool MarlinUI::is_printing_from_media() {
  return IS_SD_PRINTING() || IS_SD_PAUSED();
}

bool MarlinUI::is_printing() {
  /*SERIAL_ECHO_START();
  SERIAL_ECHO(
    ", is_printing_from_media: ", is_printing_from_media(),
    ", printJobOngoing: ", printJobOngoing(),
    ", printingIsPaused: ", printingIsPaused()
  );
  SERIAL_EOL();*/
  return is_printing_from_media() || printJobOngoing() || printingIsPaused();
}

bool MarlinUI::is_printing_paused() {
  return is_printing() && (is_printing_from_media_paused() || print_job_timer.isPaused());
}

bool MarlinUI::are_commands_in_queue() {
  return (planner.has_blocks_queued() || queue.has_commands_queued());
}

MarlinUI::progress_t MarlinUI::_get_progress() {
#if ENABLED(SDSUPPORT)
  return ((progress_override & PROGRESS_MASK) ?: card.percentDone());
#else
  return (progress_override & PROGRESS_MASK);
#endif
}

#if ENABLED(SDSUPPORT)
void MarlinUI::media_changed(const uint8_t old_status, const uint8_t new_status) {
  refresh_screen_timeout();
  if (old_status == new_status) {
    ExtUI::onMediaError(); // Failed to mount/unmount
    return;
  }

  if (old_status < 2) {   // Skip this section on first boot check
    if (new_status)           // Media Mounted
      ExtUI::onMediaMounted();
    else                  // Media Removed
      ExtUI::onMediaRemoved();
  }
}
#endif
