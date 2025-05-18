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
#include "core.h"
#include "graphs.h"
#include "dimming.h"
#include "pages.h"
#include "task.h"
#include "dgus.h"
#include "buzzer.h"
#include "reentrant.h"
#include "status.h"
#include "progress.h"
#include "dates.h"
#include "../screens/common/wait.h"
#include "../screens/common/set_temperature.h"

#include "../screens/controls/load_unload.h"
#include "../screens/controls/preheat.h"
#include "../screens/controls/move.h"

#include "../screens/leveling/rough.h"
#include "../screens/leveling/mesh.h"
#include "../screens/leveling/grid.h"
#include "../screens/leveling/z_height.h"
#include "../screens/leveling/xtwist.h"

#include "../screens/print/print.h"
#include "../screens/print/pause_options.h"
#include "../screens/print/sd_card.h"
#include "../screens/print/temperatures.h"
#include "../screens/print/powerloss_recovery.h"
#include "../screens/print/print_options.h"
#include "../screens/print/baby_steps.h"

#include "../screens/tuning/extruder_tuning.h"
#include "../screens/tuning/pid_tuning.h"
#include "../screens/tuning/bltouch_testing.h"
#include "../screens/tuning/input_shaping.h"
#include "../screens/tuning/vibrations.h"
#include "../screens/tuning/skew.h"
#include "../screens/tuning/linear_advance.h"

#include "../screens/settings/setup.h"
#include "../screens/settings/eeprom_mismatch.h"
#include "../screens/settings/factory_reset.h"
#include "../screens/settings/sensor_settings.h"
#include "../screens/settings/brightness_settings.h"
#include "../screens/settings/beeper_settings.h"
#include "../screens/settings/pid_settings.h"
#include "../screens/settings/step_settings.h"
#include "../screens/settings/feedrate_settings.h"
#include "../screens/settings/max_acceleration_settings.h"
#include "../screens/settings/def_acceleration_settings.h"
#include "../screens/settings/runout_settings.h"
#include "../screens/settings/power_off_settings.h"
#include "../screens/settings/power_loss_settings.h"

#include "../screens/info/io.h"
#include "../screens/info/versions.h"
#include "../screens/info/statistics.h"
#include "../screens/info/copyrights.h"
#include "../screens/info/killed.h"

namespace ADVi3pp {

  // ----------------------------------------------------------------------------

  Once::operator bool() {
    if(!once_)
      return false;
    once_ = false;
    return true;
  }

  // ----------------------------------------------------------------------------
  namespace Core {

    inline namespace internals {
      enum class FLAGS: uint8_t {
        NONE = 0x00,
        INVALID_SETTINGS = 0x01
      };

      constexpr unsigned int TO_LCD_DELAY = 250; // ms
      constexpr unsigned int Z_ROOM = 20; // mm
      constexpr unsigned int ANTI_BOUNCE_DELAY = 20; // ms
      constexpr unsigned VERSION_FIELD_LENGTH = 16;
      constexpr uint16_t VARIABLE_ACTION_FIRST = 0x0600;
      constexpr uint16_t ACTION_FIRST = 0x0400;
      constexpr uint16_t ACTION_LAST = 0x04FF;
      constexpr uint16_t KEY_CODE_TEMPS = 0;
      constexpr uint16_t KEY_CODE_PRINT = 1;

      Once once_{};
      Action last_action_ = {};
      Elapse anti_bonce_{ANTI_BOUNCE_DELAY};
      Elapse to_lcd_{TO_LCD_DELAY};
      FLAGS flags_ = FLAGS::NONE;

      bool init();
      void send_gplv3_7b_notice();
      void from_lcd();
      void to_lcd();
      void process_action(Action action, uint16_t key_code, uint16_t arg);
      void send_lcd_data();
      void send_lcd_touch_request();
      void send_versions();

#ifdef ADVi3PP_DEBUG
      void debug();
#endif
    }
  }
}

ENABLE_BITMASK_OPERATOR(ADVi3pp::Core::FLAGS);

namespace ADVi3pp::Core {

  void startup() {
    Dgus::open();
    Status::set(GET_TEXT_F(ADVI3PP_MSG_BOOTING), Status::STATUS_OPTIONS::RESET | Status::STATUS_OPTIONS::PERSISTENT);
  }

  void idle() {
    static Reentrant reentrant;
    ReentrantScope scope{reentrant};
    if(!scope.reentrant()) init();

    from_lcd();
    if(to_lcd_.is_elapsed()) to_lcd();

    if(!scope.reentrant()) {
      wait_task.execute();
      status_task.execute();
      background_task.execute();
    }
  }

  void killed(const FlashChar* error, const FlashChar* component) {
    Status::set(error, Status::STATUS_OPTIONS::RESET | Status::STATUS_OPTIONS::PERSISTENT); // Set directly the status
    send_lcd_data();
    Dimming::sleep_off();
    Killed::show(component);
  }

  void killed(const FlashChar* error, heater_id_t header_id) {
    Status::set(error, Status::STATUS_OPTIONS::RESET | Status::STATUS_OPTIONS::PERSISTENT); // Set directly the status
    send_lcd_data();
    Dimming::sleep_off();
    Killed::show(header_id == ExtUI::BED ? GET_TEXT_F(MSG_BED) : GET_TEXT_F(ADVI3PP_MSG_HOTEND));
  }

  bool is_busy()  {
    return
      is_printing() ||
      ExtUI::isMoving() ||
      ExtUI::getHostKeepaliveState() == GcodeSuite::MarlinBusyState::IN_HANDLER ||
      ExtUI::getHostKeepaliveState() == GcodeSuite::MarlinBusyState::IN_PROCESS;
  }

  bool is_printing() {
    return ExtUI::isPrinting();
  }

  bool is_print_paused() {
    return ExtUI::isPrintingPaused();
  }

  bool check_not_busy() {
    if(!is_busy()) return true;
    Wait::wait_back(GET_TEXT_F(ADVI3PP_TITLE_WARNING), GET_TEXT_F(ADVI3PP_MSG_BUSY));
    return false;
  }

  void inject_commands(const FlashChar* commands) {
    Log::info() << F("inject_commands") << commands << Log::endl();
    ExtUI::injectCommands_P(from_flash(commands));
  }

  void inject_commands(const char *commands) {
    Log::info() << F("inject_commands") << commands << Log::endl();
    ExtUI::injectCommands(commands);
  }

  //! Show one of the temperature graph screens depending of the context: either the SD printing screen,
  //! the printing screen or the temperature screen.
  void show_temps() {
    display(is_printing() || is_print_paused() ? Page::Print : Page::Temperatures);
  }

  //! Show one of the Printing screens depending of the context:
  //! - If a print is running, display the Print screen
  //! - Otherwise, try to access the SD card. Depending of the result, display the SD card Page or the Temperatures page
  void show_print() {
    // If there is a print running (or paused), display the SD or USB print screen
    if(is_printing() || is_print_paused()) {
      display(Page::Print);
      return;
    }

    Status::set(GET_TEXT_F(ADVI3PP_MSG_ACCESSING_SD), Status::STATUS_OPTIONS::RESET);
    display(Page::SdCard);
  }

  void go_to_print() {
    // If already on the print page, do nothing
    if(Pages::get_current_page() == Page::Print) return;
    Pages::back_all(Pages::BACK_ALL_OPTIONS::SEND_BACK);
    display(Page::Print);
  }

  void pause_temperature() {
    SetTemperature::extruder([] (CALLBACK_SOURCE src) -> void {
      Pages::clear_current();
      if(src == CALLBACK_SOURCE::BACK) {
        ExtUI::stopPrint();
        ExtUI::setUserConfirmed(true);
        Wait::not_busy();
      }
      else {
        ExtUI::setUserConfirmed(false);
        Wait::wait();
      }
    }, SetTemperature::OPTIONS::INIT_DEFAULT);
  }

  void print_started() {
    go_to_print();
    Progress::start();
  }

  void print_paused() {
    // Nothing to do
  }

  void print_done() {
    Progress::finish();
    auto current = Pages::get_current_page();
    if(current != Page::Print && current != Page::BabySteps and current != Page::PrintOptions) return;
    Pages::back_all(Pages::BACK_ALL_OPTIONS::SEND_BACK | Pages::BACK_ALL_OPTIONS::SHOW_MAIN);
  }

  bool handle_core_command(uint16_t key_code) {
    switch (key_code) {
      case KEY_CODE_BACK:    Pages::back(Pages::BACK_OPTIONS::NONE); break;
      case KEY_CODE_TEMPS:   show_temps(); break;
      case KEY_CODE_PRINT:   show_print(); break;
      default: return false;
    }
    return true;
  }

  bool handle_show_command(Page page, uint16_t key_code) {
    if(key_code != KEY_CODE_SHOW) return false;
    Pages::show(page);
    return true;
  }

  void process(Page page, uint16_t key_code, uint16_t arg) {
    process_action(static_cast<Action>(static_cast<uint16_t>(page) + ACTION_FIRST), key_code, arg);
  }

  void display(Page page, DISPLAY_OPTIONS options, uint16_t arg) {
    if(test_one_bit(options, DISPLAY_OPTIONS::CLEAR_TEMPORARIES)) Pages::clear_temporaries();
    if(test_one_bit(options, DISPLAY_OPTIONS::BACK_ALL)) Pages::back_all(Pages::BACK_ALL_OPTIONS::NONE);
    if(test_one_bit(options, DISPLAY_OPTIONS::CLEAR_CURRENT)) Pages::clear_current();
    process(page, KEY_CODE_SHOW, arg);
  }

  //! Get current digital pin state (adapted from Atmega2560 datasheet).
  //! @param pin  Pin number to check.
  //! @return     The current state: High, Low
  PIN_STATE get_pin_state(uint8_t pin) {
    // Atmega2560 datasheet page 69, 3.2.4 Reading the Pin Value
    auto mask = digitalPinToBitMask(pin); // Bitmask for this digital pin
    auto port = digitalPinToPort(pin); // Atmega2560 I/O port for this digital pin
    if(port == NOT_A_PIN) return PIN_STATE::Low; // Never the case for the Atmega2560

    // Contrary to Arduino's code, we do not touch the time (PWM)
    auto pin_value = *portInputRegister(port); // Atmega2560 PINx (latched value) register
    return (pin_value & mask) ? PIN_STATE::High : PIN_STATE::Low;
  }

  //! Get current digital pin direction (adapted from Atmega2560 datasheet).
  //! @param pin  Pin number to check.
  //! @return     The current state: Input, Output
  PIN_DIRECTION get_pin_direction(uint8_t pin) {
    // Atmega2560 datasheet page 69, 13.2.3 Switching Between Input and Output
    uint8_t mask = digitalPinToBitMask(pin); // Bitmask for this digital pin
    uint8_t port = digitalPinToPort(pin); // Atmega2560 I/O port for this digital pin
    if(port == NOT_A_PIN) return PIN_DIRECTION::Input; // Never the case for the Atmega2560

    volatile uint8_t* ddr = portModeRegister(port); // Atmega2560 DDRx (direction) register
    return (*ddr & mask) ? PIN_DIRECTION::Output : PIN_DIRECTION::Input;
  }

  //! Get current digital pin pull (adapted from Atmega2560 datasheet).
  //! @param pin  Pin number to check.
  //! @return     The current pull: Up, Down, None
  PIN_PULL get_pin_pull(uint8_t pin) {
    uint8_t mask = digitalPinToBitMask(pin); // Bitmask for this digital pin
    uint8_t port = digitalPinToPort(pin); // Atmega2560 I/O port for this digital pin
    if(port == NOT_A_PIN) return PIN_PULL::None; // Never the case for the Atmega2560

    // Atmega2560 datasheet page 69, Table 13-1. Port Pin Configurations
    // DDRxn | PORTxn | PUD | I/O    | Comment
    // ----------------------------------------
    // 0     | 0      | x   | Input  | Hi-Z
    // 0     | 1      | 0   | Input  | Pull-up
    // 0     | 1      | 1   | Input  | Hi-Z
    // 1     | 0      | x   | Output | Low
    // 1     | 1      | x   | Output | High

    volatile uint8_t* ddr = portModeRegister(port); // Atmega2560 DDRx (direction) register
    volatile uint8_t* data = portOutputRegister(port); // Atmega2560 PORTx (data) register

    // Output?
    if(*ddr & mask)
      return (*data & mask) ? Core::PIN_PULL::High : Core::PIN_PULL::Low;

    // Input
    // Atmega2560 datasheet page 96, 13.4.1 MCUCR – MCU Control Register
    volatile uint8_t pud = MCUCR & _BV(PUD);
    return (*data & mask && !pud) ? PIN_PULL::Up : PIN_PULL::HiZ;
  }

  float ensure_z_enough_room() {
    auto previous_z = ExtUI::getAxisPosition_mm(ExtUI::Z);
    // If the Z position is low and the Z axis position is known, raise it
    if(previous_z < 20 && ExtUI::isAxisPositionKnown(ExtUI::Z))
      ExtUI::setAxisPosition_mm(Z_ROOM, ExtUI::Z, 20);
    return previous_z;
  }

  const FlashChar *get_advi3pp_version() {
    static const char advi3pp[] PROGMEM = {
        '0' + advi3_pp_version / 0x0100,
        '.',
        '0' + (advi3_pp_version % 0x100) / 0x10,
        '.',
        '0' + advi3_pp_version % 0x10,
        0
    };
    return to_flash(advi3pp);
  }

  const FlashChar *get_advi3pp_build() {
    static const char build[] PROGMEM = {
        '0' + (YEAR__ - 2000) / 10,
        '0' + (YEAR__ - 2000) % 10,
        '0' + MONTH__ / 10,
        '0' + MONTH__ % 10,
        '0' + DAY__ / 10,
        '0' + DAY__ % 10,
        '0' + HOUR__ / 10,
        '0' + HOUR__ % 10,
        '0' + MIN__ / 10,
        '0' + MIN__ % 10,
        '0' + SEC__ / 10,
        '0' + SEC__ % 10,
        0
    };
    return to_flash(build);
  }

  void on_settings_loaded(bool success) {
    if(!success) set_one_bit(flags_, FLAGS::INVALID_SETTINGS, false);
  }

  void on_settings_validated(bool success) {
    if(!success) set_one_bit(flags_, FLAGS::INVALID_SETTINGS, true);
  }

  bool are_settings_invvalid() {
    return test_one_bit(flags_, FLAGS::INVALID_SETTINGS);
  }

  void on_power_loss_set(bool set) {}

  void on_power_loss() {
    Pages::show(Page::PowerOutage);
    Status::set(GET_TEXT_F(MSG_OUTAGE_RECOVERY), Status::STATUS_OPTIONS::RESET | Status::STATUS_OPTIONS::PERSISTENT);
  }

  void on_power_loss_resume() {
    Core::display(Page::PowerLossRecovery);
  }

  void on_power_off() {
    Pages::show(Page::PowerOff);
  }

  void send_lcd_zero() {
    WriteRamRequest{Variable::Zero}.write_word(0);
  }

  // ----------------------------------------------------------------------------
  // Private functions

  inline namespace internals {

    bool init() {
      if(!once_) return false;

      Dgus::setup();
      send_gplv3_7b_notice(); // You are not authorized to remove or alter this notice
      Dimming::init();
      Graphs::clear();
      Status::init();
      Progress::reset();
      Progress::set_animation(false);
      send_lcd_zero();
      send_versions();

      #if HAS_LEVELING
      if(ExtUI::getLevelingIsValid()) ExtUI::setLevelingActive(true);
      #endif

      Log::info() << F("Init finished") << Log::endl();
      Status::set(GET_TEXT_F(WELCOME_MSG), Status::STATUS_OPTIONS::RESET);
      display(are_settings_invvalid() ? Page::EepromMismatch : Page::Main);

      return true;
    }

    void to_lcd() {
      send_lcd_data();
      Graphs::update();
      send_lcd_touch_request();
    }

    void send_gplv3_7b_notice() {
      SERIAL_ECHO_START();
      //! Note to forks author:
      //! Under GPLv3 provision 7(b), you are not authorized to remove or alter this notice.
      SERIAL_ECHOLNPGM("Based on ADVi3++, Copyright (C) 2017-2025 Sebastien Andrivet");
      SERIAL_ECHO_START();
      SERIAL_ECHOLNPGM("ADVi3++ version: ", get_advi3pp_version(), ", build: ", get_advi3pp_build(), ", flavor: ", F(ADVi3PP_NAME));
    }

    //! Send the different versions to the LCD screen.
    void send_versions() {
      char lcd_version[4];
      Dgus::get_firmware_version(lcd_version);

      WriteRamRequest{Variable::ADVi3ppVersion}.write_text(get_advi3pp_version(), VERSION_FIELD_LENGTH);
      WriteRamRequest{Variable::ADVi3ppBuild}.write_text(get_advi3pp_build(), VERSION_FIELD_LENGTH);
      WriteRamRequest{Variable::ADVi3ppDGUSVersion}.write_text(lcd_version, VERSION_FIELD_LENGTH);
      WriteRamRequest{Variable::ADVi3ppMarlinVersion}.write_text(F(SHORT_BUILD_VERSION), VERSION_FIELD_LENGTH);
      WriteRamRequest{Variable::PrinterModel}.write_text(F(PRINTER_MODEL), VERSION_FIELD_LENGTH);
      WriteRamRequest{Variable::MainboardVersion}.write_text(F(MAINBOARD_VERSION), VERSION_FIELD_LENGTH);
    }

    //! Read a frame from the LCD and act accordingly.
    void from_lcd() {
      if(Dimming::receive()) return;

      ReadAction frame{};
      if(!frame.receive()) return;

      Buzzer::buzz_on_press();
      ui.refresh_screen_timeout();

      auto action = frame.get_parameter();
      auto key_code = frame.read_uint();

      if(action == last_action_ && anti_bonce_.is_pending()) {
        Log::info() << F("Bounce detected") << Log::endl();
        return;
      }

      last_action_ = action;

      Log::verbose(true) << F("=R==> Action =") << static_cast<uint16_t>(action)
        << F("KeyCode =") << key_code << Log::endl();

       process_action(action, key_code, 0);
    }


    void send_lcd_touch_request() {
      SuspendLogging no_log{};
      Dimming::send();
    }

    //! Update the status of the printer on the LCD.
    void send_lcd_data() {
      SuspendLogging no_logging{};

      // If a PID tuning is running, display the default instead of the target temperature
      PidTuning::RUNNING pid_running = PidTuning::is_running();

      double x, y, z;
      if(is_printing()) {
        // "Normal" position when printing (to be sure there is no bad interaction)
        // It is often the target position
        x = ExtUI::getAxisPosition_mm(ExtUI::X) * 100.0;
        y = ExtUI::getAxisPosition_mm(ExtUI::Y) * 100.0;
        z = ExtUI::getAxisPosition_mm(ExtUI::Z) * 100.0;
      }
      else {
        // Real-time position
        xyz_pos_t xyz;
        ExtUI::getRealtimeAxisPositions_mm(xyz);
        x = xyz.X * 100;
        y = xyz.Y * 100;
        z = xyz.z * 100;
      }

      // Send the current status in one frame
      WriteRamRequest{Variable::HotEnd}.write_words(
        lround(ExtUI::getActualTemp_celsius(ExtUI::E0) * 10), // HotEnd
        lround(pid_running == PidTuning::RUNNING::EXTRUDER ? ExtUI::getDefaultTemp_celsius(ExtUI::H0) : ExtUI::getTargetTemp_celsius(ExtUI::E0)), // TargetHotEnd
        lround(ExtUI::getActualTemp_celsius(ExtUI::BED) * 10), // Bed
        lround(pid_running == PidTuning::RUNNING::BED ? ExtUI::getDefaultTemp_celsius(ExtUI::BED) : ExtUI::getTargetTemp_celsius(ExtUI::BED)), // TargetBed
        lround(ExtUI::getActualFan_percent(ExtUI::FAN0)), // FanSpeed
        0, // Unused
        0, // Unused
        0, // Unused
        ExtUI::getLevelingActive(), // SensorActive
        static_cast<uint16_t>(lround(ExtUI::getFeedrate_percent() * 1.0)), // Feedrate
        ExtUI::getFlow_percent(ExtUI::E0), // Flowrate
        static_cast<int16_t>(lround(x)), // X
        static_cast<int16_t>(lround(y)), // Y
        static_cast<int16_t>(lround(z))  // Z
      );

      if(is_printing()) Progress::send();
    }

    #ifdef ADVi3PP_DEBUG
    void debug() {
      Log::info() << F("Debug action") << Log::endl();
      Dgus::forwarding_loop();
    }
    #endif

    void process_action_variable(Variable variable, uint16_t value) {
      Log::info() << F("Process action variable:") << static_cast<uint16_t>(variable) << F("value:") << value << Log::endl();
      switch (variable) {
        case Variable::BeepDuration:      BeeperSettings::handle_duration_command(value); break;
        case Variable::NormalBrightness:  BrightnessSettings::handle_normal_brightness_command(value); break;
        case Variable::DimmingBrightness: BrightnessSettings::handle_dimming_brightness_command(value); break;
        default: Log::error() << F("Invalid variable") << static_cast<uint16_t>(variable) << Log::endl(); break;
      }
    }

    void process_action(Action action, uint16_t key_code, uint16_t arg) {
      if(static_cast<uint16_t>(action) >= VARIABLE_ACTION_FIRST) {
        process_action_variable(static_cast<Variable>(action), key_code);
        return;
      }

      if(static_cast<uint16_t>(action) < ACTION_FIRST || static_cast<uint16_t>(action) > ACTION_LAST) {
        Log::error() << F("Invalid action") << static_cast<uint16_t>(action) << Log::endl();
        return;
      }

      Page page = static_cast<Page>(static_cast<uint16_t>(action) - ACTION_FIRST);
      Log::info() << F("Process action:") << static_cast<uint16_t>(action) << F("Page:") << page << F("code:") << key_code << Log::endl();

      bool handled = false;
      switch(page) {
        case Page::None:                    handled = handle_core_command(key_code); break;
        case Page::Main:
        case Page::Controls:
        case Page::Tuning:
        case Page::Infos:
        case Page::Motors:                  handled = handle_show_command(page, key_code); break;

#if ENABLED(ADVi3PP_PROBE)
        case Page::Settings:                handled = handle_show_command(Page::Settings, key_code); break;
#else
        case Page::Settings:                handled = handle_show_command(Page::SettingsNoSensor, key_code); break;
#endif

        case Page::LoadUnload:              handled = LoadUnload::handle_command(key_code); break;
        case Page::Preheat:                 handled = Preheat::handle_command(key_code); break;
        case Page::Move:                    handled = Move::handle_command(key_code); break;
        case Page::ExtruderTemperature:
        case Page::BedTemperature:          handled = SetTemperature::handle_command(key_code); break;

        case Page::RoughLeveling:           handled = RoughLeveling::handle_command(key_code); break;
        case Page::AutomaticMesh:
        case Page::ManualMesh:              handled = Mesh::handle_command(key_code); break;
#if ENABLED(ADVi3PP_PROBE)
        case Page::Leveling:                handled = handle_show_command(Page::Leveling, key_code); break;
        case Page::ZHeightTuning:           handled = SensorZHeight::handle_command(key_code); break;
#else
        case Page::Leveling:                handled = handle_show_command(Page::LevelingManual, key_code); break;
#endif
        case Page::SensorGrid:              handled = Grid::handle_command(key_code); break;
        case Page::XTwist:                  handled = XTwist::handle_command(key_code); break;

        case Page::SdCard:                  handled = SdCard::handle_command(key_code); break;
        case Page::Print:                   handled = Print::handle_command(key_code); break;
        case Page::BabySteps:               handled = BabySteps::handle_command(key_code); break;
        case Page::PrintOptions:            handled = PrintOptions::handle_command(key_code); break;
        case Page::Temperatures:            handled = Temperatures::handle_command(key_code); break;
        case Page::PowerLossRecovery:       handled = PowerLossRecovery::handle_command(key_code); break;
        case Page::PauseOptions:            handled = PauseOptions::handle_command(key_code); break;

        case Page::PidTuning:               handled = PidTuning::handle_command(key_code); break;
        case Page::VibrationsTuning:        handled = Vibrations::handle_command(key_code); break;
#ifdef BLTOUCH
        case Page::BlTouchTesting1:         handled = BlTouchTesting::handle_command(key_code); break;
        case Page::SensorSettings:          handled = SensorSettings::handle_command(key_code); break;
#endif
        case Page::ExtruderTuning1:         handled = ExtruderTuning::handle_command(key_code); break;
        case Page::Skew1:                   handled = Skew::handle_command(key_code); break;
        case Page::LinearAdvance:           handled = LinearAdvance::handle_command(key_code); break;

        case Page::FactoryReset:            handled = FactoryReset::handle_command(key_code); break;
        case Page::PidSettings:             handled = PidSettings::handle_command(key_code, arg); break;
        case Page::StepsSettings:           handled = StepSettings::handle_command(key_code); break;
        case Page::FeedrateSettings:        handled = FeedrateSettings::handle_command(key_code); break;
        case Page::MaxAccelerationSettings: handled = MaxAccelerationSettings::handle_command(key_code); break;
        case Page::DefAccelerationSettings: handled = DefAccelerationSettings::handle_command(key_code); break;
        case Page::BrightnessSettings:      handled = BrightnessSettings::handle_command(key_code); break;
        case Page::BeeperSettings:          handled = BeeperSettings::handle_command(key_code); break;
        case Page::PowerOffSettings:        handled = PowerOffSettings::handle_command(key_code); break;
#if ENABLED(POWER_LOSS_RECOVERY)
        case Page::PowerLossSettings:       handled = PowerLossSettings::handle_command(key_code); break;
#endif
        case Page::InputShaping:            handled = InputShaping::handle_command(key_code); break;
        case Page::RunoutSettings:          handled = RunoutSettings::handle_command(key_code); break;

        case Page::Statistics:              handled = Statistics::handle_command(key_code); break;
        case Page::Versions:                handled = Versions::handle_command(key_code); break;
        case Page::Copyrights:              handled = Copyrights::handle_command(key_code); break;
        case Page::IO_51:                   handled = Io::handle_command(key_code); break;
        case Page::IO_EXT:                  handled = IoExt::handle_command(key_code); break;

        case Page::Wait:                    handled = Wait::handle_command(key_code); break;
        case Page::Setup:
        case Page::SetupNoSensor:           handled = Setup::handle_command(key_code); break;
        case Page::EepromMismatch:          handled = EepromMismatch::handle_command(key_code); break;

        default:
          if(key_code != KEY_CODE_BACK) {
            Log::error() << F("Invalid action") << static_cast<uint16_t>(action) << Log::endl();
            return;
          }

          Log::info() << F("Fallback for KEY_CODE_BACK") << Log::endl();
          Pages::back(Pages::BACK_OPTIONS::NONE);
          handled = true;
          break;
      }

      if(handled) return;

      if(key_code == KEY_CODE_BACK) {
        Log::info() << F("Fallback for KEY_CODE_BACK") << Log::endl();
        Pages::back(Pages::BACK_OPTIONS::NONE);
      }
      else
        invalid_key_code(page, key_code);
    }
  }
}
