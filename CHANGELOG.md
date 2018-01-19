# ADVi3++ Changelog

## 2.1.0 (2018-01-12)

### New features

* Align with Marlin `1.1.8`
* Upgrade `atmelavr` to `1.8.1`
* Upgrade PlatformIO to `3.5.0`
* [#34](https://github.com/andrivet/ADVi3pp-Marlin/issues/34): Change maximum temperatures to **280°C** and **150°C**. **Be careful if you use high temperatures, it is at your own risk**.
* [#40](https://github.com/andrivet/ADVi3pp-Marlin/issues/40): Change default acceleration speeds to `1000` and default jerk to `8`

### Bug fixes

* Increment values (XYZ Calibaration cube, ...) by 0.1 (was 0.5)


## 2.0.1 (2017-11-26)

### Bug fixes

* [#35](https://github.com/andrivet/ADVi3pp-Marlin/issues/35): Bed and Hotend colors and icons were inverted.
* [#33](https://github.com/andrivet/ADVi3pp-Marlin/issues/33): In Print Settings, it was not possible to go higher than 100% (print speed)


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

* [#15](https://github.com/andrivet/ADVi3pp-Marlin/issues/15): Will not x-home if you cancel a print and then restart
* [#23](https://github.com/andrivet/ADVi3pp-Marlin/issues/23): Adjust print settings while printing
* [#24](https://github.com/andrivet/ADVi3pp-Marlin/issues/24): Compilation issues with Arduino IDE
* [#25](https://github.com/andrivet/ADVi3pp-Marlin/issues/25): Change e-steps manually does not work
* [#28](https://github.com/andrivet/ADVi3pp-Marlin/issues/28): In Print Settings, pressing on plus or minus button changes the wrong value
* [#31](https://github.com/andrivet/ADVi3pp-Marlin/issues/31): Stop, Pause and Resume are not working when printing from USB. (*Note: functionality removed in this version*)
* [#32](https://github.com/andrivet/ADVi3pp-Marlin/issues/32): Back button is not working in PID tuning page 


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

