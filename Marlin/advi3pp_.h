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

#include "advi3pp_bitmasks.h"
#include "advi3pp_enums.h"
#include "advi3pp.h"
#include "advi3pp_utils.h"

namespace advi3pp { inline namespace internals {

static const Feature DEFAULT_FEATURES =
    Feature::ThermalProtection |
    Feature::HeadParking |
    Feature::Dimming |
    Feature::Buzzer;
static const Brightness DEFAULT_BRIGHTNESS = Brightness::Max;
static const uint32_t DEFAULT_USB_BAUDRATE = BAUDRATE;


enum class BackgroundTask: uint8_t
{
    None                = 0,
    ManualLeveling      = 1,
    LoadFilament        = 2,
    UnloadFilament      = 3,
    ExtruderCalibration = 4,
    ZHeightTuning       = 5,
    Undefined           = 0xFF
};

// --------------------------------------------------------------------
// PagesManager
// --------------------------------------------------------------------

struct PagesManager
{
    PagesManager() = default;

    void show_page(Page page, bool save_back = true);
    void show_waiting_page(const __FlashStringHelper* message);
    Page get_current_page();
    void show_back_page();
    void save_forward_page();
    void show_forward_page();

private:
    Stack<Page, 8> back_pages_{};
    Page forward_page_ = Page::None;
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
// Preheat
// --------------------------------------------------------------------

struct Preheat
{
    explicit Preheat(PagesManager& mgr): pages_{mgr} {}

    void store_eeprom_data(EepromWrite& eeprom);
    void restore_eeprom_data(EepromRead& eeprom);
    void reset_eeprom_data();

    void show();
    void back();
    void preset(uint16_t presetIndex);

private:
    static const size_t NB_PRESETS = 3;
    Preset presets_[NB_PRESETS] = {};
    PagesManager& pages_;
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
// LCD screen brightness and dimming
// --------------------------------------------------------------------

struct Dimming
{
    Dimming();

    void enable(bool enable);
    void check();
    void reset();
    void change_brightness(KeyValue brightness);
    void store_eeprom_data(EepromWrite& eeprom);
    void restore_eeprom_data(EepromRead& eeprom);
    void reset_eeprom_data();

private:
    void set_next_checking_time();
    void set_next_dimmming_time();
    void send_brightness();
    uint8_t get_adjusted_brithness();

private:
    bool enabled_ = true;
    bool dimming_ = false;
    Brightness brightness_ = DEFAULT_BRIGHTNESS;
    millis_t next_check_time_ = 0;
    millis_t next_dimming_time_ = 0;
};

// --------------------------------------------------------------------
// Sensor
// --------------------------------------------------------------------

#ifdef ADVi3PP_BLTOUCH

struct Sensor
{
    explicit Sensor(PagesManager& pages);

    void send_z_height_to_lcd(double height);
	void save_lcd_z_height();

    void self_test();
    void reset();
    void deploy();
    void stow();

private:
    void save_z_height(double height);

private:
    PagesManager& pages_;
};

#else

struct Sensor
{
    explicit Sensor(PagesManager& pages) {}

    void send_z_height_to_lcd(double height) {}
	void save_lcd_z_height() {}

    void leveling() {}
    void self_test() {}
    void reset() {}
    void deploy() {}
    void stow() {}

    void start_z_height() {}
};

#endif

// --------------------------------------------------------------------
// FilesManager
// --------------------------------------------------------------------

struct SDFilesManager
{
    explicit SDFilesManager(PagesManager& mgr);

    void show();
    void back();
    void up();
    void down();
    void select_file(uint16_t file_index);

private:
    void show_files();
    void get_file_name(uint8_t index_in_page, String& name);

private:
    uint16_t nb_files_ = 0;
    uint16_t last_file_index_ = 0;
    PagesManager& pages_;
};

// --------------------------------------------------------------------
// CommandProcessor
// --------------------------------------------------------------------

struct CommandProcessor
{
    CommandProcessor(PagesManager& pages, Sensor& sensor);

    void process(const GCodeParser& parser);

private:
    void icode_0(const GCodeParser& parser);

private:
    PagesManager& pages_;
    Sensor& sensor_;
};

// --------------------------------------------------------------------
// Graphs
// --------------------------------------------------------------------

struct Graphs
{
    Graphs();

    void clear();
    void send_data();
    void update();

private:
    millis_t next_update_graph_time_;
};

// --------------------------------------------------------------------
// BackTask
// --------------------------------------------------------------------

struct BackTask
{
    explicit BackTask(PagesManager& pages);

    void set_background_task(BackgroundTask task, unsigned int delta = 500);
    void clear_background_task();
    void execute_background_task();
    void send_status_data();
    void cancel_extruder_calibration();
    double extruded() const;

private:
    void set_next_update_time(unsigned int delta = 500);
    void set_next_background_task_time(unsigned int delta = 500);

    void load_filament_task();
    void unload_filament_task();
    void manual_leveling_task();
    void extruder_calibration_task();
    void extruder_calibration_finished();
    void z_height_tuning_task();

private:
    PagesManager pages_;
    millis_t next_op_time_ = 0;
    millis_t next_update_time_ = 0;
    BackgroundTask background_task_ = BackgroundTask::None;
    double extruded_ = 0.0;
};

inline double BackTask::extruded() const { return extruded_; };

// --------------------------------------------------------------------
// Printer implementation
// --------------------------------------------------------------------

//! Implementation of the Duplicator i3 Plus printer
struct Printer_
{
    Printer_();

    void setup();
    void task();
    void auto_pid_finished();
    void g29_leveling_finished();
    void store_eeprom_data(eeprom_write write, int& eeprom_index, uint16_t& working_crc);
    void restore_eeprom_data(eeprom_read read, int& eeprom_index, uint16_t& working_crc);
    void reset_eeprom_data();
    void temperature_error(const __FlashStringHelper* message);
    void send_status_data();
    bool is_thermal_protection_enabled() const;
    void process_command(const GCodeParser& parser);

    static void save_settings();

private:
    void send_gplv3_7b_notice(); // Forks: you have to keep this notice
    void send_versions();
    void read_lcd_serial();
    void send_stats();
    void show_boot_page();

    String get_lcd_firmware_version();
    void get_advi3pp_lcd_version();
    bool is_lcd_version_valid() const;

    void set_target_temperature(uint16_t temperature);
    uint16_t get_target_temperature();
    void send_features();
    void send_usb_baudrate();
    void change_usb_baudrate();

private:
    // Actions
    void sd_card(KeyValue key_value);

    void screen(KeyValue key_value);
    void show_temps();
    void show_print();
    void show_controls();
    void show_tuning();
    void show_settings();
    void show_infos();
    void show_motors();

    void back();

    void sd_print_command(KeyValue key_value);
    void sd_print_stop();
    void sd_print_pause();
    void sd_print_resume();
    void sd_print_back();

    void usb_print_command(KeyValue key_value);
    void usb_print_stop();
    void usb_print_pause();
    void usb_print_resume();
    void usb_print_back();

    void load_unload(KeyValue key_value);
    void load_unload_show();
    void load_unload_start(bool load);
    void load_unload_stop();

    void preheat(KeyValue key_value);
    void cooldown();

    void move(KeyValue key_value);
    void show_move();
    void move_x_plus();
    void move_x_minus();
    void move_x_home();
    void move_y_plus();
    void move_y_minus();
    void move_y_home();
    void move_z_plus();
    void move_z_minus();
    void move_z_home();
    void move_e_plus();
    void move_e_minus();
    void move_all_home();
    void disable_motors();
    void move_back();

    void print_settings(KeyValue key_value);
    void print_settings_show();
    void print_settings_save();
    void print_settings_cancel();

    void pid_settings(KeyValue key_value);
    void pid_settings_show(bool back, bool init = true);
    void pid_settings_save();
    void pid_settings_cancel();

    void steps_settings(KeyValue key_value);
    void steps_settings_show(bool init = true);
    void steps_settings_save();
    void steps_settings_cancel();

    void feedrate_settings(KeyValue key_value);
    void feedrate_settings_show(bool init = true);
    void feedrate_settings_save();
    void feedrate_settings_cancel();

    void acceleration_settings(KeyValue key_value);
    void acceleration_settings_show(bool init = true);
    void acceleration_settings_save();
    void acceleration_settings_cancel();

    void jerk_settings(KeyValue key_value);
    void jerk_settings_show(bool init = true);
    void jerk_settings_save();
    void jerk_settings_cancel();

    void statistics(KeyValue key_value);
    void show_stats();
    void stats_back();

    void pid_tuning(KeyValue key_value);
    void pid_tuning_step1();
    void pid_tuning_step2();
    void pid_tuning_cancel();

    void leveling(KeyValue key_value);
    void leveling_home();
    void leveling_point1();
    void leveling_point2();
    void leveling_point3();
    void leveling_point4();
    void leveling_point5();
    void leveling_finish();

    void extruder_calibration(KeyValue key_value);
    void show_extruder_calibration();
    void start_extruder_calibration();
    void extruder_calibrartion_settings();

    void xyz_motors_calibration(KeyValue key_value);
    void show_xyz_motors_calibration();
    void cancel_xyz_motors_calibration();
    void xyz_motors_calibration_settings();

    void sensor_settings(KeyValue key_value);
    void sensor_settings_show();
    void sensor_settings_save();
    void sensor_settings_cancel();

    void sensor_tuning(KeyValue key_value);
    void sensor_tuning_show();
    void sensor_tuning_back();
    void sensor_leveling();
    void sensor_z_height();

    void sensor_grid(KeyValue key_value);
    void sensor_grid_show();
    void sensor_grid_cancel();
    void sensor_grid_save();

    void sensor_z_height(KeyValue key_value);
    void sensor_z_height_cancel();
    void sensor_z_height_continue();

    void no_sensor(KeyValue key_value);
    void no_sensor_back();

    void change_filament(KeyValue key_value);
    void change_filament_show();
    void change_filament_continue();

    void firmware(KeyValue key_value);
    void firmware_settings_show();
    void firmware_settings_thermal_protection();
    void firmware_settings_baudrate_minus();
    void firmware_settings_baudrate_plus();
    void firmware_settings_save();
    void firmware_settings_back();

    void lcd(KeyValue key_value);
    void lcd_settings_show();
    void lcd_settings_dimming();
    void lcd_settings_buzzer();
    void lcd_settings_buzz_on_press();
    void lcd_settings_back();

    void factory_reset(KeyValue key_value);
    void show_factory_reset_warning();
    void do_factory_reset();
    void cancel_factory_reset();

    void versions(KeyValue key_value);
    void versions_show();
    void versions_mismatch_forward();
    void versions_back();

    void copyrights(KeyValue key_value);
    void copyrights_show();
    void copyrights_back();

private:
    PagesManager pages_;
    BackTask back_;
    SDFilesManager sd_files_;
    Preheat preheat_;
    PidSettings old_pid_{};
    StepSettings steps_{};
    FeedrateSettings feedrates_{};
    AccelerationSettings accelerations_{};
    JerkSettings jerks_{};
    uint16_t lcd_version_ = 0x0000;
    Feature features_ =  DEFAULT_FEATURES;
    uint32_t usb_baudrate_ = DEFAULT_USB_BAUDRATE;
    uint32_t usb_old_baudrate_ = DEFAULT_USB_BAUDRATE;
    Dimming dimming_{};
    CommandProcessor processor_;
    Sensor sensor_;
    Graphs graphs_;
    bool sensor_interactive_leveling_ = false;
};

// --------------------------------------------------------------------
// LCD implementation
// --------------------------------------------------------------------

//! Implementation of the Duplication i3 Plus LCD
struct LCD_
{
    static LCD_& instance();

    void update();
    void init();
    bool has_status();
    void set_status(const char* message);
    void set_status_PGM(const char* message);
    void set_alert_status_PGM(const char* message);
    void status_printf_P(const char* fmt, va_list argp);
    void set_status(const __FlashStringHelper* fmt, va_list argp);
    void buttons_update();
    void reset_alert_level();
    bool detected();
    void refresh();
    const String& get_message() const;
    void queue_message(const String& message);
    void reset_message();

    void set_progress_name(const String& name);
    const String& get_progress() const;
    void reset_progress();

    void enable_buzzer(bool enable);
    void enable_buzz_on_press(bool enable);
    void buzz(long duration, uint16_t frequency = 0);
    void buzz_on_press();

private:
    void buzz_(long duration);

private:
    String message_;
    String progress_name_;
    mutable String progress_percent_;
    mutable int percent_ = -1;
    bool buzzer_enabled_ = true;
    bool buzz_on_press_enabled_ = false;
};

}}

#endif //ADV_I3_PLUS_PLUS_PRIVATE_H

