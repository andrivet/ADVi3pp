# ADVi3++ Changelog

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

