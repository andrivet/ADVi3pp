/**
 * ADVi3++ Firmware For Wanhao Duplicator i3 Plus (based on Marlin)
 *
 * Copyright (C) 2017-2022 Sebastien Andrivet [https://github.com/andrivet/]
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

enum class Page: uint16_t {
    Temporary               = 0x8000,
    EnsureNoMove            = 0x4000,
    PageNumber              = 0x00FF,

    None                    = 0,
    Main                    = 22,                             // 0x0016
    Controls                = 24,                             // 0x0018
    Tuning                  = 26,                             // 0x001A
    Settings                = 28,                             // 0x001C
    LoadUnload              = 30 | EnsureNoMove,              // 0x401E
    WaitBack                = 32 | Temporary,                 // 0x8020
    WaitBackContinue        = 34 | Temporary,                 // 0x8022
    Preheat                 = 36 | EnsureNoMove,              // 0x0024
    Move                    = 38 | EnsureNoMove,              // 0x0026
    SdCard                  = 40 | EnsureNoMove | Temporary,  // 0xC028
    Print                   = 42,                             // 0x002A
    BabySteps               = 44,                             // 0x002C
    Waiting                 = 46 | Temporary,                 // 0x802E
    ManualLeveling          = 48 | EnsureNoMove,              // 0x4030
    ExtruderTuningTemp      = 50 | EnsureNoMove,              // 0x4032
    WaitContinue            = 52 | Temporary,                 // 0x8034
    ExtruderTuningMeasure   = 54 | EnsureNoMove,              // 0x4036
    Leveling                = 56 | EnsureNoMove,              // 0x4038
    PidTuning               = 58 | EnsureNoMove,              // 0x403A
    Killed                  = 60 | Temporary,                 // 0x803C
    MotorsSettings          = 62,                             // 0x003E
    PidSettings             = 64,                             // 0x0040
    FactoryReset            = 66 | Temporary,                 // 0x8042
    Statistics              = 68,                             // 0x0044
    Versions                = 70,                             // 0x0046
    StepsSettings           = 72,                             // 0x0048
    FeedrateSettings        = 74,                             // 0x004A
    AccelerationSettings    = 76,                             // 0x004C
    PauseOptions            = 78 | Temporary,                 // 0x804E
    PrintSettings           = 80 | EnsureNoMove,              // 0x4050
    Setup                   = 82,                             // 0x0052
    SetupNoSensor           = 84,                             // 0x0054
    Temperature             = 86,                             // 0x0056
    Infos                   = 88,                             // 0x0058
    VibrationsTuning        = 90 | EnsureNoMove,              // 0x405A
    NoSensor                = 92 | Temporary,                 // 0x805C
    SensorSettings          = 94,                             // 0x005E
    LCD                     = 96,                             // 0x0060
    Copyrights              = 98,                             // 0x0062
    KeyboardInteger         = 100,                            // 0x0064
    SensorGrid              = 102,                            // 0x0066
    EEPROMMismatch          = 104 | Temporary,                // 0x8068
    ZHeightTuning           = 106 | EnsureNoMove,             // 0x406A
    LinearAdvanceTuning     = 108 | EnsureNoMove,             // 0x406C
    LinearAdvanceSettings   = 110,                            // 0x006E
    IO                      = 112,                            // 0x0070
    BLTouchTesting1         = 114 | Temporary,                // 0x8072
    BLTouchTesting2         = 116 | Temporary,                // 0x8074
    BLTouchTesting3         = 118 | Temporary,                // 0x8076
    BLTouchTesting4         = 120 | EnsureNoMove | Temporary, // 0xC078
    KeyboardDecimal         = 122,                            // 0x007A
    KeyboardSignedInteger   = 124,                            // 0x007C
    KeyboardSignedDecimal   = 126,                            // 0x007E
    LevelingNoSensor        = 128,                            // 0x0080
    ManualMesh              = 130,                            // 0x0082
    XTwist                  = 132 | EnsureNoMove,             // 0x4084
    Runout                  = 134,                            // 0x0086
    Skew1Settings           = 136,                            // 0x0088
    Skew2Settings           = 138,                            // 0x008A
    Skew3Settings           = 140,                            // 0x008C
    BuzzerSettings          = 142,                            // 0x008E
    AutomaticLeveling       = 144 | EnsureNoMove,             // 0x408F

    Boot                    = 200 | Temporary                 // 0x80C8
};
ENABLE_BITMASK_OPERATOR(Page);


//! List of variables and their addresses.
enum class Variable: uint16_t
{
    // 0 - Statuses
    TargetBed               = 0x0000,
    Bed                     = 0x0001,
    TargetHotEnd            = 0x0002,
    HotEnd                  = 0x0003,
    FanSpeed                = 0x0004,
    ZHeight                 = 0x0005,
    ProgressLow             = 0x0006,
    ProgressHigh            = 0x0007,
    ZLayer                  = 0x0008,
    SensorActive            = 0x0009,
    Feedrate                = 0x000A,
    Flowrate                = 0x000B,
    Message                 = 0x0010,
    CenteredMessage         = 0x0028,
    ProgressText            = 0x0040,
    ProgressPercent         = 0x0058,
    ET                      = 0x005C,
    TC                      = 0x0060,

    // 1 - Short Texts
    ShortText0              = 0x0100,
    ShortText1              = 0x0108,
    ShortText2              = 0x0110,
    ShortText3              = 0x0118,
    ShortText4              = 0x0120,

    // 2 - Long Texts
    LongText0               = 0x0200,
    LongText1               = 0x0218,
    LongText2               = 0x0230,
    LongText3               = 0x0248,
    LongText4               = 0x0260,
    LongTextCentered0       = 0x0278,

    // 3 - Values
    Value0                  = 0x0300,
    Value1                  = 0x0301,
    Value2                  = 0x0302,
    Value3                  = 0x0303,
    Value4                  = 0x0304,
    Value5                  = 0x0305,
    Value6                  = 0x0306,
    Value7                  = 0x0307,

    // 5 - Versions
    ADVi3ppVersion          = 0x0500,
    ADVi3ppBuild            = 0x0508,
    ADVi3ppDGUSVersion      = 0x0510,
    ADVi3ppMarlinVersion    = 0x0518,

    BeepDuration            = 0x0617,
    NormalBrightness        = 0x0618,
    DimmingBrightness       = 0x0619
};

//! List of actions sent by the LCD.
enum class Action: uint16_t
{
    None                    = 0,
    // 4 - Actions
    Controls                = 0x0400,
    PrintCommand            = 0x0401,
    Wait                    = 0x0402,
    LoadUnload              = 0x0403,
    Preheat                 = 0x0404,
    Move                    = 0x0405,
    SdCard                  = 0x0406,
    FactoryReset            = 0x0407,
    ManualLeveling          = 0x0408,
    ExtruderTuning          = 0x0409,
    AutomaticLeveling       = 0x040A,
    PidTuning               = 0x040B,
    SensorSettings          = 0x040C,
    VibrationsTuning        = 0x040D,
    NoSensor                = 0x040E,
    LCD                     = 0x040F,
    Statistics              = 0x0410,
    Versions                = 0x0411,
    PrintSettings           = 0x0412,
    PIDSettings             = 0x0413,
    StepsSettings           = 0x0414,
    FeedrateSettings        = 0x0415,
    AccelerationSettings    = 0x0416,
    PauseOptions            = 0x0417,
    Copyrights              = 0x0418,
    //unused                = 0x0419,
    SensorGrid              = 0x041A,
    SensorZHeight           = 0x041B,
    ChangeFilament          = 0x041C,
    EEPROMMismatch          = 0x041D,
    USB2LCD                 = 0x041E,
    BLTouchTesting          = 0x041F,
    LinearAdvanceSettings   = 0x0420,
    Diagnosis               = 0x0421,
    Temperatures            = 0x0422,
    Setup                   = 0x0423,
    XTwist                  = 0x0424,
    Runout                  = 0x0425,
    Skew                    = 0x0426,
    BeeperSettings          = 0x0427,
    BabySteps               = 0x0428,

    // 6 - Moves
    MoveXMinus              = 0x0600,
    MoveXPlus               = 0x0601,
    MoveYMinus              = 0x0602,
    MoveYPlus               = 0x0603,
    MoveZMinus              = 0x0604,
    MoveZPlus               = 0x0605,
    MoveEMinus              = 0x0606,
    MoveEPlus               = 0x0607,
    BabyMinus               = 0x0608,
    BabyPlus                = 0x0609,
    ZHeightMinus            = 0x060A,
    ZHeightPlus             = 0x060B,
    // UNUSED               = 0x060C,
    FeedrateMinus           = 0x060D,
    FeedratePlus            = 0x060E,
    FanMinus                = 0x060F,
    FanPlus                 = 0x0610,
    HotendMinus             = 0x0611,
    HotendPlus              = 0x0612,
    BedMinus                = 0x0613,
    BedPlus                 = 0x0614,
    XTwistMinus             = 0x0615,
    XTwistPlus              = 0x0616,
    BeepDuration            = 0x0617,
    NormalBrightness        = 0x0618,
    DimmingBrightness       = 0x0619,
    FlowrateMinus           = 0x061A,
    FlowratePlus            = 0x061B,

    Undefined                = 0xFFFF
};

//! Key values used by the LCD screens.
enum class KeyValue: uint16_t
{
    Show                    = 0x0000,

    Temps                   = 0x0000,
    Print                   = 0x0001,
    Controls                = 0x0002,
    Tuning                  = 0x0003,
    Settings                = 0x0004,
    Infos                   = 0x0005,
    Motors                  = 0x0006,
    Leveling                = 0x0007,
    PrintSettings           = 0x0008,
    BabySteps               = 0x0009,

    PrintStop               = 0x0001,
    PrintPause              = 0x0002,
    PrintAdvancedPause      = 0x0003,

    Load                    = 0x0001,
    Unload                  = 0x0002,

    PresetPrevious          = 0x0001,
    PresetNext              = 0x0002,
    Cooldown                = 0x0003,

    MoveXHome               = 0x0001,
    MoveYHome               = 0x0002,
    MoveZHome               = 0x0003,
    MoveAllHome             = 0x0004,
    DisableMotors           = 0x0005,

    SDLine1                 = 0x0001,
    SDLine2                 = 0x0002,
    SDLine3                 = 0x0003,
    SDLine4                 = 0x0004,
    SDLine5                 = 0x0005,
    SDUp                    = 0x0006,
    SDDown                  = 0x0007,
    SDParent                = 0x0008,

    LevelingPoint1          = 0x0001,
    LevelingPoint2          = 0x0002,
    LevelingPoint3          = 0x0003,
    LevelingPoint4          = 0x0004,
    LevelingPoint5          = 0x0005,
    LevelingPointA          = 0x0006,
    LevelingPointB          = 0x0007,
    LevelingPointC          = 0x0008,
    LevelingPointD          = 0x0009,

    LevelingTest            = 0x0001,
    LevelingResetProbe      = 0x0002,

    TuningStart             = 0x0001,
    TuningSettings          = 0x0002,

    PidTuningStep2          = 0x0001,
    PidTuningHotend         = 0x0002,
    PidTuningBed            = 0x0003,

    SensorSettingsPrevious  = 0x0001,
    SensorSettingsNext      = 0x0002,
    SensorSettingsHighSpeed = 0x0003,

    LCDDimming              = 0x0001,

    BuzzOnAction            = 0x0001,
    BuzzOnPress             = 0x0002,

    MismatchForward         = 0x0001,

    PidSettingsHotend       = 0x0001,
    PidSettingsBed          = 0x0002,
    PidSettingPrevious      = 0x0003,
    PidSettingNext          = 0x0004,

    SensorSelfTest          = 0x0001,
    SensorReset             = 0x0002,
    SensorDeploy            = 0x0003,
    SensorStow              = 0x0004,

    Multiplier1             = 0x0001,
    Multiplier2             = 0x0002,
    Multiplier3             = 0x0003,
    Point_L                 = 0x0004,
    Point_M                 = 0x0005,
    Point_R                 = 0x0006,

    Baby1                   = 0x0001,
    Baby2                   = 0x0002,
    Baby3                   = 0x0003,

    Extrude                 = 0x0001,
    Resume                  = 0x0002,

    BLTouchTestingStep1Yes  = 0x0001,
    BLTouchTestingStep1No   = 0x0002,
    BLTouchTestingStep2Yes  = 0x0003,
    BLTouchTestingStep2No   = 0x0004,
    BLTouchTestingStep3Yes  = 0x0005,
    BLTouchTestingStep3No   = 0x0006,

    VibrationsX             = 0x0001,
    VibrationsY             = 0x0002,
    VibrationsXY            = 0x0003,
    VibrationsYX            = 0x0004,
    VibrationsZ             = 0x0005,

    RunoutEnable            = 0x0001,
    RunoutHigh2Low          = 0x0002,
    RunoutLow2High          = 0x0003,
    RunoutLED               = 0x0004,
    RunoutDistance          = 0x0005,

    SkewStep2               = 0x0001,
    SkewStep3               = 0x0002,

    Save                    = 0xFFFE,
    Back                    = 0xFFFF
};


}