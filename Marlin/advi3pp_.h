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
#include "temperature.h"
#include "printcounter.h"
#include "advi3pp_bitmasks.h"
#include "advi3pp_enums.h"
#include "advi3pp.h"
#include "advi3pp_stack.h"
#include "advi3pp_dgus.h"
#include "ADVcallback.h"
#include "ADVcrtp.h"


namespace advi3pp {

const size_t message_length = 48;
const size_t progress_name_length = 44;
const size_t progress_percent_length = progress_name_length + 4;
const uint8_t sd_file_length = 48;

using adv::Callback;
using BackgroundTask = Callback<void(*)()>;
using WaitCallback = Callback<void(*)()>;

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
// Pages
// --------------------------------------------------------------------

struct Pages
{
    void show_page(Page page, bool save_back = true);
    Page get_current_page();
    void save_forward_page();
    void show_back_page();
    void show_forward_page();

private:
    Stack<Page, 8> back_pages_{};
    Page forward_page_ = Page::None;
};

// --------------------------------------------------------------------
// Handler - Handle inputs from the LCD Panel
// --------------------------------------------------------------------

template<typename Self>
struct Handler: adv::Crtp<Self, Handler>
{
public:
    void handle(KeyValue value);
    void show(bool backup = true);

    bool dispatch(KeyValue value) { return this->self().do_dispatch(value); }
    void show_command() { this->self().do_show_command(); }
    void save_command() { this->self().do_save_command(); }
    void back_command() { this->self().do_back_command(); }
    Page prepare_page() { return this->self().do_prepare_page(); }
    void write(EepromWrite& eeprom) const { this->self().do_write(eeprom); }
    void read(EepromRead& eeprom) { this->self().do_read(eeprom); }
    void reset() { this->self().do_reset(); }
    uint16_t size_of() const { return this->self().do_size_of(); }
    void backup() { this->self().do_backup(); }
    void restore() { this->self().do_restore(); }

protected:
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
    void do_backup() {}
    void do_restore() {}
};

// --------------------------------------------------------------------
// Load and Unload
// --------------------------------------------------------------------

struct Wait: Handler<Wait>
{
    void show(const FlashChar* message, bool save_back = true);
    void show(const FlashChar* message, const WaitCallback& back, bool save_back = true);
    void show(const FlashChar* message, const WaitCallback& back, const WaitCallback& cont, bool save_back = true);
    void show_continue(const FlashChar* message, const WaitCallback& cont, bool save_back = true);

private:
    Page do_prepare_page();
    void do_save_command();
    void do_back_command();

    WaitCallback back_;
    WaitCallback continue_;

    friend Parent;
};

// --------------------------------------------------------------------
// Load and Unload
// --------------------------------------------------------------------

struct LoadUnload: Handler<LoadUnload>
{
private:
    bool do_dispatch(KeyValue key_value);
    Page do_prepare_page();
    void prepare(const BackgroundTask& background);
    void load_command();
    void unload_command();
    void stop();
    void stop_task();
    void load_start_task();
    void load_task();
    void unload_start_task();
    void unload_task();
    void start_task(const char* command,  const BackgroundTask& back_task);

    friend Parent;
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

struct Preheat: Handler<Preheat>
{
private:
    bool do_dispatch(KeyValue key_value);
    Page do_prepare_page();
    void do_write(EepromWrite& eeprom) const;
    void do_read(EepromRead& eeprom);
    void do_reset();
    uint16_t do_size_of() const;
    void do_save_command();

    void send_presets();
    void retrieve_presets();
    void previous_command();
    void next_command();
    void cooldown_command();

private:
    static const size_t NB_PRESETS = 5;
    Preset presets_[NB_PRESETS] = {};
    size_t index_ = 0;

    friend Parent;
};

// --------------------------------------------------------------------
// Move
// --------------------------------------------------------------------

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
// SD Card
// --------------------------------------------------------------------

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
    uint16_t nb_files_ = 0;
    uint16_t last_file_index_ = 0;

    friend Parent;
};

// --------------------------------------------------------------------
// Printing
// --------------------------------------------------------------------

template<typename D>
struct Print: Handler<D>
{
private:
    bool do_dispatch(KeyValue value);
    Page do_prepare_page();
    void stop_command();
    void pause_resume_command();
    void advanced_pause_command();
    void reset_messages_task();

    void stop() { this->self().do_stop(); }
    void pause() { this->self().do_pause(); }
    void resume() { this->self().do_resume(); }
    bool is_printing() const { return this->self().do_is_printing(); }

    friend Handler<D>;
};

// --------------------------------------------------------------------
// SD Print
// --------------------------------------------------------------------

struct SdPrint: Print<SdPrint>
{
private:
    void do_stop();
    void do_pause();
    void do_resume();
    bool do_is_printing() const;

    friend Print<SdPrint>;
};

// --------------------------------------------------------------------
// USB Print
// --------------------------------------------------------------------

struct UsbPrint: Print<UsbPrint>
{
private:
    void do_stop();
    void do_pause();
    void do_resume();
    bool do_is_printing() const;

    friend Print<UsbPrint>;
};

// --------------------------------------------------------------------
// Factory Reset
// --------------------------------------------------------------------

struct FactoryReset: Handler<FactoryReset>
{
private:
    Page do_prepare_page();
    void do_save_command();

    friend Parent;
};

// --------------------------------------------------------------------
// Manual Leveling
// --------------------------------------------------------------------

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
// Extruder Tuning
// --------------------------------------------------------------------

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

private:
    double extruded_ = 0.0;
    friend Parent;
};

// --------------------------------------------------------------------
// PID Tuning
// --------------------------------------------------------------------

struct PidTuning: Handler<PidTuning>
{
    void finished();

private:
    bool do_dispatch(KeyValue value);
    Page do_prepare_page();
    void step2_command();
    void hotend_command();
    void bed_command();

private:
    bool hotend_ = true;
    Uint16 temperature_;

    friend Parent;
};

#ifdef ADVi3PP_BLTOUCH

// --------------------------------------------------------------------
// Sensor Settings
// --------------------------------------------------------------------

struct SensorSettings: Handler<SensorSettings>
{
    void send_z_height_to_lcd(double height);
    void save_lcd_z_height();

private:
    bool do_dispatch(KeyValue value);
    Page do_prepare_page();
    void save_z_height(double height);
    void do_save_command();
    void previous_command();
    void next_command();

    friend Parent;
};

// --------------------------------------------------------------------
// Sensor Tuning
// --------------------------------------------------------------------

struct SensorTuning: Handler<SensorTuning>
{
    void g29_leveling_finished(bool success);

private:
    bool do_dispatch(KeyValue key_value);
    Page do_prepare_page();
    void leveling();
    void g29_leveling_failed();
    void self_test_command();
    void reset_command();
    void deploy_command();
    void stow_command();

private:
    bool sensor_interactive_leveling_ = false;

    friend Parent;
};

// --------------------------------------------------------------------
// Sensor Grid
// --------------------------------------------------------------------

struct SensorGrid: Handler<SensorGrid>
{
private:
    Page do_prepare_page();
    void do_save_command();

    friend Parent;
};

// --------------------------------------------------------------------
// Sensor Z Height
// --------------------------------------------------------------------

struct SensorZHeight: Handler<SensorZHeight>
{
private:
    bool do_dispatch(KeyValue key_value);
    Page do_prepare_page();
    void do_back_command();
    void do_save_command();
    void home_task();
    void center_task();
    void multiplier01_command();
    void multiplier05_command();
    void multiplier10_command();

    friend Parent;
};

#else

// --------------------------------------------------------------------
// No Sensor
// --------------------------------------------------------------------

struct SensorSettings: Handler<SensorSettings>
{
    void send_z_height_to_lcd(double) {}
	void save_lcd_z_height() {}

private:
    Page do_prepare_page();
    friend Parent;
};

struct SensorTuning: Handler<SensorTuning>
{
    void g29_leveling_finished(bool) {}

private:
    Page do_prepare_page();
    friend Parent;
};

struct SensorGrid: Handler<SensorGrid>
{
private:
    Page do_prepare_page();
    friend Parent;
};

struct SensorZHeight: Handler<SensorZHeight>
{
private:
    Page do_prepare_page();
    friend Parent;
};

#endif

// --------------------------------------------------------------------
// No Sensor
// --------------------------------------------------------------------

struct NoSensor: Handler<NoSensor>
{
private:
    Page do_prepare_page();

    friend Parent;
};

// --------------------------------------------------------------------
// Features Settings
// --------------------------------------------------------------------

template<typename D>
struct FeaturesSettings: Handler<D>
{
protected:
    void send_features();
    void do_save_command();

    Feature features_ = Feature::None;
};

// --------------------------------------------------------------------
// Firmware Setting
// --------------------------------------------------------------------

struct FirmwareSettings: FeaturesSettings<FirmwareSettings>
{
private:
    bool do_dispatch(KeyValue key_value);
    Page do_prepare_page();
    void do_save_command();
    void thermal_protection_command();
    void runout_sensor_command();
    void send_usb_baudrate();
    void baudrate_minus_command();
    void baudrate_plus_command();

    uint32_t usb_baudrate_ = 0;

    friend Parent;
};

// --------------------------------------------------------------------
// LCD Setting
// --------------------------------------------------------------------

struct LcdSettings: FeaturesSettings<LcdSettings>
{
private:
    bool do_dispatch(KeyValue key_value);
    Page do_prepare_page();
    void dim_command();
    void buzz_on_action_command();
    void buzz_on_press_command();

    friend Parent;
};

// --------------------------------------------------------------------
// Statistics
// --------------------------------------------------------------------

struct Statistics: Handler<Statistics>
{
private:
    Page do_prepare_page();
    void send_stats();

    friend Parent;
};

// --------------------------------------------------------------------
// Versions
// --------------------------------------------------------------------

struct Versions: Handler<Versions>
{
    void get_version_from_lcd();
    void send_advi3pp_version();
    bool is_lcd_version_valid() const;

private:
    bool do_dispatch(KeyValue key_value);
    Page do_prepare_page();
    void versions_mismatch_forward_command();
    void send_versions();

    uint16_t lcd_version_ = 0x0000;

    friend Parent;
};

// --------------------------------------------------------------------
// Sponsors
// --------------------------------------------------------------------

struct Sponsors: Handler<Sponsors>
{
private:
    Page do_prepare_page();

    friend Parent;
};

// --------------------------------------------------------------------
// Copyrights
// --------------------------------------------------------------------

struct Copyrights: Handler<Copyrights>
{
private:
    Page do_prepare_page();

    friend Parent;
};

// --------------------------------------------------------------------
// Print Settings
// --------------------------------------------------------------------

struct PrintSettings: Handler<PrintSettings>
{
    void baby_minus_command();
    void baby_plus_command();

protected:
    bool do_dispatch(KeyValue value);

private:
    Page do_prepare_page();
    void do_save_command();

private:
    double multiplier_ = 0.01;

    friend Parent;
};

// --------------------------------------------------------------------
// PidSettings
// --------------------------------------------------------------------

struct Pid
{
    float Kp_, Ki_, Kd_;
    uint16_t temperature_;
};

struct PidSettings: Handler<PidSettings>
{
public:
    void add(bool hotend, uint16_t temperature);

private:
    bool do_dispatch(KeyValue key_value);
    Page do_prepare_page();
    void do_backup();
    void do_restore();
    void do_save_command();
    void hotend_command();
    void bed_command();
    void previous_command();
    void next_command();

private:
    static const size_t NB_PIDs = 5;
    Pid backup_ = {};
    Pid pid_[2][NB_PIDs] = {};
    bool hotend_ = true;
    size_t index_ = 0;

    friend Parent;
};

// --------------------------------------------------------------------
// StepSettings
// --------------------------------------------------------------------

struct StepSettings: Handler<StepSettings>
{
private:
    Page do_prepare_page();
    void do_backup();
    void do_restore();
    void do_save_command();

    float backup_[XYZE_N] = {};

    friend Parent;
};

// --------------------------------------------------------------------
// FeedrateSettings
// --------------------------------------------------------------------

struct FeedrateSettings: Handler<FeedrateSettings>
{
private:
    Page do_prepare_page();
    void do_save_command();
    void do_backup();
    void do_restore();

    float backup_max_feedrate_mm_s_[XYZE_N] = {};
    float backup_min_feedrate_mm_s_ = 0;
    float backup_min_travel_feedrate_mm_s_ = 0;

    friend Parent;
};

// --------------------------------------------------------------------
// AccelerationSettings
// --------------------------------------------------------------------

struct AccelerationSettings: Handler<AccelerationSettings>
{
private:
    Page do_prepare_page();
    void do_save_command();
    void do_backup();
    void do_restore();

    uint32_t backup_max_acceleration_mm_per_s2_[XYZE_N] = {};
    float backup_acceleration_ = 0;
    float backup_retract_acceleration_ = 0;
    float backup_travel_acceleration_ = 0;

    friend Parent;
};

// --------------------------------------------------------------------
// JerkSettings
// --------------------------------------------------------------------

struct JerkSettings: Handler<JerkSettings>
{
private:
    Page do_prepare_page();
    void do_save_command();
    void do_backup();
    void do_restore();

    float backup_max_jerk_[XYZE] = {};

    friend Parent;
};

// --------------------------------------------------------------------
// Linear Advance Settings
// --------------------------------------------------------------------

struct LinearAdvanceSettings: Handler<LinearAdvanceSettings>
{
private:
    Page do_prepare_page();
    void do_save_command();
    void do_backup();
    void do_restore();

    float backup_extruder_advance_K = 0;

    friend Parent;
};

// --------------------------------------------------------------------
// Change Filament
// --------------------------------------------------------------------

struct ChangeFilament: Handler<ChangeFilament>
{
private:
    Page do_prepare_page();

    friend Parent;
};

// --------------------------------------------------------------------
// EEPROM Mistatch
// --------------------------------------------------------------------

struct EepromMismatch: Handler<EepromMismatch>
{
    bool does_mismatch() const;
    void set_mismatch();
    void reset_mismatch();

private:
    Page do_prepare_page();
    void do_save();

    bool mismatch_ = false;

    friend Parent;
};


// --------------------------------------------------------------------
// Linear Advance Tuning
// --------------------------------------------------------------------

struct LinearAdvanceTuning: Handler<LinearAdvanceTuning>
{
private:
    Page do_prepare_page();

    friend Parent;
};

// --------------------------------------------------------------------
// Diagnosis
// --------------------------------------------------------------------

struct Diagnosis: Handler<Diagnosis>
{
private:
    Page do_prepare_page();

    friend Parent;
};


// --------------------------------------------------------------------
// Advanced Pause
// --------------------------------------------------------------------

struct AdvancedPause: Handler<AdvancedPause>
{
    void advanced_pause_show_message(AdvancedPauseMessage message);

private:
    void init();
    void insert_filament();
    void printing();
    void filament_inserted();

private:
    AdvancedPauseMessage last_advanced_pause_message_ = static_cast<AdvancedPauseMessage>(-1);

    friend Parent;
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
    void write(eeprom_write write, int& eeprom_index, uint16_t& working_crc);
    void read(eeprom_read read, int& eeprom_index, uint16_t& working_crc);
    void reset();
    uint16_t size_of() const;
    void eeprom_settings_mismatch();
    void temperature_error(const FlashChar* message);
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
    void set_status(const FlashChar* fmt, ...);
    void set_status(const char * const fmt, va_list& args);
    void queue_status(const char* message);
    void queue_status(const FlashChar* message);
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
// Handler implementation
// --------------------------------------------------------------------

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
void Handler<Self>::show(bool backup)
{
    if(backup)
        this->backup();

    Page page = prepare_page();
    if(page != Page::None)
        pages.show_page(page);
}

template<typename Self>
void Handler<Self>::save_settings() const
{
    enqueue_and_echo_commands_P(PSTR("M500"));
}

template<typename Self>
void Handler<Self>::do_show_command()
{
    backup();
    Page page = prepare_page();
    if(page != Page::None)
        pages.show_page(page);
}

template<typename Self>
void Handler<Self>::do_save_command()
{
    pages.show_forward_page();
}

template<typename Self>
void Handler<Self>::do_back_command()
{
    restore();
    advi3pp.save_settings();
    pages.show_back_page();
}

// --------------------------------------------------------------------
// Printing
// --------------------------------------------------------------------

//! Handle print commands.
//! @param key_value    The sub-action to handle
template<typename D>
bool Print<D>::do_dispatch(KeyValue value)
{
    if(Handler<D>::do_dispatch(value))
        return true;

    switch(value)
    {
        case KeyValue::PrintStop:           stop_command(); break;
        case KeyValue::PrintPauseResume:    pause_resume_command(); break;
        case KeyValue::PrintAdvancedPause:  advanced_pause_command(); break;
        default:                            return false;
    }

    return true;
}

template<typename D>
Page Print<D>::do_prepare_page()
{
    return Page::Print;
}

//! Stop printing
template<typename D>
void Print<D>::stop_command()
{
    Log::log() << F("Stop Print") << Log::endl();

    stop();
    clear_command_queue();
    quickstop_stepper();
    PrintCounter::stop();
    Temperature::disable_all_heaters();
    fanSpeeds[0] = 0;

    pages.show_back_page();
    task.set_background_task(BackgroundTask(this, &Print::reset_messages_task), 500);
}

template<typename D>
void Print<D>::reset_messages_task()
{
    task.clear_background_task();
    advi3pp.reset_progress();
    advi3pp.reset_status();
}

//! Pause printing
template<typename D>
void Print<D>::pause_resume_command()
{
    // FIX
    Log::log() << F("Pause or Resume Print") << Log::endl();

    if(is_printing())
    {
        advi3pp.queue_status(F("Pause printing..."));
        pause();
        PrintCounter::pause();
#if ENABLED(PARK_HEAD_ON_PAUSE)
        enqueue_and_echo_commands_P(PSTR("M125"));
#endif
    }
    else
    {
        Log::log() << F("Resume Print") << Log::endl();

        advi3pp.queue_status(F("Resume printing"));
#if ENABLED(PARK_HEAD_ON_PAUSE)
        enqueue_and_echo_commands_P(PSTR("M24"));
#endif
        resume();
        PrintCounter::start(); // TODO: Check this is right
    }
}

//! Resume the current SD printing
template<typename D>
void Print<D>::advanced_pause_command()
{
    enqueue_and_echo_commands_P(PSTR("M600"));
}

// --------------------------------------------------------------------

}

#endif //ADV_I3_PLUS_PLUS_PRIVATE_H
