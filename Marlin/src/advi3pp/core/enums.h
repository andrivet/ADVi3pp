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
  Temporary               = 0x8000, // Temporary screen
  ExitFinishMove          = 0x4000, // Wait end of moved when exiting
  EnterNoPrint            = 0x2000, // Forbidden to enter if printing
  PageNumber              = 0x00FF, // Mask for the page number

  None                    = 0,
  Main                    = 22,
  Controls                = 24,
  Tuning                  = 26,
  Settings                = 28,
  LoadUnload              = 30 | EnterNoPrint | ExitFinishMove,
  WaitBack                = 32 | Temporary,
  WaitBackContinue        = 34 | Temporary,
  Preheat                 = 36 | EnterNoPrint | ExitFinishMove,
  Move                    = 38 | EnterNoPrint | ExitFinishMove,
  SdCard                  = 40 | ExitFinishMove | Temporary,
  Print                   = 42,
  BabySteps               = 44,
  Waiting                 = 46 | Temporary,
  ManualLeveling          = 48 | EnterNoPrint | ExitFinishMove,
  ExtruderTuning          = 50 | EnterNoPrint | ExitFinishMove,
  WaitContinue            = 52 | Temporary,
  ExtruderTuningMeasure   = 54 | ExitFinishMove,
  Leveling                = 56,
  PidTuning               = 58 | EnterNoPrint | ExitFinishMove,
  Killed                  = 60 | Temporary,
  MotorsSettings          = 62,
  PidSettings             = 64 | EnterNoPrint,
  FactoryReset            = 66 | EnterNoPrint | Temporary,
  Statistics              = 68,
  Versions                = 70,
  StepsSettings           = 72 | EnterNoPrint,
  FeedrateSettings        = 74 | EnterNoPrint,
  AccelerationSettings    = 76 | EnterNoPrint,
  PauseOptions            = 78 | Temporary,
  PrintSettings           = 80,
  Setup                   = 82,
  SetupNoSensor           = 84,
  Temperature             = 86,
  Infos                   = 88,
  VibrationsTuning        = 90 | EnterNoPrint | ExitFinishMove,
  NoSensor                = 92 | Temporary,
  SensorSettings          = 94 | EnterNoPrint,
  LCD                     = 96,
  Copyrights              = 98,
  KeyboardInteger         = 100,
  SensorGrid              = 102,
  EEPROMMismatch          = 104 | Temporary,
  ZHeightTuning           = 106 | EnterNoPrint | ExitFinishMove,
  LinearAdvanceTuning     = 108 | EnterNoPrint | ExitFinishMove,
  LinearAdvanceSettings   = 110 | EnterNoPrint,
  IO                      = 112,
  BLTouchTesting1A        = 114 | EnterNoPrint | Temporary,
  BLTouchTesting2         = 116 | Temporary,
  BLTouchTesting3         = 118 | Temporary,
  BLTouchTesting4         = 120 | ExitFinishMove | Temporary,
  KeyboardDecimal         = 122,
  KeyboardSignedInteger   = 124,
  KeyboardSignedDecimal   = 126,
  LevelingNoSensor        = 128,
  ManualMesh              = 130,
  XTwist                  = 132 | EnterNoPrint | ExitFinishMove,
  Runout                  = 134,
  Skew                    = 136 | EnterNoPrint | ExitFinishMove,
  Skew2                   = 138 | EnterNoPrint | ExitFinishMove,
  Skew3                   = 140 | EnterNoPrint | ExitFinishMove,
  BuzzerSettings          = 142,
  AutomaticLeveling       = 144 | EnterNoPrint | ExitFinishMove,
  BLTouchTesting1B        = 146 | EnterNoPrint | Temporary,

  Boot                    = 200 | Temporary
};
ENABLE_BITMASK_OPERATOR(Page);


//! List of variables and their addresses.
enum class Variable: uint16_t {
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
enum class Action: uint16_t {
  None                    = 0,
  // 4 - Actions
  Controls                = 0x0400,
  Print                   = 0x0401,
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
  PidSettings             = 0x0413,
  StepsSettings           = 0x0414,
  FeedrateSettings        = 0x0415,
  AccelerationSettings    = 0x0416,
  PauseOptions            = 0x0417,
  Copyrights              = 0x0418,
  //unused                = 0x0419,
  SensorGrid              = 0x041A,
  ZHeightTuning           = 0x041B,
  ChangeFilament          = 0x041C, // Not used
  EEPROMMismatch          = 0x041D, // Not used
  USB2LCD                 = 0x041E,
  BLTouchTesting          = 0x041F,
  LinearAdvanceSettings   = 0x0420,
  IO                      = 0x0421,
  Temperatures            = 0x0422,
  Setup                   = 0x0423,
  XTwist                  = 0x0424,
  Runout                  = 0x0425,
  Skew                    = 0x0426,
  BuzzerSettings          = 0x0427,
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
enum class KeyValue: uint16_t {
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

  BLTouchTestingStep1aYes   = 0x0001,
  BLTouchTestingStep1aNo    = 0x0002,
  BLTouchTestingStep1bSlow  = 0x0003,
  BLTouchTestingStep1bQuick = 0x0004,
  BLTouchTestingStep1bNo    = 0x0005,
  BLTouchTestingStep2Yes    = 0x0006,
  BLTouchTestingStep2No     = 0x0007,

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

  Abort                   = 0xFFFD,
  Save                    = 0xFFFE,
  Back                    = 0xFFFF
};


}