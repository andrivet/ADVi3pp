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

#pragma once

#include "../../inc/MarlinConfig.h"

#if ENABLED(ADVANCED_PAUSE_FEATURE)
#include "../../feature/pause.h"
#include "../../module/motion.h" // for active_extruder
#endif

typedef struct {
    uint16_t hotend_temp;
    uint16_t bed_temp;
    uint16_t fan_speed;
} preheat_t;


class MarlinUI {
public:

    MarlinUI() = default;

    static void init();
    static void update();

    static void abort_print();
    static void pause_print();
    static void resume_print();
    static void synchronize(FSTR_P msg=nullptr);
    static void kill_screen(FSTR_P lcd_error, FSTR_P lcd_component);

    static bool has_status();
    static void reset_status(bool no_welcome=false);
    static void set_alert_status(FSTR_P fstr);
    static void reset_alert_level();
    static void set_status(const char *str, bool persist = true);
    static void set_status(FSTR_P fstr, int8_t level=0);
    static void status_printf(uint8_t level, FSTR_P fmt, ...);
    static void finish_status(bool persist);
    static void return_to_status(bool show_main = false);

    static void set_brightness(int16_t value);
    static int16_t get_brightness();

    static void update_buttons();
    static bool button_pressed();
    static void quick_feedback(bool clear_buttons=true);
    static void refresh();
    static void wait_for_release();
    static bool use_click();

    static void capture();
    static void release();
    static void chirp();

    static preheat_t material_preset[PREHEAT_COUNT];
    static FSTR_P get_preheat_label(uint8_t m);

    typedef uint8_t progress_t;
    static void set_progress(progress_t p);
    static void set_progress_done();
    static uint8_t get_progress_percent();
    static void buzz(long duration, uint16_t freq);

    static void pause_show_message(PauseMessage message, PauseMode mode=PAUSE_MODE_SAME, uint8_t extruder=active_extruder);

#if LCD_HAS_WAIT_FOR_MOVE
    static bool wait_for_move;
#else
    static constexpr bool wait_for_move = true;
#endif

    // There is no way to detect media changes, so this part is not implemented
    static inline bool detected() { return true; }
    static inline void media_changed(const uint8_t old_stat, const uint8_t stat) {}

    static bool lcd_clicked;
};

extern MarlinUI ui;

#define LCD_MESSAGE_F(S)       ui.set_status(F(S))
#define LCD_MESSAGE(M)         ui.set_status(GET_TEXT_F(M))
#define LCD_ALERTMESSAGE_F(S)  ui.set_alert_status(F(S))
#define LCD_ALERTMESSAGE(M)    ui.set_alert_status(GET_TEXT_F(M))
