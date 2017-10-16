# ADVi3++ a Custom Marlin Firmware for Wanhao i3 Plus Printers

![boot](https://user-images.githubusercontent.com/981049/31636347-6151d4b2-b2ca-11e7-80d4-777faaa8f9a3.gif)

## Introduction

**ADVi3++** is a fork of the Marlin Firmware for the Wanhao Duplicator i3 Plus (and its clones: Monoprice Maker Select Plus, Coccon Create). It includes both Marlin itself and customizations of the LCD screens.

## Objectives

The objectives with this custom version (when compared with Wanhao stock firmwares) are the following:

* Based on the latest version of Marlin available (currently: 1.1.6)
* As few modifications of the Marlin code as possible. The goal is to be able to port **ADVi3++** to new versions of Marlin with only a few effort.
* Fully open source code. Unfortunately, even if it is required by the GPL, Wanhao has not released the source code of their latest Marlin-based firmwares.
* Object-oriented code with as few hard-coded values as possible. For example, instead of constructing a frame by hand, use instances of classes that know how to construct themselves.
* Documented code, logging and error handling (without using exceptions as they are not supported).
* Full colors and nice LCD screens including temperature graphs.
* Icons and pictures recreated from scratch.
* About screen with the various versions (LCD, Mainboard, ...) and integrity check.
* Etc.

## How to flash

There are two parts to flash:

* The mainboard firmware
* The LCD resources

### Part 1 - Flash the mainboard firmware

There are several ways to flash the mainboard firmware. The first step is to download the firmware from the **Releases** page on [GitHub](https://github.com/andrivet/ADVi3pp-Marlin/releases):

[ADVi3pp-Mainboard-1.0.0.hex](https://github.com/andrivet/ADVi3pp-Marlin/releases/download/v1.0.0.0/ADVi3pp-Mainboard-1.0.0.hex)

#### Option 1 - Flash using Cura

* Start **Cura**
* Under **Machines** select your printer
* Select **Install custom firmware**
* Select the downloaded file `ADVi3pp-1.0.0.hex` and click on **Open**

#### Option 2 - Flash using OctoPrint

You need `advdude` and the development build of the **Firmware Updater** plugin.

To install `advdude` on a Raspberry Pi:

* Connect to the Raspberry (for example through SSH) and enter the command:

```
sudo apt update; sudo apt install avrdude
```

To install the development build of the **Firmware Updater** plugin

* Open a navigator and connect to **OctoPrint**
* **Login** and click on the wrench icon in the toolbar
* Select **Plugin Manager** &#8594; **Get More...**
* Under **from URL** enter `https://github.com/OctoPrint/OctoPrint-FirmwareUpdater/archive/devel.zip` and click on **Install**
* Restart **OctoPrint**

To flash the firmware:

![firmwareupdater](https://user-images.githubusercontent.com/981049/31636354-65b72dfe-b2ca-11e7-8c7d-7279477906d0.png)

* When **OctoPrint** in rebooted and the UI reloaded, click on the wrench icon in the toolbar
* Under **Plugins**, choose **Firmware Updater**
* Click on the wrench icon, and after **Path to advdude**, enter `/usr/bin/avrdude` and click **Save**
* Be sure your USB port appears after **Serial Port**
* After **... from file**, click on **Browse** and select the firmware you have downloaded such as `ADVi3pp-1.0.0.hex`
* Click on **Flash from File**
* The flashing process may take around 30 seconds
* When it is finished, a message appears saying "Flashing successful". Click on **Save**
* Reconnect the printer

### Part 2 - Flash the LCD resources

**Note**: *The LCD resources are hosted on their own GitHub project: [ADVi3pp-LCD](https://github.com/andrivet/ADVi3pp-LCD).*

#### Step 1 - Prepare a microSD card

**IMPORTANT**: *You have to use a microSD card with a maximum capacity of **8GiB**. If you use a microSD card with a greater capacity, the results are **unreliable** (sometimes it flashes, sometimes not). This is a limitation of the LCD display itself.*

You have two possibilities to flash:

##### Option 1 - Manual copy

* Download the LCD resources: [ADVi3pp-LCD-1.0.0.zip](https://github.com/andrivet/ADVi3pp-Marlin/releases/download/v1.0.0.0/ADVi3pp-LCD-1.0.0.zip)
* Unzip the file somewhere
* Copy manually the files under `SD` to the root of a microSD card. The microSD card **has** to be formatted with the following parameters: FAT32, 4096 bytes per cluster (i.e. 8 sectors). To format under Linux (and macOS with the `dosfstools` Homebrew package):

```
mkfs.fat -F 32 -n SD -s 8 -v /dev/disk2
```

Of course, replace `/dev/disk2` with the right value.

##### Option 2 - SD image

* Use the SD card dump I have prepared. Download: [ADVi3pp-LCD-1.0.0.img.zip](https://github.com/andrivet/ADVi3pp-Marlin/releases/download/v1.0.0.0/ADVi3pp-LCD-1.0.0.img.zip)
* Unzip the `.img.zip` file and use either `dd` (Linux, macOS) or [Etcher](https://etcher.io) (Windows, Linux, macOS). For example with `dd`:

```
unzip ADVi3pp-LCD-1.0.0.img.zip
sudo dd if=ADVi3pp-LCD-1.0.0.img.zip of=/dev/disk2 bs=64K
```

Of course, replace `/dev/disk2` with the right value.

If you prefer graphical applications, [Etcher](https://etcher.io) is a great multi-platform tool.

#### Step 2 - Install the new version

- Disconnect the printer from power
- Remove the two screws located on the front and loose the two M3 screws on the top

![front-panel-screws](https://user-images.githubusercontent.com/981049/31637200-e3ecc438-b2cd-11e7-888f-aad32bc96676.jpg)

- Remove the front panel carefully (don't break the flat cable)
- If you are lucky, you can insert the microSD card on the left of the panel (this is the case on the Monoprice clone)
- Otherwise, remove the four M3 screws and remove the cover
- Insert the microSD card in the slot

![lcd-board-microsd](https://user-images.githubusercontent.com/981049/31637212-f5511148-b2cd-11e7-958a-9f496205c498.jpg)

- Turn on the printer (either by connecting it to power or by connecting the USB slot to the computer)
- The screen turns blue and then every image will appear one by one
- After around 2 or 3 minutes, the screen turns black
- Turn off the printer and remove the microSD card
- Re-assemle the front panel, do not forget the two M3 screws on the top
- Turn the printer on. You know have the new version of the LCD images

# Features

![screens](https://user-images.githubusercontent.com/981049/31637836-ebc14dc0-b2d0-11e7-8d02-d14152116ba9.gif)

This custom firmware has the following features:

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

# Sister project

**ADVi3++** is split into two sister projects:

* ADVi3pp-Mainboard: this project, a fork of Marlin firmware
* [ADVi3pp-LCD](https://github.com/andrivet/ADVi3pp-LCD): the LCD resources

# Wanhao Duplicator i3 Plus

## LCD display

![LCD board](https://raw.github.com/andrivet/Wanhao-i3-Plus/master/assets/LCD-board.jpg)

The LCD panel is manufactured by AMP Display Inc. and uses DGUS (DWIN Graphic User System) developed by DWIN Technologies. The reference of the LCD panel is `DMT48270M043_05W` and you can find its [datasheet](http://www.dwin.com.cn/uploads/English%20Documents/Mini%20DGUS/DMT48270M043_05W_DATASHEET.pdf) under "Resources" below.

It is build around the proprietary K600+ kernel and simplifies the development of user interfaces: the different screens are defined by a set of files (images, fonts, etc) stored in the flash memory of the panel and they can be customized using a graphical editor (part of [DGUS SDK](http://www.dwin.com.cn/uploads/English%20Documents/DGUS_SDK.rar)).

The LDC panel has thus two distinct software components:

- A [firmware](https://en.wikipedia.org/wiki/Firmware), made by DWIN Technologies.
- A set of resources, made by the manufacturer of the 3D printer (Wanhao, Monoprice, Coccon) or yourself. This component is often called "LCD firmware" even if it is wrong.

### LCD firmware

**WARNING**: *Do not make the confusion between the actual LCD firmware (made by DWIN Technologies) and the LCD resources for the customization of the display (next paragraph).*

There is a version of LCD firmware available on the DWIN web site ([firmware version 7.1 - 20161024](http://www.dwin.com.cn/uploads/产品数据手册/DGUS%20upgrade%20program%2020161024%20.rar)). 

**I have not yet tested it. I will.**.

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

- [DMT48270M043_05W Datasheet](http://www.dwin.com.cn/uploads/English%20Documents/Mini%20DGUS/DMT48270M043_05W_DATASHEET.pdf)
- [DWIN DGUS Display Development Guide version 4.3](http://dwin.com.cn/uploads/产品数据手册/DWIN%20DGUS%20DEV%20GUIDE_V43_2015.pdf)
- [DGUS SDK](http://www.dwin.com.cn/uploads/English%20Documents/DGUS_SDK.rar) - Windows only
- [DGUS firmware update](http://www.dwin.com.cn/uploads/产品数据手册/DGUS%20upgrade%20program%2020161024%20.rar). _WARNING: I have not yet tested it._
- [DWIN Documentations and softwares](http://dwin.com.cn/english/products/bt-72876584214435.html)

# Other projects

There are some other projects (I am aware of) on a similar topic:

- [i3plus+](https://github.com/Silverquark/i3PlusPlus) from Leo Lüker (Silverquark): a modified version of the Marlin 1.1.2 firmware for the WANHAO Duplicator i3 Plus 3d printer.

- [i3extra](https://github.com/nepeee/i3Extra) from Nemcsik Péter: a modified version of the Marlin 1.1.0-RC8 firmware and updated LCD resources. **Important**: this project is no more maintained.

- [Wanhao-I3-Plus](https://github.com/jamesarm97/Wanhao-I3-Plus) by James Armstrong: Sources of version 2.18 of Mainboard firware and LCD resources.

- [Wanhao-I3-Plus](https://github.com/garychen99/Duplicator-i3-plus) by WANHAO 3D PRINTER: Official sources of version 2.1.803 of Mainboard firmware and LCD resources.

- [Original Wanhao i3 Plus Firmware version 3.0](http://www.wanhao3dprinter.com/FAQ/ShowArticle.asp?ArticleID=79) As far as I know, Wanhao has not yet released the source of version 3.0 (even if it is required under GPL). I have ask them when they plan to release the sources. I am waiting...

# Thanks

Thank you to Leo Lüker (Silverquark) for his [i3plus+](https://github.com/Silverquark/i3PlusPlus) project. I take many ideas from his code. But the project is not very active (still in Beta) and I do not like the screen (personal taste) so I have created the **ADVi3++** fork (technically, it is a fork of **Marlin**, not of **i3plus+**).

Thank to [Marlin](http://marlinfw.org) for their great work.

# Copyright

* Copyright &copy; 2017 Sebastien Andrivet [https://github.com/andrivet/advi3pp-Marlin]
* Copyright &copy; 2016 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
* Based on Sprinter and grbl.
* Copyright &copy; 2011 Camiel Gubbels / Erik van der Zalm

# License

> This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

> This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

> You should have received a copy of the GNU General Public License along with this program. If not, see <http://www.gnu.org/licenses/>.

