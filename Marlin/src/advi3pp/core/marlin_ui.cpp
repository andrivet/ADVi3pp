/**
 * ADVi3++ Firmware For Wanhao Duplicator i3 Plus (based on Marlin 2)
 *
 * Copyright (C) 2017-2020 Sebastien Andrivet [https://github.com/andrivet/]
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

// Minimal implementation of MarlinUI for ADVi3++

#include "../parameters.h"
#include "logging.h"
#include "status.h"
#include "buzzer.h"
#include "../screens/print/pause.h"
#include "../screens/core/wait.h"

#include "../../lcd/ultralcd.h"
#include "../../module/printcounter.h"
#include "../../MarlinCore.h"
#include "../../sd/cardreader.h"
#include "../../gcode/queue.h"
#include "../../module/planner.h"

using namespace ADVi3pp;

preheat_t MarlinUI::material_preset[PREHEAT_COUNT];  // Initialized by settings.load()

extern PauseMode pause_mode;

namespace
{
    bool external_control = false;
    uint8_t progress = 0;
    uint8_t alert_level = 0;
    int16_t contrast = LCD_CONTRAST_INIT;
}


void lcd_pause_show_message(const PauseMessage message, const PauseMode mode, const uint8_t /*extruder*/)
{
    Log::log() << F("lcd_pause_show_message(")
        << static_cast<uint16_t>(message)
        << F(", ")
        << static_cast<uint16_t>(mode)
        << F(")")
        << Log::endl();
    if (mode != PAUSE_MODE_SAME) pause_mode = mode;
    pause.show_message(message);
}

void MarlinUI::return_to_status()
{
    Log::log() << F("return_to_status") << Log::endl();
    pages.reset();
    if(print_job_timer.isRunning())
        pages.show_page(Page::Print, ShowOptions::SaveBack);
    else
        pages.show_page(Page::Main, ShowOptions::None);
}

void MarlinUI::reset_alert_level()
{
    Log::log() << F("reset_alert_level") << Log::endl();
    alert_level = 0;
}

void MarlinUI::finish_status(const bool persist)
{
    // Nothing to do
}

bool MarlinUI::has_status()
{
    Log::log() << F("has_status") << Log::endl();
    return status.has();
}

void MarlinUI::set_status(const char* const message, const bool persist)
{
    Log::log() << F("set_status") << Log::endl();
    if(alert_level) return;
    status.set(message);
    finish_status(persist);
}

void MarlinUI::status_printf_P(const uint8_t level, PGM_P const fmt, ...)
{
    Log::log() << F("status_printf_P") << Log::endl();

    if (level < alert_level) return;
    alert_level = level;

    va_list args;
    va_start(args, fmt);
    status.set(reinterpret_cast<const FlashChar*>(fmt), args);
    va_end(args);

    finish_status(level > 0);
}

void MarlinUI::set_status_P(PGM_P const message, int8_t level)
{
    Log::log() << F("set_status_P") << Log::endl();

    if (level < 0) level = alert_level = 0;
    if (level < alert_level) return;
    alert_level = level;

    status.set(reinterpret_cast<const FlashChar*>(message));
    finish_status(level > 0);
}

void MarlinUI::set_alert_status_P(PGM_P const message)
{
    Log::log() << F("set_alert_status_P") << Log::endl();
    set_status_P(message, 1);
    return_to_status();
}

void MarlinUI::reset_status(const bool no_welcome)
{
    Log::log() << F("reset_status") << Log::endl();

    PGM_P msg;
    if(printingIsPaused())
        msg = GET_TEXT(MSG_PRINT_PAUSED);
    else if (IS_SD_PRINTING())
        msg = GET_TEXT(MSG_PRINTING);
    else if (print_job_timer.isRunning())
        msg = GET_TEXT(MSG_PRINTING);
    else if (!no_welcome)
        msg = GET_TEXT(WELCOME_MSG);
    else
        return;

    set_status_P(msg, -1);
}

void MarlinUI::abort_print()
{
    Log::log() << F("abort_print") << Log::endl();

#if ENABLED(SDSUPPORT)
    wait_for_heatup = wait_for_user = false;
    card.flag.abort_sd_printing = true;
#endif
#ifdef ACTION_ON_CANCEL
    host_action_cancel();
#endif
#if ENABLED(HOST_PROMPT_SUPPORT)
    host_prompt_open(PROMPT_INFO, PSTR("UI Aborted"), DISMISS_STR);
#endif
    print_job_timer.stop();
    set_status_P(GET_TEXT(MSG_PRINT_ABORTED));
#if HAS_LCD_MENU
    return_to_status();
#endif
}

void MarlinUI::pause_print() {
#if HAS_LCD_MENU
    synchronize(GET_TEXT(MSG_PAUSE_PRINT));
#endif

#if ENABLED(HOST_PROMPT_SUPPORT)
    host_prompt_open(PROMPT_PAUSE_RESUME, PSTR("UI Pause"), PSTR("Resume"));
#endif

    set_status_P(GET_TEXT(MSG_PRINT_PAUSED));

#if ENABLED(PARK_HEAD_ON_PAUSE)
    queue.inject_P(PSTR("M25 P\nM24"));
#elif ENABLED(SDSUPPORT)
    queue.inject_P(PSTR("M25"));
#elif defined(ACTION_ON_PAUSE)
      host_action_pause();
#endif
}

void MarlinUI::resume_print()
{
    reset_status();
#if ENABLED(PARK_HEAD_ON_PAUSE)
    wait_for_heatup = wait_for_user = false;
#endif
    if (IS_SD_PAUSED()) queue.inject_P(M24_STR);
#ifdef ACTION_ON_RESUME
    host_action_resume();
#endif
    print_job_timer.start(); // Also called by M24
}

void MarlinUI::synchronize(PGM_P msg)
{
    static bool no_reentry = false;
    if(no_reentry) return;

    if(msg == nullptr)
        msg = GET_TEXT(MSG_MOVING);
    wait.show(reinterpret_cast<const FlashChar*>(msg));

    planner.synchronize(); // idle() is called until moves complete
    no_reentry = false;

    pages.show_back_page();
}

void MarlinUI::set_contrast(const int16_t value)
{
    contrast = value;
}

int16_t MarlinUI::get_contrast()
{
    return contrast;
}

void MarlinUI::update_buttons()
{
    // Nothing to do here, UI updates are made within idle()
}

bool MarlinUI::button_pressed()
{
    Log::log() << F("button_pressed") << Log::endl();
    // TODO
    return false;
}

void MarlinUI::quick_feedback(const bool /*clear_buttons*/)
{
    Log::log() << F("quick_feedback") << Log::endl();

    refresh();
    buzzer.buzz_on_action();
}

void MarlinUI::refresh()
{
    Log::log() << F("refresh") << Log::endl();
    // Nothing to do
}

void MarlinUI::wait_for_release()
{
    Log::log() << F("wait_for_release") << Log::endl();
    safe_delay(50);
}

bool MarlinUI::use_click()
{
    Log::log() << F("use_click") << Log::endl();
    // TODO
    return false;
}

void MarlinUI::capture()
{
    Log::log() << F("capture") << Log::endl();
    external_control = true;
}

void MarlinUI::release()
{
    Log::log() << F("release") << Log::endl();
    external_control = false;
}

void MarlinUI::chirp()
{
    Log::log() << F("chirp") << Log::endl();
    buzzer.buzz_on_action();
}

void MarlinUI::set_progress(const progress_t p)
{
    progress = p;
}

void MarlinUI::set_progress_done()
{
    Log::log() << F("set_progress_done") << Log::endl();
    progress = 100;
}

uint8_t MarlinUI::get_progress_percent()
{
    return card.isPrinting() ? card.percentDone() : progress;
}

void MarlinUI::media_changed(const uint8_t old_status, const uint8_t status)
{
    if (old_status == status) {
        TERN_(EXTENSIBLE_UI, ExtUI::onMediaError()); // Failed to mount/unmount
        return;
    }

    if (status) {
        if (old_status < 2) {
            ExtUI::onMediaInserted(); // ExtUI response
            set_status_P(GET_TEXT(MSG_MEDIA_INSERTED));
        }
    }
    else {
        if (old_status < 2) {
            ExtUI::onMediaRemoved(); // ExtUI response
        }
    }
}
