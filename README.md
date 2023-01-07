# ADVi3++, an advanced and better firmware for Wanhao i3 Plus Printers

## Introduction

**ADVi3++** is a fork of the [Marlin Firmware](http://marlinfw.org/) for the Wanhao Duplicator i3 Plus (and its clones: Monoprice Maker Select Plus, Cocoon Create, ...). It includes both Marlin itself and screens for the LCD Panel.

## Become a Sponsor of this Project

[![Patreon](https://c5.patreon.com/external/logo/become_a_patron_button.png)](https://www.patreon.com/bePatron?u=6504486)

Working on **ADVi3++** takes a lot of time and effort. I make my best to release good quality software, useful and precise documents. I track bugs reported by users and replies to help requests. I invest also time and money to implement new hardware features.

If you like what I am doing, please consider supporting me as a supporter on [**Patreon**](https://www.patreon.com/bePatron?u=6504486) so I can continue and develop new releases and new documents. Supporting this project is a guaranty for you that it will continue and become better and better.

## Objectives

The objectives with this custom version (when compared with Wanhao stock firmwares) are the following:

* Based on the latest version of Marlin available (currently: 2.1.1)
* As few modifications of the Marlin code as possible. The goal is to be able to port **ADVi3++** to new versions of Marlin with only a few effort.
* Open source firmware (GPLv3 like Marlin).
* Object-oriented code with as few hard-coded values as possible. For example, instead of constructing a frame by hand, use instances of classes that know how to construct themselves.
* Use latest C++ features and technics including metaprogramming.
* Documented code, logging and error handling (without using exceptions as they are not supported).
* Full color and nice LCD screens (CC BY-NC-SA).
* Support of hardware such as the BLTouch sensor for automatic bed leveling, filament runout sensors, ...

## ADVi3++ User Manual

A complete and up-to-date user manual is published on [ADVi3++ User Manual](https://manual.advi3pp.com). This manual contains:

* A list of features and of compatible printers
* How to Flash the firmware
* LCD Touch Screen Guide
* BLTouch Sensor installation instructions
* Filament Runout Sensor installation instructions
* Building ADVi3++ from Sources
* Alternative ways to flash
* Several troubleshooting guides

**Note**: Starting with version 5.1, this User Manual is only accessible to [supporters of the project on Patreon](https://www.patreon.com/bePatron?u=6504486).

## Support

You can get support from those resources:

* The [User Manual](https://manual.advi3pp.com), especially the [Troubleshooting part](https://manual.advi3pp.com/en/troubleshooting).
* The [ADVi3++ Community](https://community.advi3pp.com/). It is a forum, based on [Discourse](https://www.discourse.org). 
I monitor this forum and try to reply to all requests.

# Wanhao Duplicator i3 Plus

I have posted an article on my web site: [Wanhao Duplicator i3 Plus 3D Printer - Everything I know about the Wanhao Duplicator i3 Plus 3D printer and its clones](http://sebastien.andrivet.com/en/posts/). It contains several technical information about the printer internals.

# Thanks

* Thanks to [Dylan xKoldx](https://github.com/xKoldx/ADVi3pp-Marlin) and [weidercs](https://github.com/weidercs/ADVi3pp-Marlin) for their forks supporting BLTouch. Thanks to Matt Cowell [SureshotM6](https://github.com/SureshotM6/ADVi3pp-Marlin) for the fork supporting M600 and head parking.
* Thanks to Leo Lüker (Silverquark) for his [i3plus+ project](https://github.com/Silverquark/i3PlusPlus). I take many ideas from his code. But the project is no more active and I do not like the screens (personal taste) so I have created the **ADVi3++** fork. It is technically a fork of Marlin, not of i3plus+.
* Thanks to [MarlinTeam](http://marlinfw.org/) for their great work.

# Copyright

## ADVi3++

* Copyright &copy; 2017-2023 Sebastien Andrivet [GitHub ADVi3pp project](https://github.com/andrivet/advi3pp])
* Copyright &copy; 2016-2023 [MarlinFirmware](https://github.com/MarlinFirmware/Marlin)
* Based on Sprinter and grbl. Copyright &copy; 2011 Camiel Gubbels / Erik van der Zalm

## ADVi3++ LCD Panels

* Copyright &copy; 2017-2023 Sebastien Andrivet [GitHub ADVi3pp project](https://github.com/andrivet/advi3pp])

## ADVi3++ User Manual

* Copyright &copy; 2018-2023 Sebastien Andrivet

# License

## ADVi3++ Firmware

![](https://www.gnu.org/graphics/gplv3-127x51.png)

> This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
>
> This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
>
> You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.

## ADVi3++ LCD Panels

![](https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png)

ADVi3++ LCD Panels are licensed under a <a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License</a>.

**IMPORTANT**: If you want to use these panels (or modified versions of them) for **commercial purposes**, [please contact me](https://community.advi3pp.com/u/andrivet).

## ADVi3++ User Manual

![](https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png)

The ADVi3++ User Manual is licensed under a <a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License</a>.
