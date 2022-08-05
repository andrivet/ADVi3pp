#pragma once

#if !defined(__AVR_ATmega1280__) && !defined(__AVR_ATmega2560__)
  #error "Oops!  Make sure you have 'Arduino Mega' selected from the 'Tools -> Boards' menu."
#endif

#define BOARD_NAME "I3PLUS3030"

#define LARGE_FLASH true
#define X_STEP_PIN         61
#define X_DIR_PIN          62
#define X_ENABLE_PIN       60
#define X_MIN_PIN          54
#define Y_STEP_PIN         64
#define Y_DIR_PIN          65
#define Y_ENABLE_PIN       2
#define Y_MIN_PIN          24
#define Z_STEP_PIN         67
#define Z_DIR_PIN          69
#define Z_ENABLE_PIN       66
#define Z_MIN_PIN          6
#define E0_STEP_PIN        58 
#define E0_DIR_PIN         59 
#define E0_ENABLE_PIN      57 
#define FAN_PIN            5
#define KILL_PIN           -1
#define FAN_SOFT_PWM
#define HEATER_0_PIN       4
#define HEATER_1_PIN       -1
#define HEATER_2_PIN       -1
#define HEATER_BED_PIN     3
#define TEMP_0_PIN         1    
#define TEMP_1_PIN         -1   
#define TEMP_2_PIN         -1   
#define TEMP_BED_PIN       14   
#define SD_DETECT_PIN       49
#define SERVO0_PIN         7
#define SDSS               53
#define LED_PIN            13
#ifndef FILWIDTH_PIN
#define FILWIDTH_PIN       12
#endif
#if ENABLED(ULTRA_LCD)
#define LCD_PINS_RS         37
#define LCD_PINS_ENABLE     36
#define LCD_PINS_D4         34
#define LCD_PINS_D5         35
#define LCD_PINS_D6         32
#define LCD_PINS_D7         33
#define BEEPER_PIN        27
#define BTN_EN2           28
#define BTN_EN1           29
#define BTN_ENC           30
#endif
#if ENABLED(FILAMENT_RUNOUT_SENSOR)
  #define FIL_RUNOUT_PIN     8
#endif
#ifdef FYS_SAFE_PRINT_BREAK
#define POW_BREAK_CHECK_PIN         63  
#endif
