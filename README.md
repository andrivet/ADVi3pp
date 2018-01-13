# ADVi3++ a Custom Marlin Firmware for Wanhao i3 Plus Printers

![boot](https://user-images.githubusercontent.com/981049/31636347-6151d4b2-b2ca-11e7-80d4-777faaa8f9a3.gif)

## Introduction

**ADVi3++** is a fork of the Marlin Firmware for the Wanhao Duplicator i3 Plus (and its clones: Monoprice Maker Select Plus, Cocoon Create). It includes both Marlin itself and customizations of the LCD screens.

## Objectives

The objectives with this custom version (when compared with Wanhao stock firmwares) are the following:

* Based on the latest version of Marlin available (currently: 1.1.8)
* As few modifications of the Marlin code as possible. The goal is to be able to port **ADVi3++** to new versions of Marlin with only a few effort.
* Fully open source code. Unfortunately, even if it is required by the GPL, Wanhao has not released the source code of their latest Marlin-based firmwares.
* Object-oriented code with as few hard-coded values as possible. For example, instead of constructing a frame by hand, use instances of classes that know how to construct themselves.
* Documented code, logging and error handling (without using exceptions as they are not supported).
* Full colors and nice LCD screens including temperature graphs.
* Icons and pictures recreated from scratch.
* About screen with the various versions (LCD, Mainboard, ...) and integrity check.
* Etc.

## ADVi3++ User Manual

A complete and up to date [User Manual](https://andrivet.github.io/ADVi3pp-User-Manual/) is published on [GitHub Pages](https://andrivet.github.io/ADVi3pp-User-Manual/). This manual contains:

* a list of features
* How to Flash the firmware
* LCD Touch Screen Manual
* Default Configuration used by ADVi3++
* Resources 

This manual is also available as a [standalone PDF file](https://github.com/andrivet/ADVi3pp-User-Manual/blob/master/ADVi3%2B%2B%20User%20Manual.pdf).

# Sister project

**ADVi3++** is split into two sister projects:

* ADVi3pp-Mainboard: this project, a fork of Marlin firmware
* [ADVi3pp-LCD](https://github.com/andrivet/ADVi3pp-LCD): the LCD resources

I have also published an article ["Wanhao Duplicator i3 Plus 3D Printer - Everything I know about the Wanhao Duplicator i3 Plus 3D printer and its clones"](http://sebastien.andrivet.com/en/posts/wanhao-duplicator-i3-plus-3d-printer/).

# Wanhao Duplicator i3 Plus

## LCD display

![lcd-board](https://user-images.githubusercontent.com/981049/31936387-bc21d81e-b8b1-11e7-9635-783c5854a6a4.jpg)

The LCD panel is manufactured by AMP Display Inc. and uses DGUS (DWIN Graphic User System) developed by DWIN Technologies. The reference of the LCD panel is `DMT48270M043_05W` and you can find its [datasheet](https://github.com/andrivet/ADVi3pp-LCD/blob/master/Resources/DMT48270M043_05W_DATASHEET.pdf) under "Resources" below.

It is build around the proprietary K600+ kernel and simplifies the development of user interfaces: the different screens are defined by a set of files (images, fonts, etc) stored in the flash memory of the panel and they can be customized using a graphical editor (part of [DGUS SDK](https://github.com/andrivet/ADVi3pp-LCD/blob/master/Resources/DGUS_Setup_5.1.zip)).

The LDC panel has thus two distinct software components:

- A [firmware](https://en.wikipedia.org/wiki/Firmware), made by DWIN Technologies.
- A set of resources, made by the manufacturer of the 3D printer (Wanhao, Monoprice, Cocoon) or yourself. This component is often called "LCD firmware" even if it is wrong.

### LCD firmware

**WARNING**: *Do not make the confusion between the actual LCD firmware (made by DWIN Technologies) and the LCD resources for the customization of the display (next paragraph).*

### LCD resources

The images have the following format:

- BMP (Windows 3.x Bitmap)
- 480 x 272
- 24 bits per pixel (3 x 8 bits, no alpha)
- [Bottom-up bitmap](https://msdn.microsoft.com/en-us/library/windows/desktop/dd407212(v=vs.85).aspx)

The last point is very important. The LCD (i.e. DGUS) is not able to handle other formats like top-down bitmaps. To convert images to the righr format, you can use [ImageMagick](https://www.imagemagick.org/) `convert`:

```
convert 180.bmp -resize 480x272 BMP3:180.bmp
```

To convert all images:

```
for f in *.bmp; do convert "$f" -resize 480x272 BMP3:"$f"; done
```
# Resources

**Note:** *In the past, it was possible to download these resources directory from the [dwin.com.cn](http://dwin.com.cn) website. It is no more the case (you get plenty of errors) so I copy them into this repository.*

- [DMT48270M043_05W Datasheet](https://github.com/andrivet/ADVi3pp-LCD/blob/master/Resources/DMT48270M043_05W_DATASHEET.pdf)
- [DWIN DGUS Display Development Guide version 4.0 - 2014](https://github.com/andrivet/ADVi3pp-LCD/blob/master/Resources/DWIN%20DGUS%20DEV%20GUIDE_V40_2014.pdf)
- [DGUS SDK User Guide 5.1](https://github.com/andrivet/ADVi3pp-LCD/blob/master/Resources/DGUSV5.10.pdf)
- [DGUS SDK 5.1 Setup](https://github.com/andrivet/ADVi3pp-LCD/blob/master/Resources/DGUS_Setup_5.1.zip) - Windows only

# Other projects

There are some other projects (I am aware of) on a similar topic:

- [i3plus+](https://github.com/Silverquark/i3PlusPlus) from Leo Lüker (Silverquark): a modified version of the Marlin 1.1.2 firmware for the WANHAO Duplicator i3 Plus 3d printer. **Important**: No more actively developed (that's why I created ADVi3++).

- [i3extra](https://github.com/nepeee/i3Extra) from Nemcsik Péter: a modified version of the Marlin 1.1.0-RC8 firmware and updated LCD resources. **Important**: this project is no more maintained.

- [Wanhao-I3-Plus](https://github.com/jamesarm97/Wanhao-I3-Plus) by James Armstrong: Sources of version 2.18 of Mainboard firmware and LCD resources.

- [Wanhao-I3-Plus](https://github.com/garychen99/Duplicator-i3-plus) by WANHAO 3D PRINTER: Official sources of version 2.1.803 of Mainboard firmware and LCD resources.

- [Original Wanhao i3 Plus Firmware version 3.0](http://www.wanhao3dprinter.com/FAQ/ShowArticle.asp?ArticleID=79) As far as I know, Wanhao has not yet released the source of version 3.0 (even if it is required under GPL). I have ask them when they plan to release the sources. I am waiting...

# Thanks

Thank you to Leo Lüker (Silverquark) for his [i3plus+](https://github.com/Silverquark/i3PlusPlus) project. I take many ideas from his code. But the project is not very active (still in Beta) and I do not like the screen (personal taste) so I have created the **ADVi3++** fork (technically, it is a fork of **Marlin**, not of **i3plus+**).

Thank to [Marlin](http://marlinfw.org) for their great work.

# Copyright

* Copyright &copy; 2017-2018 Sebastien Andrivet [https://github.com/andrivet/advi3pp-Marlin]
* Copyright &copy; 2016-2018 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
* Based on Sprinter and grbl.
* Copyright &copy; 2011 Camiel Gubbels / Erik van der Zalm

# License

![](https://www.gnu.org/graphics/gplv3-127x51.png)

> This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

> This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

> You should have received a copy of the GNU General Public License along with this program. If not, see <http://www.gnu.org/licenses/>.

