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

// This is only to ensure that Jetbrains CLion is parsing code properly inside the IDE
#ifdef __CLION_IDE__
#define HAS_BED_PROBE 1
#endif

#include "Marlin.h"
#include "advi3pp_bitmasks.h"
#include "advi3pp_enums.h"
#include "advi3pp.h"
#include "advi3pp_stack.h"
#include "advi3pp_dgus.h"
#include "ADVcallback.h"

namespace advi3pp {

const size_t message_length = 40;
const size_t progress_name_length = 40;
const size_t progress_percent_length = progress_name_length + 4;
const uint8_t sd_file_length = 48;

using andrivet::Callback;
using BackgroundTask = Callback<void(*)()>;
using WaitCallback = Callback<void(*)()>;

// --------------------------------------------------------------------

//! Transform a value from a scale to another one.
//! @param value        Value to be transformed
//! @param valueScale   Current scale of the value (maximal)
//! @param targetScale  Target scale
//! @return             The scaled value
inline int16_t scale(int16_t value, int16_t valueScale, int16_t targetScale) { return value * targetScale / valueScale; }

template <typename T, size_t N>
constexpr size_t countof(T const (&)[N]) noexcept { return N; }

// --------------------------------------------------------------------
// EEPROM Data Read & Write
// --------------------------------------------------------------------

struct EepromWrite
{
    EepromWrite(eeprom_write write, int& eeprom_index, uint16_t& working_crc);
    template <typename T> void write(T& data);

private:
    eeprom_write write_;
    int& eeprom_index_;
    uint16_t& working_crc_;
};

struct EepromRead
{
    EepromRead(eeprom_read read, int& eeprom_index, uint16_t& working_crc);
    template <typename T> inline void read(T& data);

private:
    eeprom_read read_;
    int& eeprom_index_;
    uint16_t& working_crc_;
};

inline EepromWrite::EepromWrite(eeprom_write write, int& eeprom_index, uint16_t& working_crc)
        : write_(write), eeprom_index_(eeprom_index), working_crc_(working_crc)
{
}

template <typename T>
inline void EepromWrite::write(T& data)
{
    write_(eeprom_index_, reinterpret_cast<uint8_t*>(&data), sizeof(T), &working_crc_);
}

inline EepromRead::EepromRead(eeprom_read read, int& eeprom_index, uint16_t& working_crc)
        : read_(read), eeprom_index_(eeprom_index), working_crc_(working_crc)
{
}

template <typename T>
inline void EepromRead::read(T& data)
{
    read_(eeprom_index_, reinterpret_cast<uint8_t*>(&data), sizeof(T), &working_crc_, false);
}

// --------------------------------------------------------------------
// Pages
// --------------------------------------------------------------------

struct Pages
{
    void show_page(Page page, bool save_back = true);
    Page get_current_page();
    void show_back_page();
    void save_forward_page();
    void show_forward_page();

private:
    Stack<Page, 8> back_pages_{};
    Page forward_page_ = Page::None;
};

// --------------------------------------------------------------------
// Handler - Handle inputs from the LCD Panel
// --------------------------------------------------------------------

struct Handler
{
    Handler() = default;
    virtual ~Handler() = default;

    void handle(KeyValue value);
    void show(bool save_forward = false, bool save_back = true);
    virtual void store_eeprom_data(EepromWrite& eeprom);
    virtual void restore_eeprom_data(EepromRead& eeprom);
    virtual void reset_eeprom_data();
    virtual uint16_t size_of_eeprom_data() const;

protected:
    virtual bool dispatch(KeyValue value);
    virtual void save();
    virtual void back();
    virtual void do_backup();
    virtual void do_rollback();
    virtual Page do_show();
    virtual void do_save();

    void invalid(KeyValue value);
};

// --------------------------------------------------------------------
// Load and Unload
// --------------------------------------------------------------------

struct Wait: Handler
{
    void show(const __FlashStringHelper* message, bool save_back = true);
    void show(const __FlashStringHelper* message, const WaitCallback& back, bool save_back = true);
    void show(const __FlashStringHelper* message, const WaitCallback& back, const WaitCallback& cont, bool save_back = true);
    void show_continue(const __FlashStringHelper* message, const WaitCallback& cont, bool save_back = true);

private:
    void do_save() override;
    void do_rollback() override;

    WaitCallback back_;
    WaitCallback continue_;
};

// --------------------------------------------------------------------
// Load and Unload
// --------------------------------------------------------------------

struct LoadUnload: Handler
{
private:
    bool dispatch(KeyValue key_value) override;
    Page do_show() override;
    void prepare(const BackgroundTask& background);
    void load();
    void unload();
    void stop();
    void stop_task();
    void load_start_task();
    void load_task();
    void unload_start_task();
    void unload_task();
    void start_task(const char* command,  const BackgroundTask& back_task);
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
    void store_eeprom_data(EepromWrite& eeprom) override;
    void restore_eeprom_data(EepromRead& eeprom) override;
    void reset_eeprom_data() override;
    uint16_t size_of_eeprom_data() const override;

private:
    bool dispatch(KeyValue key_value) override;
    Page do_show() override;
    void send_presets();
    void previous();
    void next();
    void cooldown();

private:
    static const size_t NB_PRESETS = 5;
    Preset presets_[NB_PRESETS] = {};
    size_t index_ = 0;
};

// --------------------------------------------------------------------
// Move
// --------------------------------------------------------------------

struct Move: Handler
{
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

private:
    bool dispatch(KeyValue key_value) override;
    Page do_show() override;
    void move(const char* command, millis_t delay);

private:
    millis_t last_move_time_ = 0;
};

// --------------------------------------------------------------------
// SD Card
// --------------------------------------------------------------------

struct SdCard: Handler
{
    void show_first_page();

private:
    bool dispatch(KeyValue value) override;
    Page do_show() override;
    void show_current_page();
    void get_file_name(uint8_t index_in_page, ADVString<sd_file_length>& name);
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
private:
    bool dispatch(KeyValue value) override;
    Page do_show() override;
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
private:
    Page do_show() override;
    void do_save() override;
};

// --------------------------------------------------------------------
// Manual Leveling
// --------------------------------------------------------------------

struct ManualLeveling: Handler
{
private:
    bool dispatch(KeyValue value) override;
    Page do_show() override;
    void back() override;
    void point1();
    void point2();
    void point3();
    void point4();
    void point5();
    void pointA();
    void pointB();
    void pointC();
    void pointD();
    void leveling_task();
};

// --------------------------------------------------------------------
// Extruder Tuning
// --------------------------------------------------------------------

struct ExtruderTuning: Handler
{
private:
    bool dispatch(KeyValue value) override;
    Page do_show() override;
    void back() override;
    void start();
    void heating_task();
    void extruding_task();
    void finished();
    void settings();

private:
    double extruded_ = 0.0;
};

// --------------------------------------------------------------------
// PID Tuning
// --------------------------------------------------------------------

struct PidTuning: Handler
{
    void finished();

private:
    bool dispatch(KeyValue value) override;
    Page do_show() override;
    void step2();
    void hotend();
    void bed();
};

#ifdef ADVi3PP_BLTOUCH

// --------------------------------------------------------------------
// Sensor Settings
// --------------------------------------------------------------------

struct SensorSettings: Handler
{
    void send_z_height_to_lcd(double height);
    void save_lcd_z_height();

private:
    bool dispatch(KeyValue value) override;
    Page do_show() override;
    void save_z_height(double height);
    void do_save() override;
    void previous();
    void next();
};

// --------------------------------------------------------------------
// Sensor Tuning
// --------------------------------------------------------------------

struct SensorTuning: Handler
{
    void g29_leveling_finished(bool success);

private:
    bool dispatch(KeyValue key_value) override;
    Page do_show() override;
    void leveling();
    void g29_leveling_failed();
    void self_test();
    void reset();
    void deploy();
    void stow();

private:
    bool sensor_interactive_leveling_ = false;
};

// --------------------------------------------------------------------
// Sensor Grid
// --------------------------------------------------------------------

struct SensorGrid: Handler
{
private:
    Page do_show() override;
    void do_save() override;
};

// --------------------------------------------------------------------
// Sensor Z Height
// --------------------------------------------------------------------

struct SensorZHeight: Handler
{
private:
    bool dispatch(KeyValue key_value) override;
    Page do_show() override;
    void do_rollback() override;
    void save() override;
    void home_task();
    void center_task();
    void multiplier01();
    void multiplier05();
    void multiplier10();
};

#else // No sensor

struct SensorSettings: Handler
{
    void send_z_height_to_lcd(double) {}
	void save_lcd_z_height() {}

private:
    Page do_show() override;
};

struct SensorTuning: Handler
{
    void g29_leveling_finished(bool) {}

private:
    Page do_show() override;
};

struct SensorGrid: Handler
{
private:
    Page do_show() override;
};

struct SensorZHeight: Handler
{
private:
    Page do_show() override;
};

#endif

// --------------------------------------------------------------------
// No Sensor
// --------------------------------------------------------------------

struct NoSensor: Handler
{
private:
    Page do_show() override;
};

// --------------------------------------------------------------------
// Features Settings
// --------------------------------------------------------------------

struct FeaturesSettings: Handler
{
protected:
    void send_features();
    void do_save() override;

    Feature features_ = Feature::None;
};

// --------------------------------------------------------------------
// Firmware Setting
// --------------------------------------------------------------------

struct FirmwareSettings: FeaturesSettings
{
private:
    bool dispatch(KeyValue key_value) override;
    Page do_show() override;
    void do_save() override;
    void thermal_protection();
    void runout_sensor();
    void send_usb_baudrate();
    void baudrate_minus();
    void baudrate_plus();

    uint32_t usb_baudrate_ = 0;
};

// --------------------------------------------------------------------
// LCD Setting
// --------------------------------------------------------------------

struct LcdSettings: FeaturesSettings
{
private:
    bool dispatch(KeyValue key_value) override;
    Page do_show() override;
    void dim();
    void buzz_on_action();
    void buzz_on_press();
};

// --------------------------------------------------------------------
// Statistics
// --------------------------------------------------------------------

struct Statistics: Handler
{
private:
    Page do_show() override;
    void send_stats();
};

// --------------------------------------------------------------------
// Versions
// --------------------------------------------------------------------

struct Versions: Handler
{
    void get_version_from_lcd();
    void send_advi3pp_version();
    bool is_lcd_version_valid() const;

private:
    bool dispatch(KeyValue key_value) override;
    Page do_show() override;
    void versions_mismatch_forward();
    void send_versions();

    uint16_t lcd_version_ = 0x0000;
};

// --------------------------------------------------------------------
// Print Settings
// --------------------------------------------------------------------

struct PrintSettings: Handler
{
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
    uint16_t temperature_;
};

struct PidSettings: Handler
{
public:
    void set(uint16_t temperature, bool bed);

private:
    bool dispatch(KeyValue key_value) override;
    void do_backup() override;
    void do_rollback() override;
    Page do_show() override;
    void do_save() override;
    void hotend();
    void bed();
    void previous();
    void next();

private:
    static const size_t NB_PIDs = 5;
    Pid backup_ = {};
    Pid pid_[2][NB_PIDs] = {};
    bool bed_ = true;
    size_t index_ = 0;
};

// --------------------------------------------------------------------
// StepSettings
// --------------------------------------------------------------------

struct StepSettings: Handler
{
private:
    Page do_show() override;
    void do_backup() override;
    void do_rollback() override;
    void do_save() override;

    float backup_[XYZE_N] = {};
};

// --------------------------------------------------------------------
// FeedrateSettings
// --------------------------------------------------------------------

struct FeedrateSettings: Handler
{
private:
    void do_backup() override;
    void do_rollback() override;
    Page do_show() override;
    void do_save() override;

    float backup_max_feedrate_mm_s_[XYZE_N] = {};
    float backup_min_feedrate_mm_s_ = 0;
    float backup_min_travel_feedrate_mm_s_ = 0;
};

// --------------------------------------------------------------------
// AccelerationSettings
// --------------------------------------------------------------------

struct AccelerationSettings: Handler
{
private:
    void do_backup() override;
    void do_rollback() override;
    Page do_show() override;
    void do_save() override;

    uint32_t backup_max_acceleration_mm_per_s2_[XYZE_N] = {};
    float backup_acceleration_ = 0;
    float backup_retract_acceleration_ = 0;
    float backup_travel_acceleration_ = 0;
};

// --------------------------------------------------------------------
// JerkSettings
// --------------------------------------------------------------------

struct JerkSettings: Handler
{
private:
    void do_backup() override;
    void do_rollback() override;
    Page do_show() override;
    void do_save() override;

    float backup_max_jerk_[XYZE] = {};
};

// --------------------------------------------------------------------
// Copyrights
// --------------------------------------------------------------------

struct Copyrights: Handler
{
private:
    Page do_show() override;
};

// --------------------------------------------------------------------
// Change Filament
// --------------------------------------------------------------------

struct ChangeFilament: Handler
{
private:
    Page do_show() override;
};

// --------------------------------------------------------------------
// EEPROM Mistatch
// --------------------------------------------------------------------

struct EepromMismatch: Handler
{
    bool does_mismatch() const;
    void set_mismatch();
    void reset_mismatch();

private:
    Page do_show() override;
    void do_save() override;

    bool mismatch_ = false;
};

// --------------------------------------------------------------------
// Sponsors
// --------------------------------------------------------------------

struct Sponsors: Handler
{
private:
    Page do_show() override;
};

// --------------------------------------------------------------------
// Linear Advance Tuning
// --------------------------------------------------------------------

struct LinearAdvanceTuning: Handler
{
private:
    Page do_show() override;
};

// --------------------------------------------------------------------
// Linear Advance Settings
// --------------------------------------------------------------------

struct LinearAdvanceSettings: Handler
{
private:
    Page do_show() override;
};

// --------------------------------------------------------------------
// Diagnosis
// --------------------------------------------------------------------

struct Diagnosis: Handler
{
private:
    Page do_show() override;
};


// --------------------------------------------------------------------
// Advanced Pause
// --------------------------------------------------------------------

struct AdvancedPause: Handler
{
    void advanced_pause_show_message(AdvancedPauseMessage message);

private:
    void init();
    void insert_filament();
    void printing();
    void filament_inserted();

private:
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
    uint8_t get_adjusted_brightness();

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
    void set_background_task(const BackgroundTask& task, unsigned int delta = 500);
    void clear_background_task();
    void execute_background_task();
    bool has_background_task() const;
    bool is_update_time();

private:
    void set_next_update_time(unsigned int delta = 500);

private:
    unsigned int op_time_delta_ = 500;
    millis_t next_op_time_ = 0;
    millis_t next_update_time_ = 0;
    BackgroundTask background_task_;
};


// --------------------------------------------------------------------
// Main class
// --------------------------------------------------------------------

struct ADVi3pp_
{
    void setup();
    void idle();
    void store_eeprom_data(eeprom_write write, int& eeprom_index, uint16_t& working_crc);
    void restore_eeprom_data(eeprom_read read, int& eeprom_index, uint16_t& working_crc);
    void reset_eeprom_data();
    uint16_t size_of_eeprom_data() const;
    void eeprom_settings_mismatch();
    void temperature_error(const __FlashStringHelper* message);
    bool is_thermal_protection_enabled() const;
    void process_command();
    void advanced_pause_show_message(AdvancedPauseMessage message);
    void set_brightness(int16_t britghness);
    uint16_t last_used_hotend_temperature() const { return last_used_hotend_temperature_; }
    uint16_t last_used_bed_temperature() const { return last_used_bed_temperature_; }
    void save_settings();
    bool is_busy();

    bool has_status();
    void set_status(const char* message);
    void set_status(const __FlashStringHelper* fmt, ...);
    void set_status(const char * const fmt, va_list& args);
    void queue_status(const char* message);
    void queue_status(const __FlashStringHelper* message);
    void reset_status();

    void set_progress_name(const char* name);
    void reset_progress();

    void enable_buzzer(bool enable);
    void enable_buzz_on_press(bool enable);
    void buzz(long duration, uint16_t frequency = 0);
    void buzz_on_press();
    uint32_t get_current_baudrate() const { return usb_baudrate_; }
    void change_usb_baudrate(uint32_t baudrate);
    Feature get_current_features() const { return features_; }
    void change_features(Feature features);

private:
    void buzz_(long duration);
    void init();
    void check_and_fix();
    void update_progress();
    void send_status_data(bool force_update = false);
    void send_gplv3_7b_notice(); // Forks: you have to keep this notice
    void send_sponsors();
    void read_lcd_serial();
    void show_boot_page();

    void icode_0(const GCodeParser& parser);

    void screen(KeyValue key_value);
    void show_temps();
    void show_print();
    void show_sd_or_temp_page();
    void back();

    void print_command(KeyValue key_value);

    void compute_progress();

private:
    bool init_ = true;
    uint32_t usb_baudrate_ = BAUDRATE;
    Feature features_ = Feature::None;
    uint16_t last_used_hotend_temperature_ = 200;
    uint16_t last_used_bed_temperature_ = 50;
    bool has_status_ = false;
    ADVString<message_length> message_;
    ADVString<message_length> centered_;
    ADVString<progress_name_length> progress_name_;
    ADVString<progress_percent_length> progress_;
    int percent_ = -1;
    bool buzzer_enabled_ = true;
    bool buzz_on_press_enabled_ = false;
};

}

#endif //ADV_I3_PLUS_PLUS_PRIVATE_H
