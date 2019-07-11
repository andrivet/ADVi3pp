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
#ifndef ADV_I3_PLUS_PLUS_ENUMS_H
#define ADV_I3_PLUS_PLUS_ENUMS_H

#include <stdint.h>
#include "advi3pp_bitmasks.h"

namespace advi3pp {


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
    Babysteps               = 0x000B,
    Message                 = 0x0010,
    CenteredMessage         = 0x0028,
    Progress                = 0x0040,

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
    ADVi3ppLCDVersion_Raw       = 0x0500,
    ADVi3ppMotherboardVersion   = 0x0501,
    ADVi3ppMotherboardBuild     = 0x0509,
    ADVi3ppLCDVersion           = 0x0511,
    ADVi3ppDGUSVersion          = 0x0519,
    ADVi3ppMarlinVersion        = 0x0521
};

//! List of actions sent by the LCD.
enum class Action: uint16_t
{
    // 4 - Actions
    Screen                  = 0x0400,
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
    Firmware                = 0x040D,
    NoSensor                = 0x040E,
    LCD                     = 0x040F,
    Statistics              = 0x0410,
    Versions                = 0x0411,
    PrintSettings           = 0x0412,
    PIDSettings             = 0x0413,
    StepsSettings           = 0x0414,
    FeedrateSettings        = 0x0415,
    AccelerationSettings    = 0x0416,
    JerkSettings            = 0x0417,
    Copyrights              = 0x0418,
    SensorTuning            = 0x0419,
    SensorGrid              = 0x041A,
    SensorZHeight           = 0x041B,
    ChangeFilament          = 0x041C,
    EEPROMMismatch          = 0x041D,
    Sponsors                = 0x041E,
    LinearAdvanceTuning     = 0x041F,
    LinearAdvanceSettings   = 0x0420,
    Diagnosis               = 0x0421,
    Temperatures            = 0x0422,
    VersionsMismatch        = 0x0423,

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
    LCDBrightness           = 0x060C,
    FeedrateMinus           = 0x060D,
    FeedratePlus            = 0x060E,
    FanMinus                = 0x060F,
    FanPlus                 = 0x0610,
    HotendMinus             = 0x0611,
    HotendPlus              = 0x0612,
    BedMinus                = 0x0613,
    BedPlus                 = 0x0614,

    Undefined               = 0xFFFF
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

    LevelingPoint1          = 0x0001,
    LevelingPoint2          = 0x0002,
    LevelingPoint3          = 0x0003,
    LevelingPoint4          = 0x0004,
    LevelingPoint5          = 0x0005,
    LevelingPointA          = 0x0006,
    LevelingPointB          = 0x0007,
    LevelingPointC          = 0x0008,
    LevelingPointD          = 0x0009,

    TuningStart             = 0x0001,
    TuningSettings          = 0x0002,

    PidTuningStep2          = 0x0001,
    PidTuningHotend         = 0x0002,
    PidTuningBed            = 0x0003,

    SensorSettingsPrevious  = 0x0001,
    SensorSettingsNext      = 0x0002,

    ThermalProtection       = 0x0001,
    RunoutSensor            = 0x0002,
    USBBaudrateMinus        = 0x0003,
    USBBaudratePlus         = 0x0004,

    LCDDimming              = 0x0001,
    BuzzerOnAction          = 0x0002,
    BuzzOnPress             = 0x0003,

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

    Baby1                   = 0x0001,
    Baby2                   = 0x0002,
    Baby3                   = 0x0003,

    Save                    = 0xFFFE,
    Back                    = 0xFFFF
};

//! Set of possible feature to enable or disable
enum class Feature: uint16_t
{
    None                = 0b0000000000000000,
    HeadParking         = 0b0000000000000001,
    ThermalProtection   = 0b0000000000000010,
    Dimming             = 0b0000000000000100,
    Buzzer              = 0b0000000000001000,
    BuzzOnPress         = 0b0000000000010000,
    RunoutSensor        = 0b0000000000100000
};
ENABLE_BITMASK_OPERATOR(Feature);

}

#endif //ADV_I3_PLUS_PLUS_ENUMS_H
