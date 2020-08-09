# ADVi3++, an advanced and better firmware for Wanhao i3 Plus Printers

## Introduction

**ADVi3++** is a fork of the [Marlin Firmware](http://marlinfw.org/) for the Wanhao Duplicator i3 Plus (and its clones: Monoprice Maker Select Plus, Cocoon Create, ...). It includes both Marlin itself and screens for the LCD Panel.

## Become a Sponsor of this Project

[![Patreon](https://c5.patreon.com/external/logo/become_a_patron_button.png)](https://www.patreon.com/bePatron?u=6504486)

Working on **ADVi3++** takes a lot of time and effort. I make my best to release good quality software, useful and precise documents. I track bugs reported by users and replies to help requests. I invest also time and money to implement new hardware features.

If you like what I am doing, please consider supporting me as a supporter on [**Patreon**](https://www.patreon.com/bePatron?u=6504486) so I can continue and develop new releases and new documents. Supporting this project is a guaranty for you that it will continue and become better and better.

## Objectives

The objectives with this custom version (when compared with Wanhao stock firmwares) are the following:

* Based on the latest version of Marlin available (currently: 1.1.9, I am working on a Marlin 2 version)
* As few modifications of the Marlin code as possible. The goal is to be able to port **ADVi3++** to new versions of Marlin with only a few effort.
* Open source firmware (GPLv3 like Marlin)
* Object-oriented code with as few hard-coded values as possible. For example, instead of constructing a frame by hand, use instances of classes that know how to construct themselves.
* Documented code, logging and error handling (without using exceptions as they are not supported).
* Full color and nice LCD screens.
* Support of hardware such as the BLTouch sensor for automatic bed leveling.

## ADVi3++ User Manual

A complete and up to date User Manual is published on [ADVi3++ Community Web Site](hhttps://community.advi3pp.com/c/user-manual). This manual contains:

* [A list of features and of compatible printers](https://community.advi3pp.com/t/introduction/)
* [How to Flash the firmware](https://community.advi3pp.com/t/how-to-flash/)
* [LCD Touch Screen Guide](https://community.advi3pp.com/t/lcd-touch-screen-guide/)
* [Tuning instructions](https://community.advi3pp.com/t/tuning/)
* [Default Settings used by ADVi3++](https://community.advi3pp.com/t/default-settings/)
* [Starting and Ending G-Code](https://community.advi3pp.com/t/starting-and-ending-g-code/)
* [Resources](https://community.advi3pp.com/t/resources/)

[Sponsors of the **ADVI3++** project](https://www.patreon.com/andrivet) have also access to advanced topics:

* [BLTouch Sensor installation instructions](https://community.advi3pp.com/t/add-a-bltouch-sensor/)
* [Build ADVi3++ from Sources](https://community.advi3pp.com/t/build-advi3-from-sources/)
* [Alternative ways to Flash](https://community.advi3pp.com/t/alternative-ways-to-flash/)

## Support

You can get support from those resources:

* The [User Manual](https://community.advi3pp.com/c/user-manual). You may be especially interested by the [Troubleshooting part](https://community.advi3pp.com/c/user-manual/troubleshooting).
* The [ADVi3++ Community - Get Help](https://community.advi3pp.com/c/get-help). It is a forum, based on [Discourse](https://www.discourse.org). In particular, it includes a [Search page](https://community.advi3pp.com/search?expanded=true).
I monitor this forum and try to reply to all requests, from sponsors and non-sponsors of the ADVi3++ project.

# Wanhao Duplicator i3 Plus

I have posted an article on my web site: [Wanhao Duplicator i3 Plus 3D Printer - Everything I know about the Wanhao Duplicator i3 Plus 3D printer and its clones](http://sebastien.andrivet.com/en/posts/). It contains several technical information about the printer internals.

# Other projects

There are some other projects (I am aware of) on a similar topic:

* [i3plus+](https://github.com/Silverquark/i3PlusPlus) from Leo Lüker (Silverquark): a modified version of the Marlin 1.1.2 firmware for the WANHAO Duplicator i3 Plus 3d printer. **Important**: No more actively developed (that's why I created ADVi3++).

* [i3extra](https://github.com/nepeee/i3Extra) from Nemcsik Péter: a modified version of the Marlin 1.1.0-RC8 firmware and updated LCD resources. **Important**: this project is no more maintained.

* [Wanhao-I3-Plus](https://github.com/jamesarm97/Wanhao-I3-Plus) by James Armstrong: Sources of version 2.18 of Mainboard firmware and LCD resources.

* [Wanhao-I3-Plus](https://github.com/garychen99/Duplicator-i3-plus) by WANHAO 3D PRINTER: Official sources of version 2.1.803 of Mainboard firmware and LCD resources.

# Thanks

Thanks to [Dylan xKoldx](https://github.com/xKoldx/ADVi3pp-Marlin) and [weidercs](https://github.com/weidercs/ADVi3pp-Marlin) for their forks supporting BLTouch. Thanks to Matt Cowell [SureshotM6](https://github.com/SureshotM6/ADVi3pp-Marlin) for the fork supporting M600 and head parking.

Thanks to Leo Lüker (Silverquark) for his [i3plus+ project](https://github.com/Silverquark/i3PlusPlus). I take many ideas from his code. But the project is no more active and I do not like the screens (personal taste) so I have created the **ADVi3++** fork. It is technically a fork of Marlin, not of i3plus+.

Thanks to [MarlinTeam](http://marlinfw.org/) for their great work.

# Copyright

## ADVi3++

* Copyright &copy; 2017-2020 Sebastien Andrivet [GitHub ADVi3pp project](https://github.com/andrivet/advi3pp])
* Copyright &copy; 2016-2020 [MarlinFirmware](https://github.com/MarlinFirmware/Marlin)
* Based on Sprinter and grbl.
* Copyright &copy; 2011 Camiel Gubbels / Erik van der Zalm

## ADVi3++ LCD Panels

* Copyright &copy; 2017-2020 Sebastien Andrivet [GitHub ADVi3pp project](https://github.com/andrivet/advi3pp])

## ADVi3++ User Manual

* Copyright &copy; 2018-2020 Sebastien Andrivet

# License

## ADVi3++ Firmware

![](https://www.gnu.org/graphics/gplv3-127x51.png)

> This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
>
> This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
>
> You should have received a copy of the GNU General Public License along with this program. If not, see <http://www.gnu.org/licenses/>.

The sources of the ADVi3++ firmware are based (fork) on [Marlin firmware](http://marlinfw.org/) and located under `Marlin`.

## ADVi3++ LCD Panels

![](https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png)

ADVi3++ LCD Panels are licensed under a <a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License</a>.

**IMPORTANT**: If you want to use these panels (or modified versions of them) for **commercial purposes**, [please contact me](https://community.advi3pp.com/u/andrivet).

The sources of the LCD panels are:
 
* The [Sketch](https://www.sketch.com) file provided in the repository of ADVi3++ under `LCD-Panel/Masters`
* The files provided under `LCD-Panel/DGUS-root` for DGUS SDK 5.1

Those are the "true" sources, in the sense that everything is generated from them. 
However, for convenience, I have also included under `LCD-Panel/DGUS-root` bitmaps (.BMP) generated from the Sketch file.

## ADVi3++ User Manual

![](https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png)

This manual is licensed under a <a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License</a>.
