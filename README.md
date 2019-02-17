# ADVi3++ a Custom Marlin Firmware for Wanhao i3 Plus Printers

## Introduction

**ADVi3++** is a fork of the [Marlin Firmware](http://marlinfw.org/) for the Wanhao Duplicator i3 Plus (and its clones: Monoprice Maker Select Plus, Cocoon Create). It includes both Marlin itself and customizations of the LCD screens.

## Become a Sponsor of this Project

![sther](https://user-images.githubusercontent.com/981049/52916591-da1ee900-32e1-11e9-9112-5bb292d7cb79.png)

Working on **ADVi3++** takes a lot of time and effort. I make my best to release good quality software, useful and precise documents. I track bugs reported by users and replies to help requests. I invest also time and money to implement new hardware features.

If you like what I am doing, please consider supporting me as a supporter [on **sther**](https://www.sther.co/andrivet) so I can continue and develop new releases and new documents. Supporting this project is a guaranty for you that it will continue and become better and better.

## Objectives

The objectives with this custom version (when compared with Wanhao stock firmwares) are the following:

* Based on the latest version of Marlin available (currently: 1.1.9 + post 1.1.9 fixes)
* As few modifications of the Marlin code as possible. The goal is to be able to port **ADVi3++** to new versions of Marlin with only a few effort.
* Fully open source code. Unfortunately, even if it is required by the GPL, Wanhao has not released the source code of their latest Marlin-based firmwares.
* Object-oriented code with as few hard-coded values as possible. For example, instead of constructing a frame by hand, use instances of classes that know how to construct themselves.
* Documented code, logging and error handling (without using exceptions as they are not supported).
* Full colors and nice LCD screens including temperature graphs.
* Icons and pictures recreated from scratch.
* Support of hardware such as the BLTouch sensor for automatic bed leveling.
* Etc.

## ADVi3++ User Manual

A complete and up to date User Manual is published on [GitHub Pages](https://andrivet.github.io/ADVi3pp-User-Manual/). This manual contains:

* a list of features
* How to Flash the firmware
* LCD Touch Screen Manual
* BLTouch Sensor installation instructions
* Tuning instructions
* Default Configuration used by ADVi3++
* Resources 

# Sister projects

**ADVi3++** is split into two sister projects:

* ADVi3pp-Mainboard: this project, a fork of Marlin firmware
* [ADVi3pp-LCD](https://github.com/andrivet/ADVi3pp-LCD): the LCD resources

I have created a repository with stock firmwares:

* [Duplicator-i3-Plus-firmware-collection](https://github.com/andrivet/Duplicator-i3-Plus-firmware-collection)

# Wanhao Duplicator i3 Plus

I have posted an article on my web site: [Wanhao Duplicator i3 Plus 3D Printer - Everything I know about the Wanhao Duplicator i3 Plus 3D printer and its clones](http://sebastien.andrivet.com/en/posts/). It contains several technical information about the printer internals.

# Other projects

There are some other projects (I am aware of) on a similar topic:

- [i3plus+](https://github.com/Silverquark/i3PlusPlus) from Leo Lüker (Silverquark): a modified version of the Marlin 1.1.2 firmware for the WANHAO Duplicator i3 Plus 3d printer. **Important**: No more actively developed (that's why I created ADVi3++).

- [i3extra](https://github.com/nepeee/i3Extra) from Nemcsik Péter: a modified version of the Marlin 1.1.0-RC8 firmware and updated LCD resources. **Important**: this project is no more maintained.

- [Wanhao-I3-Plus](https://github.com/jamesarm97/Wanhao-I3-Plus) by James Armstrong: Sources of version 2.18 of Mainboard firmware and LCD resources.

- [Wanhao-I3-Plus](https://github.com/garychen99/Duplicator-i3-plus) by WANHAO 3D PRINTER: Official sources of version 2.1.803 of Mainboard firmware and LCD resources.

# Thanks

Thanks to [Dylan xKoldx](https://github.com/xKoldx/ADVi3pp-Marlin) and [weidercs](https://github.com/weidercs/ADVi3pp-Marlin) for their forks supporting BLTouch. Thanks to Matt Cowell [SureshotM6](https://github.com/SureshotM6/ADVi3pp-Marlin) for the fork supporting M600 and head parking.

Thanks you to Leo Lüker (Silverquark) for his [i3plus+ project](https://github.com/Silverquark/i3PlusPlus). I take many ideas from his code. But the project is no more active and I do not like the screens (personal taste) so I have created the **ADVi3++** fork. It is technically a fork of Marlin, not of i3plus+.

Thanks to [MarlinTeam](http://marlinfw.org/) for their great work.

# Copyright

**ADVi3++**

* Copyright &copy; 2017-2019 Sebastien Andrivet [GitHub ADVi3pp-Marlin project](https://github.com/andrivet/advi3pp-Marlin])
* Copyright &copy; 2016-2019 [MarlinFirmware](https://github.com/MarlinFirmware/Marlin)
* Based on Sprinter and grbl.
* Copyright &copy; 2011 Camiel Gubbels / Erik van der Zalm

**ADVi3++ User Manual**

* Copyright &copy; 2018-2019 Sebastien Andrivet

# License

**ADVi3++**

![](https://www.gnu.org/graphics/gplv3-127x51.png)

> This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

> This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

> You should have received a copy of the GNU General Public License along with this program. If not, see <http://www.gnu.org/licenses/>.

**ADVi3++ User Manual**

<a rel="license" href="http://creativecommons.org/licenses/by-sa/4.0/"><img alt="Creative Commons License" style="border-width:0" src="https://i.creativecommons.org/l/by-sa/4.0/88x31.png" /></a><br />This manual is licensed under a <a rel="license" href="http://creativecommons.org/licenses/by-sa/4.0/">Creative Commons Attribution-ShareAlike 4.0 International License</a>.


