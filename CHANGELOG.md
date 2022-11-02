# ADVi3++ Changelog

## 5.5.0 (2022-11-02)

* **#318** - [Thermal runaway error is not always displayed on the LCD screen](https://github.com/andrivet/ADVi3pp/issues/318)
* **#317** - [G-Code M114 response breaks Octolapse](https://github.com/andrivet/ADVi3pp/issues/317)
* **#313** - [Cancel automatic leveling](https://github.com/andrivet/ADVi3pp/issues/313)
* **#312** - [Detect and prevent bounces](https://github.com/andrivet/ADVi3pp/issues/312)
* **#311** - [Restore Z-offset in Sensor Settings](https://github.com/andrivet/ADVi3pp/issues/311)
* **#310** - [Add steps in BLTouch Tuning for cases when the BLTouch is blinking red](https://github.com/andrivet/ADVi3pp/issues/310)
* **#308** - [Update description of Print Settings pane](https://github.com/andrivet/ADVi3pp/issues/308)
* **#307** - [Versions over 5.0.3 having issues accessing SD card files](https://github.com/andrivet/ADVi3pp/issues/307)
* **#306** - [Fast probing with BLTouch](https://github.com/andrivet/ADVi3pp/issues/306)
* **#293** - [BLTouch - warning on transposed black and white wires](https://github.com/andrivet/ADVi3pp/issues/293)
* **#296** - [Manual jog controls disturb homing function, triggering "Homing failed" hardware error](https://github.com/andrivet/ADVi3pp/issues/296)
* **#294** - [If print menu is accessed while printer is moving, it shows as if it's printing](https://github.com/andrivet/ADVi3pp/issues/294)
* **#267** - [Live Flow Tuning](https://github.com/andrivet/ADVi3pp/issues/267)
* **#238** - [M851 should accept all offset arguments, not just Z offset](https://github.com/andrivet/ADVi3pp/issues/238)
* **#153** - [Graph as default view or GCode to show the graph view](https://github.com/andrivet/ADVi3pp/issues/153)

## 5.4.2 (2022-08-03)

* **#298** - [The computation of the Extruder Tuning is wrong](https://github.com/andrivet/ADVi3pp/issues/298)
* **#297** - [The firmware does not take into account arguments for the M300 code](https://github.com/andrivet/ADVi3pp/issues/297)
* **#293** - [BLTouch - warning on transposed black and white wires](https://github.com/andrivet/ADVi3pp/issues/293)

## 5.4.1 (2022-07-17)

* **#292** - [LCD and beeper settings are not saved or taken into account](https://github.com/andrivet/ADVi3pp/issues/292)

## 5.4.0 (2022-07-15)

* **#291** - [When pressing on a move button, it continues for a while after the button is released](https://github.com/andrivet/ADVi3pp/issues/291)
* **#290** - [Avoid homing when not necessary](https://github.com/andrivet/ADVi3pp/issues/290)
* **#289** - [Travel movements during manual bed levelling significantly slower than earlier versions](https://github.com/andrivet/ADVi3pp/issues/289)
* **#288** - [Inversion of black and white BLTouch wires not detected](https://github.com/andrivet/ADVi3pp/issues/288)
* **#287** - [Warn if manual bed leveling is attempted while extruder is hot](https://github.com/andrivet/ADVi3pp/issues/287)
* **#285** - [Show what triggered a hardware error](https://github.com/andrivet/ADVi3pp/issues/285)
* **#284** - [LCD stuck dim when hardware error detected](https://github.com/andrivet/ADVi3pp/issues/284)
* **#279** - [Enhance the beeping feature](https://github.com/andrivet/ADVi3pp/issues/279)
* **#277** - [Raise extruder during Tuning / Extruder](https://github.com/andrivet/ADVi3pp/issues/277)
* **#274** - [Enhanced motion during filament load](https://github.com/andrivet/ADVi3pp/issues/274)
* **#246** - [Skew compensation implementation](https://github.com/andrivet/ADVi3pp/issues/246)
* **#177** - [Cancel e-steps tuning](https://github.com/andrivet/ADVi3pp/issues/177)

## 5.3.0 (2022-06-04)

* **#214** - Troubleshooting vibrations and mechanical issues

## 5.2.0 (2022-05-01)

* **#278** - Hide gcode files where the file name starts with a period
* **#276** - Files with the letter "å" in the file name do not appear correctly
* **#21** - Implement file management
* Change slightly the font

## 5.1.1 (2022-03-29)

* **#283** - Automatic bed leveling is reporting 0 for all points

## 5.1.0 (2022-03-25)

* Align ADVi3++ with upstream latest release of Marlin (2.0.9.3)
* **#271** - Fix pin #6 displayed as an output in the I/O screen
* **#268** - Fix Z height not saved and 0.1mm less than expected
* **#263** - Use G-Code M256 (set brightness) instead of M250 (set contrast)
* **#79** - Support for filament runout sensors

## 5.0.3 (2022-02-27)

* **#273** - Loading feedrate is way too fast

## 5.0.2 (2021-11-14)

* **#266** - Junction Deviation bug and display one more digit
* **#260** - Fix LCD dimming

## 5.0.1 (2021-09-18)

* **#259** - X direction printing is limited to 195mm
* **#258** - The filename is not displayed when it is short (8.3 filename)
* **#257** - Holding the + E button reboots the printer
* **#256** - The tools under Controls/Move are not smooth
* **#255** - The LCD panel does not dim after a while

## 5.0.0 (2021-09-10)

* Major release based on Marlin 2.0.7
* Restructuration of the different parts (more modular) and rewrite of some parts (such as the communication layer between the mainboard and the LCD panel)
* Redesign of several panels
* New tools such as:
  - Setup panel
  - BLTouch testing panels
  - Leveling panel
  - X Twist compensation
  - Direct entry of values with an on-screen keyboard

## 4.0.6 (2020-02-23)

* **#236** - platformio.ini file contained a configuration to upload the firmware using a Dragon AVR programmer
* **#235** - Sometimes, message displayed on Wait screens was wrong
* **#234** - Some actions should be disabled during a print
* **#232** - Fan speed in Preset was not saved in EEPROM

## 4.0.5 (2020-01-20)

* **#230** - During pause (both normal and advanced), wait a moment just after purging the nozzle
* **#229** - Publish a firmware to clear the EEPROM (to use when it is corrupted)
* **#228** - Sometimes after flashing, the LCD screen displays distorted images
* **#227** - Mark II: 4.0.4 Auto bed leveling stuck at homing
* **#226** - Printer freeze when using BLTouch Y offset >= 8
* **#223** - Disabling beeps does not work with some LCD displays
* **#219** - After stopping a print the X, Y, and Z axis will all move when Z jog is pressed
* **#218** - Fix (again) "(L,R,F,B) out of bounds" error
* **#217** - Y axis jogs itself on first G0/G1 after power on

## 4.0.4 (2019-12-30)

* **#224** - Fix ghost touch with BLTouch 3.1. See note bellow
* **#218** - Fix "(L,R,F,B) out of bounds" error
* **#215** - Sometimes after flashing, the LCD Panel displays a version mismatch warning

**Note**: 

* **#224** is not really a bug but more a vibration problem with the printer itself or the mounting support.
This release tries however to minimize the occurrences of such ghosts touches. As a consequence, the G29 sequence is slower.
I will change that in version 4.1.0 to make the sensor parameters configurable. 

## 4.0.3 (2019-07-11)

* **#215** - Sometimes after flashing, the LCD Panel displays a version mismatch warning with a wrong LCD version and the continue button does not work

## 4.0.2 (2019-06-30)

* **#209** - Prints were not centered (~ 2.5 mm on Y axis)
* **#210** - Values when you press buttons in Print Settings were wrong
* **#212** - BLTouch supports positions were not perfectly correct (Baseggio/Indianagio Front)

## 4.0.1 (2019-06-15)

### Bug fixes

* **#208** - Homing Z moves extruder to X10, Y10 (i.e. disable Safe Homing for non BLTouch builds)
* **#209** - Prints are not centered

## 4.0.0 (2019-06-08)

### New features

* **#73** - Enable Linear Advance (LIN_ADVANCE). By default `K` is `0` but you can change it on the LCD Panel
* **#75** - IO page with the live state of several digital and analog lines
* **#81** - Multiple PID values recorded and auto PID tune. When a target temperature is set, choose the closest set of PID values.
* **#101** - PID Tuning of Heated Bed
* **#108** - Live Z adjustment (Babystepping)
* **#125** - Ability to enter Touch Sensor (X, Y) offset position on the LCD Panel.
* **#187** - Aldi UK printer (HE180021) support
* **#195** - Support of the new BLTouch Smart version 3
* **#195** - Add the positions of some sensor supports (Giorgio Baseggio, TechningTech, Mark II)
* **#203** - G20, G21, M149 support
* **#204** - M0 and M1 support
* **#204** - S-Curve Acceleration support
* **#204** - G26 Mesh Validation Pattern tool support. Not yet in the LCD Panel UI
* **#204** - Support of BLTouch and BLTouch Smart 3 with Aldi UK printer (HE180021)

### Enhancements

* **#27** - Add a progress bar during printing
* **#29** - During SD print, displays the remaining time in percent
* **#39** - Be able to cancel a PID tuning
* **#165** - Allow bigger values (>800) for motor steps.
* **#170** - Disable drive when stopping print
* **#179** - Add free memory (SRAM) in statistics
* **#185** - Show page number on SD Card screen
* **#192** - Clear the message on the display when G4 is finished
* **#195** - Align ADVi3++ with the latest 1.1.9 patches from Marlin
* New Z Height tuning - The Z-end stop is no more used

### Bug fixes

* **#156** - No more filament load/unload artifacts
* **#159** - Switches like on iOS (left off, right, on)
* **#163** - Title of last print stuck on LCD screen
* **#176** - Race condition between TP_Flag reading and action sent by the LCD Panel
* **#195** - Prints were not centered in the bed
* **#195** - Cooldown turns also off the fan
* **#199** - Octoprint connection speed issues
* **#204** - Under some circumstances, bed leveling compensation was disabled
* **#204** - Sometimes when hitting Pause, the head goes in the corner as expected but then suddenly goes over the piece being printed

### Removed features

* **#162**: XYZ Motors "calibration". It was wrong. It gives the false impression that this part needs some calibration when in fact, these parameters (number of steps per millimeter) are directly derived from the mechanical parts of the printer (the stepper motor, ...)

## 3.0.2 (2018-06-10)

### New features

* **#138** - Simplify the build of a BLTouch release with Arduino IDE

### Bug fixes

* **#126** - ABL stops at random heights with same g-code
* **#132** - Bug: cannot set sensor/nozzle height lower than -2 mm bug
* **#133** - The default steps (X, Y, Z and probably E) are wrong
* **#136** - When a setting is wrong (such as jerk equals to 0), reset it to the default

## 3.0.1 (2018-05-31)

### Bug fixes

* **#128**: Jerk values were wrong for Z and E axis
* **#129**: It was not possible to confirm Factory Reset

## 3.0.0 (2018-05-25)

### New features

* BLTouch support (Classic and Smart). Clones of BLTouch are not supported (and I recommend to avoid them)
* [#8], [#18]: M600 support and head parking
* Pause, Resume and Stop when printing from USB
* Z-height in status (near the fan speed, in green)
* Display messages on Home screen
* Thermal Runaway enhancements: Now displays why the printer was stopped. You can also choose to disable thermal protection. At your own risk. I recommend to let it on.
* USB baudrate: you can now choose the baud rate. It is 115200 by default (like in the previous versions)
* Buzzer control: You can now disable or enable the buzzer from the LCD panel. The buzzer is now off by default when you press the screen, but does but for some actions (load and unload for example).
* Screen Dimming control: You can now disable or enable the dimming of the LCD panel and choose its intensity.
* **#82**: Change slightly colors for a better contrast
* **#102**: Declare thermal protection capability
* **#103**: In settings stored in EEPROM are not compatible, warn the user and reset settings to factory

### Bug fixes

* **#15**: Will not x-home if you cancel a print and then restart. Very annoying but not easy to fix
* [#84]: SD Card (Print menu): Directories are displayed like if they were files
* **#37**, #97, #118: Filament loading/unloading stops when requested
* 121 Manual moves (X, Y and Z) are now smother 


## 2.1.0 (2018-01-12)

### New features

* Align with Marlin `1.1.8`
* Upgrade `atmelavr` to `1.8.1`
* Upgrade PlatformIO to `3.5.0`
* [#34](https://github.com/andrivet/ADVi3pp/issues/34): Change maximum temperatures to **280°C** and **150°C**. **Be careful if you use high temperatures, it is at your own risk**.
* [#40](https://github.com/andrivet/ADVi3pp/issues/40): Change default acceleration speeds to `1000` and default jerk to `8`

### Bug fixes

* Increment values (XYZ Calibaration cube, ...) by 0.1 (was 0.5)


## 2.0.1 (2017-11-26)

### Bug fixes

* [#35](https://github.com/andrivet/ADVi3pp/issues/35): Bed and Hotend colors and icons were inverted.
* [#33](https://github.com/andrivet/ADVi3pp/issues/33): In Print Settings, it was not possible to go higher than 100% (print speed)


## 2.0.0 (2017-11-19)

### New features

* Access on the LCD display to more printer parameters like Feedrate, Acceleration and Jerk settings
* Calibration of the X, Y, and Z motors and of the extruder
* Temperature graphs with thicker lines
* 5 points manual bed leveling
* Display of messages (M117 code) from external software such as [OctoPrint](https://github.com/foosel/OctoPrint) ([Detailed Progress](https://github.com/dattas/OctoPrint-DetailedProgress) plugin for example)
* New default settings (taken from Wanhao old sources)
* Up to date [User Manual](https://andrivet.github.io/ADVi3pp-User-Manual/)

### Bug fixes

* [#15](https://github.com/andrivet/ADVi3pp/issues/15): Will not x-home if you cancel a print and then restart
* [#23](https://github.com/andrivet/ADVi3pp/issues/23): Adjust print settings while printing
* [#24](https://github.com/andrivet/ADVi3pp/issues/24): Compilation issues with Arduino IDE
* [#25](https://github.com/andrivet/ADVi3pp/issues/25): Change e-steps manually does not work
* [#28](https://github.com/andrivet/ADVi3pp/issues/28): In Print Settings, pressing on plus or minus button changes the wrong value
* [#31](https://github.com/andrivet/ADVi3pp/issues/31): Stop, Pause and Resume are not working when printing from USB. (*Note: functionality removed in this version*)
* [#32](https://github.com/andrivet/ADVi3pp/issues/32): Back button is not working in PID tuning page 


## 1.0.1 (2017-10-25)

### Bug fixes

* EEPROM settings storage. The CRC was not correctly computed and thus the reading of settings failed.


## 1.0.0 (2017-10-17)

### New features

* Boot animation with colors and the Marlin logo
* Colored screens
* About screen with versions of the different components (Mainboard and LCD **ADVi3++** versions, LCD firmware version, Marlin version) and integrity check
* SD card and printing menu
* Temperature graphs accessible from several screens by pressing the temperature numbers
* Load and Unload filament
* Preheat bed and hotend with 3 sets of values
* Move head, extrude, unextrude
* Statistics
* PID automatic tuning
* Bed leveling (4 points)
* Factory Reset with warning

