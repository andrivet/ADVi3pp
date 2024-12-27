/**
 * ADVi3++ Firmware For Wanhao Duplicator i3 Plus (based on Marlin)
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

#pragma once

#include <stdint.h>
#include "../lib/ADVstd/bitmasks.h"

namespace ADVi3pp {

  enum class Page: uint8_t {
    None                      = 0,    // 0x00
    Main                      = 2,    // 0x02
    Controls                  = 4,    // 0x04
    Leveling                  = 6,    // 0x06
    Tuning                    = 8,    // 0x08
    Settings                  = 10,   // 0x0A
    Infos                     = 12,   // 0x0C
    Motors                    = 14,   // 0x0E
    LevelingManual            = 16,   // 0x10
    SettingsNoSensor          = 18,   // 0x12

    LoadUnload                = 20,   // 0x14
    Preheat                   = 22,   // 0x16
    Move                      = 24,   // 0x18
    ExtruderTemperature       = 26,   // 0x1A
    BedTemperature            = 28,   // 0x1C

    RoughLeveling             = 40,   // 0x28
    AutomaticMesh             = 42,   // 0x2A
    SensorGrid                = 44,   // 0x2C
    ZHeightTuning             = 46,   // 0x2E
    XTwist                    = 48,   // 0x30
    ManualMesh                = 50,   // 0x32

    SdCard                    = 60,   // 0x3C
    Print                     = 62,   // 0x3E
    BabySteps                 = 64,   // 0x40
    PrintOptions              = 66,   // 0x42
    Temperatures              = 68,   // 0x44
    // Unused                 = 70,   // 0x46
    PowerLossRecovery         = 72,   // 0x48
    PauseOptions              = 74,   // 0x4A

    PidTuning                 = 80,   // 0x50
    VibrationsTuning          = 82,   // 0x52
    BlTouchTesting1           = 84,   // 0x54
    BlTouchTesting2           = 86,   // 0x56
    BlTouchTesting3           = 88,   // 0x58
    BlTouchTesting4           = 90,   // 0x5A
    BlTouchTesting5           = 92,   // 0x5C
    ExtruderTuning1           = 94,   // 0x5E
    ExtruderTuning2           = 96,   // 0x60
    ExtruderTuning3           = 98,   // 0x62
    Skew1                     = 100,  // 0x64
    Skew2                     = 102,  // 0x66
    Skew3                     = 104,  // 0x68

    FactoryReset              = 120,  // 0x78
    PidSettings               = 122,  // 0x7A
    StepsSettings             = 124,  // 0x7C
    FeedrateSettings          = 126,  // 0x7E
    MaxAccelerationSettings   = 128,  // 0x80
    DefAccelerationSettings   = 130,  // 0x82
    SensorSettings            = 132,  // 0x84
    BrightnessSettings        = 134,  // 0x86
    BeeperSettings            = 136,  // 0x88
    LinearAdvance             = 138,  // 0x8A - Moved to Tuning
    PowerOffSettings          = 140,  // 0x8C
    PowerLossSettings         = 142,  // 0x8E
    InputShaping              = 144,  // 0x90
    RunoutSettings            = 146,  // 0x92

    Statistics                = 160,  // 0xA0
    Versions                  = 162,  // 0xA2
    Copyrights                = 164,  // 0xA4
    IO_51                     = 166,  // 0xA6
    IO_52                     = 168,  // 0xA8
    IO_EXT                    = 170,  // 0xAA

    Wait                      = 180,  // 0xB4
    WaitBack                  = 182,  // 0xB6
    WaitContinue              = 184,  // 0xB8
    WaitBackContinue          = 186,  // 0xBA
    Setup                     = 188,  // 0xBC
    SetupNoSensor             = 190,  // 0xBE
    SetupNoBlTouch            = 192,  // 0xC0
    EepromMismatch            = 194,  // 0xC2
    Killed                    = 196,  // 0xC4
    PowerOutage               = 198,  // 0xC6
    PowerOff                  = 200,  // 0xC8

    KeyboardUnsignedInteger   = 220,  // 0xDC
    KeyboardUnsignedDecimal   = 222,  // 0xDE
    KeyboardSignedInteger     = 224,  // 0xE0
    KeyboardSignedDecimal     = 226,  // 0xE2

    Boot                      = 240   // 0xF0
  };


  //! List of variables and their addresses.
  enum class Variable: uint16_t {
    // 0 - Statuses
    HotEnd                  = 0x0000,
    TargetHotEnd            = 0x0001,
    Bed                     = 0x0002,
    TargetBed               = 0x0003,
    FanSpeed                = 0x0004,
    SensorActive            = 0x0008,
    Feedrate                = 0x0009,
    Flowrate                = 0x000A,

    X                       = 0x000B,
    Y                       = 0x000C,
    Z                       = 0x000D,

    Message                 = 0x0010,
    CenteredMessage         = 0x0020,
    FilePrinted             = 0x0030,
    ET                      = 0x0040,
    TC                      = 0x0044,

    ProgressPercent         = 0x0050,
    ProgressLow             = 0x0051,
    ProgressHigh            = 0x0052,
    WaitAnimation           = 0x0053,
    Zero                    = 0x0054, // Always 0x00

    // 1 - Short Texts
    ShortText0              = 0x0100,
    ShortText1              = 0x0108,
    ShortText2              = 0x0110,
    ShortText3              = 0x0118,
    ShortText4              = 0x0120,

    // 2 - Long Texts
    LongText0               = 0x0200,
    LongText1               = 0x0210,
    LongText2               = 0x0220,
    LongText3               = 0x0230,
    LongText4               = 0x0240,
    LongTextCentered0       = 0x0250,

    // 3 - Values
    Value0                  = 0x0300,
    Value1                  = 0x0301,
    Value2                  = 0x0302,
    Value3                  = 0x0303,
    Value4                  = 0x0304,
    Value5                  = 0x0305,
    Value6                  = 0x0306,
    Value7                  = 0x0307,
    Value8                  = 0x0308,
    Value9                  = 0x0309,
    ValueA                  = 0x030A,
    Value16                 = 0x0310,
    Value32                 = 0x0320,
    Value48                 = 0x0330,

    // 5 - Versions
    ADVi3ppVersion          = 0x0500,
    ADVi3ppBuild            = 0x0508,
    ADVi3ppDGUSVersion      = 0x0510,
    ADVi3ppMarlinVersion    = 0x0518,
    PrinterModel            = 0x0520,
    MainboardVersion        = 0x0528,

    // 6 - Variables used for actions
    BeepDuration            = 0x0600,
    NormalBrightness        = 0x0601,
    DimmingBrightness       = 0x0602
  };

  constexpr size_t SHORT_TEXT_LENGTH = 16;
  constexpr size_t LONG_TEXT_LENGTH = 32;

  constexpr uint16_t KEY_CODE_SHOW = 0;
  constexpr uint16_t KEY_CODE_BACK = 0xFFFF;
  constexpr uint16_t KEY_CODE_SAVE = 0xFFFE;
}

ENABLE_BITMASK_OPERATOR(ADVi3pp::Page);
