/**
 * Marlin 3D Printer Firmware For Wanhao Duplicator i3 Plus (ADVi3++)
 *
 * Copyright (C) 2017 Sebastien Andrivet [https://github.com/andrivet/]
 *
 * Copyright (C) 2016 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Based on Sprinter and grbl.
 * Copyright (C) 2011 Camiel Gubbels / Erik van der Zalm
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
#ifndef ADV_I3_PLUS_PLUS_PRIVATE_H
#define ADV_I3_PLUS_PLUS_PRIVATE_H

#include "advi3pp_enums.h"
#include "advi3pp.h"
#include "advi3pp_utils.h"

namespace advi3pp { inline namespace internals {

enum class BackgroundTask: uint8_t
{
    None                = 0,
    Leveling            = 1,
    LoadFilament        = 2,
    UnloadFilament      = 3,
    ExtruderCalibration = 4,
    Undefined           = 99
};

// --------------------------------------------------------------------
// Preset
// --------------------------------------------------------------------

//! Hostend and bad temperature preset.
struct Preset
{
    uint16_t hotend;
    uint16_t bed;
};

// --------------------------------------------------------------------
// PidpSettings
// --------------------------------------------------------------------

struct PidSettings
{
    void init();
    void save();

    float Kp, Ki, Kd;
};

// --------------------------------------------------------------------
// StepSettings
// --------------------------------------------------------------------

struct StepSettings
{
    void init();
    void save();

    float axis_steps_per_mm[XYZE_N];
};

// --------------------------------------------------------------------
// FeedrateSettings
// --------------------------------------------------------------------

struct FeedrateSettings
{
    void init();
    void save();

    float max_feedrate_mm_s[XYZE_N];
    float min_feedrate_mm_s;
    float min_travel_feedrate_mm_s;
};

// --------------------------------------------------------------------
// AccelerationSettings
// --------------------------------------------------------------------

struct AccelerationSettings
{
    void init();
    void save();

    uint32_t max_acceleration_mm_per_s2[XYZE_N];
    float acceleration;
    float retract_acceleration;
    float travel_acceleration;
};

// --------------------------------------------------------------------
// JerkSettings
// --------------------------------------------------------------------

struct JerkSettings
{
    void init();
    void save();

    float max_jerk[XYZE];
};

// --------------------------------------------------------------------
// PrinterImpl
// --------------------------------------------------------------------

//! Implementation of the Duplication i3 Plus printer
struct PrinterImpl
{
    void setup();
    void task();
    void show_page(Page page, bool save_back = true);
    void auto_pid_finished();
    void store_presets(eeprom_write write, int& eeprom_index, uint16_t& working_crc);
    void restore_presets(eeprom_read read, int& eeprom_index, uint16_t& working_crc);
    void reset_presets();
    void temperature_error();
    void send_temperatures_data();

private:
    void clear_graphs();
    void send_versions();
    void execute_background_task();
    void send_status();
    Page get_current_page();
    void show_back_page();
    void save_forward_page();
    void show_forward_page();
    void read_lcd_serial();
    void send_stats();
    void show_sd_files(uint16_t last_index);
    void get_file_name(uint8_t index, String& name);
    String get_lcd_firmware_version();
    void set_next_background_task_time(unsigned int delta = 500);
    void set_next_update_time(unsigned int delta = 500);
    void set_background_task(BackgroundTask task, unsigned int delta = 500);
    void clear_background_task();
    void send_graphs_data();
    void set_update_graphs();
    void update_graphs();
    void set_target_temperature(uint16_t temperature);
    uint16_t get_target_temperature();
    bool is_version_valid() const;

private:
    // Actions
    void main(KeyValue key_value);
    void sd_print_command(KeyValue key_value);
    void usb_print_command(KeyValue key_value);
    void load_unload(KeyValue key_value);
    void preheat(KeyValue key_value);
    void cooldown();
    void move(KeyValue key_value);
    void disable_motors();
    void sd_card(KeyValue key_value);
    void sd_card_select_file(KeyValue key_value);
    void show_settings(KeyValue key_value);
    void save_settings(KeyValue key_value);
    void cancel_settings(KeyValue key_value);
    void factory_reset(KeyValue key_value);
    void leveling(KeyValue key_value);
    void extruder_calibration(KeyValue key_value);
    void xyz_motors_calibration(KeyValue key_value);
    void pid_tuning(KeyValue key_value);
    void statistics(KeyValue key_value);
    void about(KeyValue key_value);

    // Sub-actions
    void main_temps();
    void main_sd();
    void main_controls();
    void main_calibrations();
    void main_settings();
    void main_motors();
    void back();
    void sd_print_stop();
    void sd_print_pause();
    void sd_print_resume();
    void sd_print_back();
    void usb_print_stop();
    void usb_print_pause();
    void usb_print_resume();
    void usb_print_back();
    void load_unload_show();
    void load_unload_start(bool load);
    void load_unload_stop();
    void preheat_show();
    void preheat_back();
    void preheat_preset(uint16_t presetIndex);
    void show_move();
    void move_x_plus();
    void move_x_minus();
    void move_y_plus();
    void move_y_minus();
    void move_z_plus();
    void move_z_minus();
    void move_e_plus();
    void move_e_minus();
    void home_x();
    void home_y();
    void home_z();
    void home_all();
    void move_back();
    void show_print_settings();
    void show_pid_settings(bool back, bool init = true);
    void show_steps_settings(bool init = true);
    void show_feedrate_settings(bool init = true);
    void show_acceleration_settings(bool init = true);
    void show_jerk_settings(bool init = true);
    void save_print_settings();
    void save_pid_settings();
    void save_steps_settings();
    void save_feedrate_settings();
    void save_acceleration_settings();
    void save_jerk_settings();
    void show_stats();
    void stats_back();
    void pid_tuning_step1();
    void pid_tuning_step2();
    void pid_tuning_cancel();
    void leveling_home();
    void leveling_point1();
    void leveling_point2();
    void leveling_point3();
    void leveling_point4();
    void leveling_point5();
    void leveling_finish();
    void show_extruder_calibration();
    void start_extruder_calibration();
    void extruder_calibration_finished();
    void extruder_calibrartion_settings();
    void cancel_extruder_calibration();
    void show_xyz_motors_calibration();
    void cancel_xyz_motors_calibration();
    void xyz_motors_calibration_settings();
    void show_factory_reset_warning();
    void do_factory_reset();
    void cancel_factory_reset();
    void about_forward();
    void about_back();
    void show_about(uint16_t version);

    // Background tasks
    void load_filament_task();
    void unload_filament_task();
    void leveling_task();
    void extruder_calibration_task();

private:
    static const size_t NB_PRESETS = 3;

    uint16_t last_file_index_ = 0;
    millis_t next_op_time_ = 0;
    millis_t next_update_time_ = 0;
    BackgroundTask background_task_ = BackgroundTask::None;
    bool update_graphs_ = false;
    millis_t next_update_graph_time_ = 0;
    Stack<Page, 8> back_pages_;
    Page forward_page_ = Page::None;
    Preset presets_[NB_PRESETS] = { {0, 0}, {0, 0}, {0, 0} };
    PidSettings old_pid_;
    StepSettings steps_;
    FeedrateSettings feedrates_;
    AccelerationSettings accelerations_;
    JerkSettings jerks_;
    uint16_t adv_i3_pp_lcd_version_ = 0x0000;
    double extruded_ = 0.0;
};

// --------------------------------------------------------------------
// LCDImpl
// --------------------------------------------------------------------

//! Implementation of the Duplication i3 Plus LCD
struct LCDImpl
{
    static LCDImpl& instance();

    void update();
    void init();
    bool has_status();
    void set_status(const char* message);
    void set_status_PGM(const char* message);
    void set_alert_status_PGM(const char* message);
    void status_printf_P(const char* fmt, va_list argp);
    void buttons_update();
    void reset_alert_level();
    bool detected();
    void refresh();
    const String& get_message() const;
    void queue_message(const String& message);
    void reset_messaage();

    void set_progress_name(const String& name);
    const String& get_progress() const;
    void reset_progress();

private:
    String message_;
    String progress_name_;
    mutable String progress_percent_;
    mutable int percent_ = -1;
};

}}

#endif //ADV_I3_PLUS_PLUS_PRIVATE_H

