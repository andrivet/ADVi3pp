/**
 * Marlin 3D Printer Firmware For Wanhao Duplicator i3 Plus (ADVi3++)
 *
 * Copyright (C) 2017-2019 Sebastien Andrivet [https://github.com/andrivet/]
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
#define ADVi3PP_PROBE
#endif

#include "Marlin.h"
#include "temperature.h"
#include "printcounter.h"
#include "planner.h"
#include "point_t.h"
#include "advi3pp_versions.h"
#include "advi3pp_bitmasks.h"
#include "advi3pp_enums.h"
#include "advi3pp.h"
#include "advi3pp_stack.h"
#include "advi3pp_dgus.h"
#include "ADVcallback.h"
#include "ADVcrtp.h"
#include "advi3pp_bitmasks.h"

// From Marlin
extern bool pause_print(const float &retract, const point_t &park_point, const float &unload_length=0, bool show_lcd=false);
extern void resume_print(const float &slow_load_length=0, const float &fast_load_length=0, const float &purge_length=ADVANCED_PAUSE_PURGE_LENGTH, int8_t max_beep_count=0);
extern bool ensure_safe_temperature(AdvancedPauseMode mode=ADVANCED_PAUSE_MODE_PAUSE_PRINT);

namespace advi3pp {

const size_t message_length = 48; //!< Size of messages to be displayed on the LCD Panel
const size_t progress_name_length = 44; //!< Size of the progress name (i.e. filename) to be displayed on the LCD Panel
const size_t progress_length = 48; //!< Size of the progress message (filename and percent) to be displayed on the LCD Panel
const uint8_t sd_file_length = 26; //!< This is the maximum length handled by the SD layer (FILENAME_LENGTH)

const uint16_t default_bed_temperature = 50; //!< Default target temperature for the bed
const uint16_t default_hotend_temperature = 200; //!< Default target temperature for the hotend

using adv::Callback;
using BackgroundTask = Callback<void(*)()>;
using WaitCallback = Callback<bool(*)()>;

//! Hotend, bed temperature and fan speed preset.
struct Preset
{
    uint16_t hotend;
    uint8_t bed;
    uint8_t fan;
};

//! Position of a sensor.
struct SensorPosition { int16_t x, y; };

// --------------------------------------------------------------------

//! Transform a value from a scale to another one.
//! @param value        Value to be transformed
//! @param valueScale   Current scale of the value (maximal)
//! @param targetScale  Target scale
//! @return             The scaled value
inline int16_t scale(int16_t value, int16_t valueScale, int16_t targetScale) { return value * targetScale / valueScale; }

// --------------------------------------------------------------------
// EEPROM Data Read & Write
// --------------------------------------------------------------------

//! EEPROM writer: utility class to write values into EEPROM
struct EepromWrite
{
    EepromWrite(eeprom_write write, int& eeprom_index, uint16_t& working_crc);
    template <typename T> void write(const T& data);

private:
    eeprom_write write_;
    int& eeprom_index_;
    uint16_t& working_crc_;
};

//! EEPROM reader: utility class to read values from EEPROM
struct EepromRead
{
    EepromRead(eeprom_read read, int& eeprom_index, uint16_t& working_crc);
    template <typename T> inline void read(T& data);

private:
    eeprom_read read_;
    int& eeprom_index_;
    uint16_t& working_crc_;
};

// --------------------------------------------------------------------
// Pages - Display a page on top of the others; display back and forward pages
// --------------------------------------------------------------------

//! Options when pushing a page onto the stack
enum class ShowOptions: uint8_t
{
    None     = 0x00, //!< No option
    SaveBack = 0x01  //!< Save the current page onto the stack of back pages
};
ENABLE_BITMASK_OPERATOR(ShowOptions);

//! Display a page on top of the others; display back and forward pages
struct Pages
{
    void show_page(Page page, ShowOptions options = ShowOptions::SaveBack);
    Page get_current_page();
    void save_forward_page();
    void show_back_page();
    void show_forward_page();

private:
    Stack<Page, 8> back_pages_{};
    Page forward_page_ = Page::None;
    Page current_page_ = Page::Main;
};

// --------------------------------------------------------------------
// Handler - Handle inputs from the LCD Panel
// --------------------------------------------------------------------

//! Handle inputs from the LCD Panel
template<typename Self>
struct Handler: adv::Crtp<Self, Handler>
{
public:
    void handle(KeyValue value);
    void show(ShowOptions options);

    bool dispatch(KeyValue value) { return this->self().do_dispatch(value); }
    void show_command() { this->self().do_show_command(); }
    void save_command() { this->self().do_save_command(); }
    void back_command() { this->self().do_back_command(); }
    Page prepare_page() { return this->self().do_prepare_page(); }
    void write(EepromWrite& eeprom) const { this->self().do_write(eeprom); }
    void read(EepromRead& eeprom) { this->self().do_read(eeprom); }
    void reset() { this->self().do_reset(); }
    uint16_t size_of() const { return this->self().do_size_of(); }

protected:
    Page do_prepare_page();
    bool do_dispatch(KeyValue value);
    void do_show_command();
    void do_save_command();
    void do_back_command();
    void invalid(KeyValue value);
    void save_settings() const;

private:
    void do_write(EepromWrite& eeprom) const {}
    void do_read(EepromRead& eeprom) {}
    void do_reset() {}
    uint16_t do_size_of() const { return 0; }
};

// --------------------------------------------------------------------
// Screens
// --------------------------------------------------------------------

//! Handle Temperature, SD card and Print screens
struct Screens: Handler<Screens>
{
private:
    bool do_dispatch(KeyValue value);
    void show_temps();
    void show_print();
    void show_sd_or_temp_page();
    void show_print_settings();

    friend Parent;
};


// --------------------------------------------------------------------
// Wait Messages
// --------------------------------------------------------------------

//! Display wait page and messages
struct Wait: Handler<Wait>
{
    void show(const FlashChar* message, ShowOptions options = ShowOptions::SaveBack);
    void show(const FlashChar* message, const WaitCallback& back, ShowOptions options = ShowOptions::SaveBack);
    void show(const FlashChar* message, const WaitCallback& back, const WaitCallback& cont, ShowOptions options = ShowOptions::SaveBack);
    void show_continue(const FlashChar* message, const WaitCallback& cont, ShowOptions options = ShowOptions::SaveBack);
    void show_continue(const FlashChar* message, ShowOptions options = ShowOptions::SaveBack);
    void show_continue(ShowOptions options = ShowOptions::SaveBack);

private:
    Page do_prepare_page();
    void do_save_command();
    void do_back_command();
    bool on_continue();

    WaitCallback back_;
    WaitCallback continue_;

    friend Parent;
};

// --------------------------------------------------------------------
// Temperatures Graph
// --------------------------------------------------------------------

//! Temperatures Graph
struct Temperatures: Handler<Temperatures>
{
    void show(const WaitCallback& back);
    void show(ShowOptions options = ShowOptions::SaveBack);

private:
    Page do_prepare_page();
    void do_back_command();

    WaitCallback back_;

    friend Parent;
};

// --------------------------------------------------------------------
// Load and Unload Page
// --------------------------------------------------------------------

//! Load and Unload Page
struct LoadUnload: Handler<LoadUnload>
{
private:
    bool do_dispatch(KeyValue key_value);
    Page do_prepare_page();
    void prepare(const BackgroundTask& background);
    void load_command();
    void unload_command();
    bool stop();
    void stop_task();
    void load_start_task();
    void load_task();
    void unload_start_task();
    void unload_task();
    void start_task(const char* command,  const BackgroundTask& back_task);

    friend Parent;
};

// --------------------------------------------------------------------
// Preheat Page
// --------------------------------------------------------------------

//! Preheat Page
struct Preheat: Handler<Preheat>
{
    static const size_t NB_PRESETS = 5;

private:
    bool do_dispatch(KeyValue key_value);
    Page do_prepare_page();
    void do_save_command();
    void do_write(EepromWrite& eeprom) const;
    void do_read(EepromRead& eeprom);
    void do_reset();
    uint16_t do_size_of() const;

    void send_presets();
    void retrieve_presets();
    void previous_command();
    void next_command();
    void cooldown_command();

private:
    Preset presets_[NB_PRESETS] = {};
    size_t index_ = 0;

    friend Parent;
};

// --------------------------------------------------------------------
// Move Page
// --------------------------------------------------------------------

//! Move Page
struct Move: Handler<Move>
{
    void x_plus_command();
    void x_minus_command();
    void x_home_command();
    void y_plus_command();
    void y_minus_command();
    void y_home_command();
    void z_plus_command();
    void z_minus_command();
    void z_home_command();
    void e_plus_command();
    void e_minus_command();
    void all_home_command();
    void disable_motors_command();

private:
    bool do_dispatch(KeyValue key_value);
    Page do_prepare_page();
    void move(const char* command, millis_t delay);

private:
    millis_t last_move_time_ = 0;

    friend Parent;
};

// --------------------------------------------------------------------
// Sensor Tuning Page
// --------------------------------------------------------------------

#ifdef ADVi3PP_PROBE
//! Sensor Tuning Page
struct SensorTuning: Handler<SensorTuning>
{
private:
    bool do_dispatch(KeyValue key_value);
    Page do_prepare_page();
    void self_test_command();
    void reset_command();
    void deploy_command();
    void stow_command();

private:
    friend Parent;
};
#else
//! Sensor Tuning Page
struct SensorTuning: Handler<SensorTuning>
{
private:
    Page do_prepare_page();
    friend Parent;
};
#endif

// --------------------------------------------------------------------
// Automatic Leveling Page
// --------------------------------------------------------------------

#ifdef ADVi3PP_PROBE
//! Automatic Leveling Page
struct AutomaticLeveling: Handler<AutomaticLeveling>
{
    void g29_leveling_finished(bool success);

private:
    Page do_prepare_page();
    bool g29_leveling_failed();

private:
    bool sensor_interactive_leveling_ = false;

    friend Parent;
};
#else
//! Automatic Leveling Page
struct AutomaticLeveling: Handler<AutomaticLeveling>
{
    void g29_leveling_finished(bool) {}

private:
    Page do_prepare_page();
    friend Parent;
};
#endif

// --------------------------------------------------------------------
// Leveling Grid Page
// --------------------------------------------------------------------

#ifdef ADVi3PP_PROBE
//! Leveling Grid Page
struct LevelingGrid: Handler<LevelingGrid>
{
private:
    Page do_prepare_page();
    void do_save_command();

    friend Parent;
};
#else
//! Leveling Grid Page
struct LevelingGrid: Handler<LevelingGrid>
{
private:
    Page do_prepare_page();
    friend Parent;
};
#endif

// --------------------------------------------------------------------
// Manual Leveling Page
// --------------------------------------------------------------------

//! Manual Leveling Page
struct ManualLeveling: Handler<ManualLeveling>
{
private:
    bool do_dispatch(KeyValue value);
    Page do_prepare_page();
    void do_back_command();
    void point1_command();
    void point2_command();
    void point3_command();
    void point4_command();
    void point5_command();
    void pointA_command();
    void pointB_command();
    void pointC_command();
    void pointD_command();
    void leveling_task();

    friend Parent;
};

// --------------------------------------------------------------------
// SD Card Page
// --------------------------------------------------------------------

//! SD Card Page
struct SdCard: Handler<SdCard>
{
    void show_first_page();

private:
    bool do_dispatch(KeyValue value);
    Page do_prepare_page();
    void show_current_page();
    void get_file_name(uint8_t index_in_page, ADVString<sd_file_length>& name);
    void up_command();
    void down_command();
    void select_file_command(uint16_t file_index);

private:
    static constexpr uint16_t nb_visible_sd_files = 5; //!< Number of files per page on the SD screen

    uint16_t nb_files_ = 0;
    uint16_t last_file_index_ = 0;
    uint16_t page_index_ = 0;

    friend Parent;
};

// --------------------------------------------------------------------
// Printing Page
// --------------------------------------------------------------------

//! Printing Page
struct Print: Handler<Print>
{
    void process_pause_code();
    void process_stop_code();
    void pause_finished(bool success);
    bool is_printing() const;
    bool is_usb_printing() const;
    void send_stop_usb_print();

private:
    bool do_dispatch(KeyValue value);
    Page do_prepare_page();
    void stop_command();
    void pause_command();
    void advanced_pause_command();

    friend Parent;
};


// --------------------------------------------------------------------
// Advanced Pause Page
// --------------------------------------------------------------------

//! Advanced Pause Page
struct AdvancedPause: Handler<AdvancedPause>
{
    void advanced_pause_show_message(AdvancedPauseMessage message);

private:
    void insert_filament();
    bool filament_inserted();

private:
    AdvancedPauseMessage last_advanced_pause_message_ = static_cast<AdvancedPauseMessage>(-1);

    friend Parent;
};

// --------------------------------------------------------------------
// Sensor Z Height Tuning Page
// --------------------------------------------------------------------

#ifdef ADVi3PP_PROBE
//! Sensor Z Height Tuning Page
struct SensorZHeight: Handler<SensorZHeight>
{
    void minus();
    void plus();

    enum class Multiplier: uint8_t
    {
        M1 = 0,
        M2 = 1,
        M3 = 2
    };

private:
    bool do_dispatch(KeyValue key_value);
    Page do_prepare_page();
    void do_save_command();
    void do_back_command();
    void post_home_task();
    void multiplier1_command();
    void multiplier2_command();
    void multiplier3_command();
    double get_multiplier_value() const;
    void adjust_height(double offset);
    void send_data() const;
    void reset();

private:
    Multiplier multiplier_ = Multiplier::M1;
    friend Parent;
};
#else
//! Sensor Z Height Tuning Page
struct SensorZHeight: Handler<SensorZHeight>
{
    void minus() {}
    void plus() {}

private:
    Page do_prepare_page();
    friend Parent;
};
#endif

// --------------------------------------------------------------------
// Extruder Tuning Page
// --------------------------------------------------------------------

//! Extruder Tuning Page
struct ExtruderTuning: Handler<ExtruderTuning>
{
private:
    bool do_dispatch(KeyValue value);
    Page do_prepare_page();
    void do_back_command();
    void start_command();
    void settings_command();
    void heating_task();
    void extruding_task();
    void finished();
    bool cancel();

private:
    static constexpr uint16_t tuning_extruder_filament = 100; //!< Filament to extrude (10 cm)
    static constexpr uint16_t tuning_extruder_delta = 20; //!< Amount of filament supposes tp remain after extruding (2 cm)

    float extruded_ = 0.0;
    friend Parent;
};

// --------------------------------------------------------------------
// PID Tuning Page
// --------------------------------------------------------------------

//! PID Tuning Page
struct PidTuning: Handler<PidTuning>
{
    void finished(bool success);
    void send_data();

private:
    bool do_dispatch(KeyValue value);
    Page do_prepare_page();
    void step2_command();
    bool cancel_pid();
    void hotend_command();
    void bed_command();

private:
    uint16_t temperature_ = 0;
    TemperatureKind kind_ = TemperatureKind::Hotend;
    bool inTuning_ = false;

    friend Parent;
};

// --------------------------------------------------------------------
// Linear Advance Tuning Page
// --------------------------------------------------------------------

//! Linear Advance Tuning Page
//! Note: Not fully implemented
struct LinearAdvanceTuning: Handler<LinearAdvanceTuning>
{
private:
    Page do_prepare_page();

    friend Parent;
};

// --------------------------------------------------------------------
// Diagnosis Page
// --------------------------------------------------------------------

//! Diagnosis Page
struct Diagnosis: Handler<Diagnosis>
{
    enum class State: uint8_t { Off = 0, On = 1, Output = 2};

private:
    Page do_prepare_page();
    void do_back_command();
    State get_pin_state(uint8_t pin);
    void send_data();

    friend Parent;
};


// --------------------------------------------------------------------
// Sensor Settings Page
// --------------------------------------------------------------------

#ifdef ADVi3PP_PROBE
//! Sensor Settings Page
struct SensorSettings: Handler<SensorSettings>
{
    static const size_t NB_SENSOR_POSITIONS = 3;

    SensorSettings();

    int x_probe_offset_from_extruder() const;
    int y_probe_offset_from_extruder() const;
    int left_probe_bed_position();
    int right_probe_bed_position();
    int front_probe_bed_position();
    int back_probe_bed_position();

private:
    bool do_dispatch(KeyValue value);
    Page do_prepare_page();
    void do_save_command();
    void do_write(EepromWrite& eeprom) const;
    void do_read(EepromRead& eeprom);
    void do_reset();
    uint16_t do_size_of() const;
    void previous_command();
    void next_command();
    void send_data() const;
    void get_data();

private:
    uint16_t index_ = 0;
    SensorPosition positions_[NB_SENSOR_POSITIONS];

    friend Parent;
};
#else
//! Sensor Settings Page
struct SensorSettings: Handler<SensorSettings>
{
    void send_z_height_to_lcd(double) {}
	void save_lcd_z_height() {}

private:
    Page do_prepare_page();
    friend Parent;
};
#endif


// --------------------------------------------------------------------
// Firmware Setting Page
// --------------------------------------------------------------------

//! Firmware Setting Page
struct FirmwareSettings: Handler<FirmwareSettings>
{
private:
    bool do_dispatch(KeyValue key_value);
    Page do_prepare_page();
    void do_save_command();
    void thermal_protection_command();
    void runout_sensor_command();
    void send_usb_baudrate() const;
    void send_features() const;
    void baudrate_minus_command();
    void baudrate_plus_command();
    size_t usb_baudrate_index(uint32_t baudrate);

    uint32_t usb_baudrate_ = BAUDRATE;
    Feature features_ = Feature::BuzzOnPress | Feature::Dimming | Feature::ThermalProtection;

    friend Parent;
};

// --------------------------------------------------------------------
// LCD Setting Page
// --------------------------------------------------------------------

//! LCD Setting Page
struct LcdSettings: Handler<LcdSettings>
{
    void change_brightness(uint16_t brightness);

private:
    bool do_dispatch(KeyValue key_value);
    Page do_prepare_page();
    void dimming_command();
    void buzz_on_action_command();
    void buzz_on_press_command();
    void send_data() const;

    Feature features_ = Feature::None;
    friend Parent;
};

// --------------------------------------------------------------------
// Print Settings Page
// --------------------------------------------------------------------

//! Print Settings Page
struct PrintSettings: Handler<PrintSettings>
{
    void feedrate_minus_command();
    void feedrate_plus_command();
    void fan_minus_command();
    void fan_plus_command();
    void hotend_minus_command();
    void hotend_plus_command();
    void bed_minus_command();
    void bed_plus_command();
    void baby_minus_command();
    void baby_plus_command();

    enum class Multiplier: uint8_t { M1 = 0, M2 = 1, M3 = 2 };

protected:
    bool do_dispatch(KeyValue value);

private:
    Page do_prepare_page();
    void send_data() const;
    double get_multiplier_value() const;

private:
    Multiplier multiplier_ = Multiplier::M1;

    friend Parent;
};

// --------------------------------------------------------------------
// PID Settings Page
// --------------------------------------------------------------------

//! PID Settings Page
struct Pid
{
    float Kp_, Ki_, Kd_;
    uint16_t temperature_;
};

struct PidSettings: Handler<PidSettings>
{
    PidSettings();

    void add_pid(TemperatureKind kind, uint16_t temperature);
    void set_best_pid(TemperatureKind kind, uint16_t temperature);

private:
    bool do_dispatch(KeyValue key_value);
    Page do_prepare_page();
    void do_write(EepromWrite& eeprom) const;
    void do_read(EepromRead& eeprom);
    void do_reset();
    uint16_t do_size_of() const;
    void do_save_command();
    void do_back_command();
    void hotend_command();
    void bed_command();
    void previous_command();
    void next_command();
    void set_current_pid() const;
    void get_current_pid();
    void send_data() const;
    void save_data();

private:
    static const size_t NB_PIDs = 3;
    Pid hotend_pid_[NB_PIDs] = {};
    Pid bed_pid_[NB_PIDs] = {};
    TemperatureKind kind_ = TemperatureKind::Hotend;
    size_t index_ = 0;

    friend Parent;
};

// --------------------------------------------------------------------
// Step Settings Page
// --------------------------------------------------------------------

//! Step Settings Page
struct StepSettings: Handler<StepSettings>
{
private:
    Page do_prepare_page();
    void do_save_command();

    friend Parent;
};

// --------------------------------------------------------------------
// Feedrate Settings Page
// --------------------------------------------------------------------

//! Feedrate Settings Page
struct FeedrateSettings: Handler<FeedrateSettings>
{
private:
    Page do_prepare_page();
    void do_save_command();

    friend Parent;
};

// --------------------------------------------------------------------
// Acceleration Settings Page
// --------------------------------------------------------------------

//! Acceleration Settings Page
struct AccelerationSettings: Handler<AccelerationSettings>
{
private:
    Page do_prepare_page();
    void do_save_command();

    friend Parent;
};

// --------------------------------------------------------------------
// Jerk Settings Page
// --------------------------------------------------------------------

//! Jerk Settings Page
struct JerkSettings: Handler<JerkSettings>
{
private:
    Page do_prepare_page();
    void do_save_command();

    friend Parent;
};

// --------------------------------------------------------------------
// Linear Advance Settings Page
// --------------------------------------------------------------------

//! Linear Advance Settings Page
struct LinearAdvanceSettings: Handler<LinearAdvanceSettings>
{
private:
    Page do_prepare_page();
    void do_save_command();

    friend Parent;
};

// --------------------------------------------------------------------
// Factory Reset Page
// --------------------------------------------------------------------

//! Factory Reset Page
struct FactoryReset: Handler<FactoryReset>
{
private:
    Page do_prepare_page();
    void do_save_command();

    friend Parent;
};

// --------------------------------------------------------------------
// Statistics Page
// --------------------------------------------------------------------

//! Statistics Page
struct Statistics: Handler<Statistics>
{
private:
    Page do_prepare_page();
    void send_stats();

    friend Parent;
};

// --------------------------------------------------------------------
// Versions Page
// --------------------------------------------------------------------

//! Versions Page
struct Versions: Handler<Versions>
{
    bool check();

private:
    Page do_prepare_page();
    void get_version_from_lcd();
    bool is_lcd_version_valid();
    void send_versions() const;

    uint16_t lcd_version_ = 0x0000;

    friend Parent;
};

// --------------------------------------------------------------------
// Sponsors Page
// --------------------------------------------------------------------

//! Sponsors Page
struct Sponsors: Handler<Sponsors>
{
private:
    Page do_prepare_page();

    friend Parent;
};

// --------------------------------------------------------------------
// Copyrights Page
// --------------------------------------------------------------------

//! Copyrights Page
struct Copyrights: Handler<Copyrights>
{
private:
    Page do_prepare_page();

    friend Parent;
};


// --------------------------------------------------------------------
// Change Filament Page
// --------------------------------------------------------------------

//! Change Filament Page
struct ChangeFilament: Handler<ChangeFilament>
{
private:
    Page do_prepare_page();

    friend Parent;
};

// --------------------------------------------------------------------
// EEPROM Mismatch Page
// --------------------------------------------------------------------

//! EEPROM Mismatch Page
struct EepromMismatch: Handler<EepromMismatch>
{
    bool check();
    void set_mismatch();
    void reset_mismatch();

private:
    bool does_mismatch() const;
    Page do_prepare_page();
    void do_save_command();

    bool mismatch_ = false;

    friend Parent;
};

// --------------------------------------------------------------------
// Versions Mismatch Page
// --------------------------------------------------------------------

//! EEPROM Mismatch Page
struct VersionsMismatch: Handler<VersionsMismatch>
{
private:
    Page do_prepare_page();
    void do_save_command();

    friend Parent;
};


// --------------------------------------------------------------------
// No Sensor Page
// --------------------------------------------------------------------

//! No Sensor Page
struct NoSensor: Handler<NoSensor>
{
private:
    Page do_prepare_page();

    friend Parent;
};

// --------------------------------------------------------------------
// LCD screen brightness and dimming
// --------------------------------------------------------------------

//! LCD screen brightness and dimming
struct Dimming
{
    Dimming();

    void enable(bool enable, bool doIt = true);
    void check();
    void reset(bool force = false);
    void change_brightness(int16_t brightness);

private:
    void set_next_checking_time();
    void set_next_dimmming_time();
    void send_brightness();
    uint8_t get_adjusted_brightness();

private:
    static constexpr int8_t brightness_min = 0x01; //!< Minimum value for brightness
    static constexpr int8_t brightness_max = 0x40; //!< Maximum value for brightness
    static constexpr uint8_t dimming_ratio = 5; //!< Ratio (in percent) between normal and dimmed LCD panel
    static constexpr uint16_t dimming_delay = 5 * 60; //!< Delay before dimming the LCD panel (5 minutes)

    bool enabled_ = true;
    bool dimming_ = false;
    millis_t next_check_time_ = 0;
    millis_t next_dimming_time_ = 0;
};

// --------------------------------------------------------------------
// Graphs
// --------------------------------------------------------------------

//! Graphs
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

//! Background Task
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

//! ADVi3++ class
struct ADVi3pp_
{
    void setup_lcd_serial();
    void change_baudrate();
    void setup();
    void idle();
    void write(eeprom_write write, int& eeprom_index, uint16_t& working_crc);
    bool read(eeprom_read read, int& eeprom_index, uint16_t& working_crc);
    void reset();
    uint16_t size_of() const;
    void eeprom_settings_mismatch();
    void temperature_error(const FlashChar* message);
    bool is_thermal_protection_enabled() const;
    void advanced_pause_show_message(AdvancedPauseMessage message);
    void set_brightness(int16_t britghness);
    void save_settings();
    void restore_settings();
    bool is_busy();

    bool has_status();
    void set_status(const char* message);
    void set_status(const char* fmt, va_list& args);
    void set_status(const FlashChar* message);
    void set_status(const FlashChar* fmt, va_list& args);
    void reset_status();
    void stop_and_wait();

    void set_progress_name(const char* name);
    void reset_progress();

    void enable_buzzer(bool enable, bool doIt = true);
    void enable_buzz_on_press(bool enable, bool doIt = true);
    void buzz(long duration = 100);
    void buzz_on_press();
    uint32_t get_current_baudrate() const { return usb_baudrate_; }
    void change_usb_baudrate(uint32_t baudrate, bool disconnect);
    Feature get_current_features() const { return features_; }
    void change_features(Feature features);

    uint16_t get_last_used_temperature(TemperatureKind kind) const;
    void on_set_temperature(TemperatureKind kind, uint16_t temperature);

    double get_current_z_height(int multiply = 1) const;

    void process_command(const GCodeParser& parser);

private:
    void buzz_(long duration);
    void init();
    void update_progress();
    void send_status_data(bool force_update = false);
    void send_gplv3_7b_notice(); // Forks: you have to keep this notice
    void send_sponsors();
    void read_lcd_serial();
    void show_boot_page();

    void compute_progress();

private:
    uint16_t version_ = settings_version;
    bool init_ = true;
    uint32_t usb_baudrate_ = BAUDRATE;
    Feature features_ = Feature::None;
    uint16_t last_used_temperature_[2] = {default_bed_temperature, default_hotend_temperature};
    bool has_status_ = false;
    ADVString<message_length> message_;
    ADVString<message_length> centered_;
    ADVString<progress_name_length> progress_name_;
    ADVString<progress_length> progress_;
    int percent_ = -1;
    bool buzzer_enabled_ = true;
    bool buzz_on_press_enabled_ = false;
};

// --------------------------------------------------------------------
// Singletons
// --------------------------------------------------------------------

inline namespace singletons
{
    extern ADVi3pp_ advi3pp;
    extern Pages pages;
    extern Task task;
}

// --------------------------------------------------------------------
// EEPROM Data Read & Write implementations
// --------------------------------------------------------------------

inline EepromWrite::EepromWrite(eeprom_write write, int& eeprom_index, uint16_t& working_crc)
        : write_(write), eeprom_index_(eeprom_index), working_crc_(working_crc)
{
}

template <typename T>
inline void EepromWrite::write(const T& data)
{
    write_(eeprom_index_, reinterpret_cast<const uint8_t*>(&data), sizeof(T), &working_crc_);
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
// Handler implementation
// --------------------------------------------------------------------

template<typename Self>
Page Handler<Self>::do_prepare_page()
{
    return Page::None;
}

template<typename Self>
void Handler<Self>::handle(KeyValue value)
{
    if(!dispatch(value))
        invalid(value);
}

template<typename Self>
bool Handler<Self>::do_dispatch(KeyValue value)
{
    switch(value)
    {
        case KeyValue::Show: show_command(); break;
        case KeyValue::Save: save_command(); break;
        case KeyValue::Back: back_command(); break;
        default: return false;
    }

    return true;
}

template<typename Self>
void Handler<Self>::invalid(KeyValue value)
{
    Log::error() << F("Invalid key value ") << static_cast<uint16_t>(value) << Log::endl();
}

template<typename Self>
void Handler<Self>::show(ShowOptions options)
{
    Page page = prepare_page();
    if(page != Page::None)
        pages.show_page(page, options);
}

template<typename Self>
void Handler<Self>::save_settings() const
{
    enqueue_and_echo_commands_P(PSTR("M500"));
}

template<typename Self>
void Handler<Self>::do_show_command()
{
    show(ShowOptions::SaveBack);
}

template<typename Self>
void Handler<Self>::do_save_command()
{
    advi3pp.save_settings();
    pages.show_forward_page();
}

template<typename Self>
void Handler<Self>::do_back_command()
{
    pages.show_back_page();
}


// --------------------------------------------------------------------

}

#endif //ADV_I3_PLUS_PLUS_PRIVATE_H
