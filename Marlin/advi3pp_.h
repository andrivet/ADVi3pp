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
#include "ADVcallback.h"

namespace advi3pp {

class Printer_;

using andrivet::Callback;
using BackgroundTask = Callback<void(*)()>;
using WaitCalllback = Callback<void(*)()>;

// --------------------------------------------------------------------
// PagesManager
// --------------------------------------------------------------------

struct PagesManager
{
    explicit PagesManager(Printer_& printer);

    void show_page(Page page, bool save_back = true);
    void show_wait_page(const __FlashStringHelper* message, bool save_back = true);
    void show_wait_back_page(const __FlashStringHelper* message, WaitCalllback back, bool save_back = true);
    void show_wait_back_continue_page(const __FlashStringHelper* message, WaitCalllback back, WaitCalllback cont, bool save_back = true);
    void show_wait_continue_page(const __FlashStringHelper* message, WaitCalllback cont, bool save_back = true);
    void handle_wait(KeyValue key_value);
    Page get_current_page();
    void show_back_page();
    void save_forward_page();
    void show_forward_page();

private:
    void handle_lcd_back();
    void handle_lcd_continue();

private:
    Printer_& printer_;
    Stack<Page, 8> back_pages_{};
    Page forward_page_ = Page::None;
    WaitCalllback back_;
    WaitCalllback continue_;
};

// --------------------------------------------------------------------
// Settings
// --------------------------------------------------------------------

struct Task;
struct Printer_;
struct LCD_;

struct Handler
{
    explicit Handler(Printer_& printer);

    void handle(KeyValue value);
    void show(bool save_forward, bool save_back);
    virtual void store_eeprom_data(EepromWrite& eeprom);
    virtual void restore_eeprom_data(EepromRead& eeprom);
    virtual void reset_eeprom_data();
    virtual uint16_t size_of_eeprom_data() const;

protected:
    virtual bool dispatch(KeyValue value);
    void invalid(KeyValue value);

    PagesManager& pages() const { return printer_.pages(); }
    Task& task() const { return printer_.task(); }
    Printer_& printer() const { return printer_; }

private:
    virtual void save();
    virtual void back();
    virtual void do_backup();
    virtual void do_rollback();
    virtual Page do_show() = 0;
    virtual void do_save();

private:
    Printer_& printer_;
};

// --------------------------------------------------------------------
// Load and Unload
// --------------------------------------------------------------------

struct LoadUnload: Handler
{
    explicit LoadUnload(Printer_& printer);

private:
    bool dispatch(KeyValue key_value) override;
    Page do_show() override;
    void prepare();
    void load();
    void unload();
    void stop();
    void stop_task();
    void load_start_task();
    void load_task();
    void unload_start_task();
    void unload_task();
};

// --------------------------------------------------------------------
// Preset
// --------------------------------------------------------------------

//! Hostend and bad temperature preset.
struct Preset
{
    uint16_t hotend;
    uint16_t bed;
    uint16_t fan;
};

// --------------------------------------------------------------------
// Preheat
// --------------------------------------------------------------------

struct Preheat: Handler
{
    explicit Preheat(PagesManager& mgr);

    void store_eeprom_data(EepromWrite& eeprom) override;
    void restore_eeprom_data(EepromRead& eeprom) override;
    void reset_eeprom_data() override;
    uint16_t size_of_eeprom_data() const override;

private:
    bool dispatch(KeyValue key_value) override;
    Page do_show() override;
    void send_preset();
    void previous();
    void next();
    void cooldown();

private:
    static const size_t NB_PRESETS = 5;
    Preset presets_[NB_PRESETS];
    size_t index_ = 0;
};

// --------------------------------------------------------------------
// Move
// --------------------------------------------------------------------

struct Move: Handler
{
    explicit Move(PagesManager& pages);

private:
    bool dispatch(KeyValue key_value) override;
    Page do_show() override;

    void x_plus();
    void x_minus();
    void x_home();
    void y_plus();
    void y_minus();
    void y_home();
    void z_plus();
    void z_minus();
    void z_home();
    void e_plus();
    void e_minus();
    void all_home();
    void disable_motors();
    void move(const char* command, millis_t delay);

private:
    millis_t last_move_time_ = 0;
};

// --------------------------------------------------------------------
// SD Card
// --------------------------------------------------------------------

struct SdCard: Handler
{
    explicit SdCard(PagesManager& mgr, Task& task);

    void show_first_page();

private:
    bool dispatch(KeyValue value) override;
    void show_current_page();
    void get_file_name(uint8_t index_in_page, String& name);
    void up();
    void down();
    void select_file(uint16_t file_index);

private:
    uint16_t nb_files_ = 0;
    uint16_t last_file_index_ = 0;
};

// --------------------------------------------------------------------
// Print
// --------------------------------------------------------------------

struct Print: Handler
{
    explicit Print(PagesManager& pages);

private:
    bool dispatch(KeyValue value) override;
    void stop();
    void pause_resume();
    void advanced_pause();
    void reset_messages_task();

    virtual void do_stop() = 0;
    virtual void do_pause() = 0;
    virtual void do_resume() = 0;
    virtual bool is_printing() const = 0;
};

// --------------------------------------------------------------------
// SD Print
// --------------------------------------------------------------------

struct SdPrint: Print
{
    explicit SdPrint(PagesManager& pages);

private:
    void do_stop() override;
    void do_pause() override;
    void do_resume() override;
    bool is_printing() const override;
};

// --------------------------------------------------------------------
// USB Print
// --------------------------------------------------------------------

struct UsbPrint: Print
{
    explicit UsbPrint(PagesManager& pages);

private:
    void do_stop() override;
    void do_pause() override;
    void do_resume() override;
    bool is_printing() const override;
};

// --------------------------------------------------------------------
// Factory Reset
// --------------------------------------------------------------------

struct FactoryReset: Handler
{
    explicit FactoryReset(PagesManager& pages);
};

// --------------------------------------------------------------------
// Manual Leveling
// --------------------------------------------------------------------

struct ManualLeveling: Handler
{
    explicit ManualLeveling(PagesManager& pages);
};

// --------------------------------------------------------------------
// Extruder Tuning
// --------------------------------------------------------------------

struct ExtruderTuning: Handler
{
    explicit ExtruderTuning(PagesManager& pages);
};

// --------------------------------------------------------------------
// PID Tuning
// --------------------------------------------------------------------

struct PidTuning: Handler
{
    explicit PidTuning(PagesManager& pages);
};

// --------------------------------------------------------------------
// Sensor
// --------------------------------------------------------------------

#ifdef ADVi3PP_BLTOUCH

struct SensorSettings: Handler
{
    explicit SensorSettings(PagesManager& pages);

    void send_z_height_to_lcd(double height);
    void save_lcd_z_height();

    void self_test();
    void reset();
    void deploy();
    void stow();

private:
    void save_z_height(double height);
};

struct NoSensor: Handler
{
    explicit NoSensor(PagesManager& pages);
};

#else

struct SensorSettings: Handler
{
    explicit SensorSettings(PagesManager& pages) {}

    void send_z_height_to_lcd(double height) {}
	void save_lcd_z_height() {}

    void leveling() {}
    void self_test() {}
    void reset() {}
    void deploy() {}
    void stow() {}

    void start_z_height() {}
};

struct NoSensor: Handler
{
    explicit NoSensor(PagesManager& pages);
};

#endif

// --------------------------------------------------------------------
// Firmware Setting
// --------------------------------------------------------------------

struct FirmwareSettings: Handler
{
    explicit FirmwareSettings(PagesManager& pages);
};

// --------------------------------------------------------------------
// LCD Setting
// --------------------------------------------------------------------

struct LcdSettings: Handler
{
    explicit LcdSettings(PagesManager& pages);
};

// --------------------------------------------------------------------
// Statistics
// --------------------------------------------------------------------

struct Statistics: Handler
{
    explicit Statistics(PagesManager& pages);
};

// --------------------------------------------------------------------
// Versions
// --------------------------------------------------------------------

struct Versions: Handler
{
    explicit Versions(PagesManager& pages);
};

// --------------------------------------------------------------------
// Print Settings
// --------------------------------------------------------------------

struct PrintSettings: Handler
{
    explicit PrintSettings(PagesManager& pages);

    void baby_minus();
    void baby_plus();

protected:
    bool dispatch(KeyValue value) override;

private:
    Page do_show() override;
    void do_save() override;

private:
    double multiplier_ = 0.01;
};

// --------------------------------------------------------------------
// PidSettings
// --------------------------------------------------------------------

struct Pid
{
    float Kp_, Ki_, Kd_;
    int temperature_;
};

struct PidSettings: Handler
{
public:
    explicit PidSettings(PagesManager& pages);

    void set(uint16_t temperature, bool bed);
    void next();
    void previous();

private:
    void do_backup() override;
    void do_rollback() override;
    Page do_show() override;
    void do_save() override;

private:
    static const size_t NB_PIDs = 5;
    Pid backup_;
    Pid pid_[2][NB_PIDs];
    bool bed_ = true;
    size_t index_ = 0;
};

// --------------------------------------------------------------------
// StepSettings
// --------------------------------------------------------------------

struct StepSettings: Handler
{
    explicit StepSettings(PagesManager& pages);

private:
    void do_backup() override;
    void do_rollback() override;
    Page do_show() override;
    void do_save() override;

    float backup_[XYZE_N];
};

// --------------------------------------------------------------------
// FeedrateSettings
// --------------------------------------------------------------------

struct FeedrateSettings: Handler
{
    explicit FeedrateSettings(PagesManager& pages);

private:
    void do_backup() override;
    void do_rollback() override;
    Page do_show() override;
    void do_save() override;

    float backup_max_feedrate_mm_s_[XYZE_N];
    float backup_min_feedrate_mm_s_;
    float backup_min_travel_feedrate_mm_s_;
};

// --------------------------------------------------------------------
// AccelerationSettings
// --------------------------------------------------------------------

struct AccelerationSettings: Handler
{
    explicit AccelerationSettings(PagesManager& pages);

private:
    void do_backup() override;
    void do_rollback() override;
    Page do_show() override;
    void do_save() override;

    uint32_t backup_max_acceleration_mm_per_s2_[XYZE_N];
    float backup_acceleration_;
    float backup_retract_acceleration_;
    float backup_travel_acceleration_;
};

// --------------------------------------------------------------------
// JerkSettings
// --------------------------------------------------------------------

struct JerkSettings: Handler
{
    explicit JerkSettings(PagesManager& pages);

private:
    void do_backup() override;
    void do_rollback() override;
    Page do_show() override;
    void do_save() override;

    float backup_max_jerk_[XYZE];
};

// --------------------------------------------------------------------
// Copyrights
// --------------------------------------------------------------------

struct Copyrights: Handler
{
    explicit Copyrights(PagesManager& pages);
};

// --------------------------------------------------------------------
// Sensor Tuning
// --------------------------------------------------------------------

struct SensorTuning: Handler
{
    explicit SensorTuning(PagesManager& pages);
};

// --------------------------------------------------------------------
// Sensor Grid
// --------------------------------------------------------------------

struct SensorGrid: Handler
{
    explicit SensorGrid(PagesManager& pages);
};

// --------------------------------------------------------------------
// Sensor Z Height
// --------------------------------------------------------------------

struct SensorZHeight: Handler
{
    explicit SensorZHeight(PagesManager& pages);
};

// --------------------------------------------------------------------
// Change Filament
// --------------------------------------------------------------------

struct ChangeFilament: Handler
{
    explicit ChangeFilament(PagesManager& pages);
};

// --------------------------------------------------------------------
// EEPROM Mistatch
// --------------------------------------------------------------------

struct EepromMismatch: Handler
{
    explicit EepromMismatch(PagesManager& pages);

    bool does_mismatch() const;
    void set_mismatch();
    void reset_mismatch();
};

// --------------------------------------------------------------------
// Sponsors
// --------------------------------------------------------------------

struct Sponsors: Handler
{
    explicit Sponsors(PagesManager& pages);
};

// --------------------------------------------------------------------
// Linear Advance Tuning
// --------------------------------------------------------------------

struct LinearAdvanceTuning: Handler
{
    explicit LinearAdvanceTuning(PagesManager& pages);
};

// --------------------------------------------------------------------
// Linear Advance Settings
// --------------------------------------------------------------------

struct LinearAdvanceSettings: Handler
{
    explicit LinearAdvanceSettings(PagesManager& pages);
};

// --------------------------------------------------------------------
// Diagnosis
// --------------------------------------------------------------------

struct Diagnosis: Handler
{
    explicit Diagnosis(PagesManager& pages);
};


// --------------------------------------------------------------------
// Advanced Pause
// --------------------------------------------------------------------

struct AdvancedPause: Handler
{
    explicit AdvancedPause(PagesManager& pages);

    void advanced_pause_show_message(AdvancedPauseMessage message);

private:
    void init();
    void insert_filament();
    void printing();
    void filament_inserted();

private:
    PagesManager& pages_;
    AdvancedPauseMessage last_advanced_pause_message_ = static_cast<AdvancedPauseMessage>(-1);
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
    void change_brightness(int16_t brightness);

private:
    void set_next_checking_time();
    void set_next_dimmming_time();
    void send_brightness();
    uint8_t get_adjusted_brithness();

private:
    bool enabled_ = true;
    bool dimming_ = false;
    millis_t next_check_time_ = 0;
    millis_t next_dimming_time_ = 0;
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
// Background Task
// --------------------------------------------------------------------

struct Task
{
    explicit Task(Printer_& printer, PagesManager& pages);

    void set_background_task(BackgroundTask task, unsigned int delta = 500);
    void clear_background_task();
    void execute_background_task();
    bool has_background_task() const;
    bool is_update_time();

private:
    void set_next_update_time(unsigned int delta = 500);

private:
    Printer_& printer_;
    PagesManager pages_;
    unsigned int op_time_delta_ = 500;
    millis_t next_op_time_ = 0;
    millis_t next_update_time_ = 0;
    BackgroundTask background_task_;
};


// --------------------------------------------------------------------
// LCD implementation
// --------------------------------------------------------------------

//! Implementation of the Duplication i3 Plus LCD
struct LCD_
{
    explicit LCD_(PagesManager& pages);

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
    PagesManager& pages_;
    String message_;
    String progress_name_;
    mutable String progress_percent_;
    mutable int percent_ = -1;
    bool buzzer_enabled_ = true;
    bool buzz_on_press_enabled_ = false;
};

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
    void g29_leveling_finished(bool success);
    void store_eeprom_data(eeprom_write write, int& eeprom_index, uint16_t& working_crc);
    void restore_eeprom_data(eeprom_read read, int& eeprom_index, uint16_t& working_crc);
    void reset_eeprom_data();
    uint16_t size_of_eeprom_data() const;
    void eeprom_settings_mismatch();
    void temperature_error(const __FlashStringHelper* message);
    bool is_thermal_protection_enabled() const;
    void process_command(const GCodeParser& parser);
    void advanced_pause_show_message(AdvancedPauseMessage message);
    void set_brightness(int16_t britghness);
    uint16_t last_used_hotend_temperature() const;
    uint16_t last_used_bed_temperature() const;

    void save_settings();

private:
    void init();
    void check_and_fix();
    void update_progress();
    void send_status_data(bool force_update = false);
    void send_gplv3_7b_notice(); // Forks: you have to keep this notice
    void send_sponsors();
    void send_versions();
    void read_lcd_serial();
    void send_stats();
    void show_boot_page();
    void reset_messages_task();
    bool is_busy();

    void g29_leveling_failed();;

    String get_lcd_firmware_version();
    void get_advi3pp_lcd_version();
    bool is_lcd_version_valid() const;

    void send_features();
    void send_usb_baudrate();
    void change_usb_baudrate();

    void icode_0(const GCodeParser& parser);

    friend LCD_& LCD_::instance();

private:
    // Actions
    void screen(KeyValue key_value);
    void show_temps();
    void show_print();
    void show_sd_or_temp_page();
    void back();

    void print_command(KeyValue key_value);

    void preheat(KeyValue key_value);
    void cooldown();

    void move(KeyValue key_value);
    void show_move();
    void move(const char* command, millis_t delay);
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

    void statistics(KeyValue key_value);
    void show_stats();
    void stats_back();

    void pid_tuning(KeyValue key_value);
    void pid_tuning_step1();
    void pid_tuning_step2();
    void pid_tuning_cancel();

    void manual_leveling(KeyValue key_value);
    void leveling_home();
    void leveling_point1();
    void leveling_point2();
    void leveling_point3();
    void leveling_point4();
    void leveling_point5();
    void leveling_finish();
    void manual_leveling_task();

    void extruder_tuning(KeyValue key_value);
    void show_extruder_tuning();
    void start_extruder_tuning();
    void extruder_calibration_settings();
    void extruder_tuning_heating_task();
    void extruder_tuning_extruding_task();
    void extruder_tuning_finished();
    void cancel_extruder_tuning();

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
    void z_height_tuning_home_task();
    void z_height_tuning_center_task();

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

    void sponsors(KeyValue key_value);
    void sponsors_show();
    void sponsors_back();

    void copyrights(KeyValue key_value);
    void copyrights_show();
    void copyrights_back();

    void eeprom_mimatch(KeyValue key_value);
    void eeprom_mimatch_continue();

    void linear_advance_tuning(KeyValue key_value);
    void linear_advance_settings(KeyValue key_value);
    void diagnosis(KeyValue key_value);

private:
    PagesManager pages_;
    LCD_ lcd_;
    Task task_;
    uint16_t lcd_version_ = 0x0000;
    Feature features_;
    uint32_t usb_baudrate_;
    uint32_t usb_old_baudrate_;
    Dimming dimming_{};
    Graphs graphs_;

    LoadUnload load_unload_;
    Preheat preheat_;
    Move move_;
    SdCard sd_card_;
    FactoryReset factory_reset_;
    ManualLeveling manual_leveling_;
    ExtruderTuning extruder_tuning_;
    PidTuning pid_tuning_;
    SensorSettings sensor_settings_;
    FirmwareSettings firmware_settings_;
    NoSensor no_sensor_;
    LcdSettings lcd_settings_;
    Statistics statistics_;
    Versions versions_;
    PrintSettings print_settings_;
    PidSettings pid_settings_;
    StepSettings steps_settings_;
    FeedrateSettings feedrates_settings_;
    AccelerationSettings accelerations_settings_;
    JerkSettings jerks_settings_;
    Copyrights copyrights_;
    SensorTuning sensor_tuning_;
    SensorGrid sensor_grid_;
    SensorZHeight sensor_z_height_;
    ChangeFilament change_filament_;
    EepromMismatch eeprom_mismatch_;
    Sponsors sponsors_;
    LinearAdvanceTuning linear_advance_tuning_;
    LinearAdvanceSettings linear_advance_settings_;
    Diagnosis diagnosis_;
    SdPrint sd_print_;
    UsbPrint usb_print_;
    AdvancedPause pause_;

    bool init_ = true;
    bool sensor_interactive_leveling_ = false;
    double extruded_ = 0.0;
    uint16_t last_used_temperature = 200;
};

}

#endif //ADV_I3_PLUS_PLUS_PRIVATE_H

