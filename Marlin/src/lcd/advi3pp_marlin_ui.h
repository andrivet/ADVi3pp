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

#if ENABLED(ADVANCED_PAUSE_FEATURE)
#include "../feature/pause.h"
#include "../module/motion.h" // for active_extruder
#endif

#if ENABLED(ADVANCED_PAUSE_FEATURE)
void lcd_pause_show_message(const PauseMessage message,
                            const PauseMode mode=PAUSE_MODE_SAME,
                            const uint8_t extruder=active_extruder);
#endif


class MarlinUI {
public:

    MarlinUI() {}

    static void init();
    static void update();

    static void abort_print();
    static void pause_print();
    static void resume_print();
    static void kill_screen(PGM_P const lcd_error, PGM_P const lcd_component);

    static void set_alert_status_P(PGM_P const message);
    static void reset_alert_level();
    static bool has_status();
    static void set_status(const char* const message, const bool persist=false);
    static void set_status_P(PGM_P const message, const int8_t level=0);
    static void status_printf_P(const uint8_t level, PGM_P const fmt, ...);
    static void reset_status(const bool no_welcome=false);
    static void return_to_status();

    static void set_contrast(const int16_t value);
    static int16_t get_contrast();

    static void update_buttons();
    static bool button_pressed();
    static void quick_feedback(const bool clear_buttons=true);
    static void refresh();
    static void wait_for_release();
    static bool use_click();

    static bool external_control;
    FORCE_INLINE static void capture() { external_control = true; }
    FORCE_INLINE static void release() { external_control = false; }
    static void chirp();

    static int16_t preheat_hotend_temp[NB_MATERIAL_PRESET], preheat_bed_temp[NB_MATERIAL_PRESET];
    static uint8_t preheat_fan_speed[NB_MATERIAL_PRESET];

    typedef uint8_t progress_t;
#define PROGRESS_SCALE 1U
    static progress_t progress_override;
    static void set_progress(const progress_t p);
    static void set_progress_done();
    static uint8_t get_progress_percent();

    static void finish_status(const bool persist);
    static void synchronize(PGM_P const msg=nullptr);
};

extern MarlinUI ui;

#define LCD_MESSAGEPGM_P(x)      ui.set_status_P(x)
#define LCD_ALERTMESSAGEPGM_P(x) ui.set_alert_status_P(x)

#define LCD_MESSAGEPGM(x)        LCD_MESSAGEPGM_P(GET_TEXT(x))
#define LCD_ALERTMESSAGEPGM(x)   LCD_ALERTMESSAGEPGM_P(GET_TEXT(x))
