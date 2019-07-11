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

#ifndef ADVI3PP_MARLIN_ADVI3PP_VERSIONS_H
#define ADVI3PP_MARLIN_ADVI3PP_VERSIONS_H

// Ugly date computation (C legacy...)

#define YEAR__ ((((__DATE__ [7] - '0') * 10 + (__DATE__ [8] - '0')) * 10 \
+ (__DATE__ [9] - '0')) * 10 + (__DATE__ [10] - '0'))

#define MONTH__ (__DATE__[2] == 'n' ? (__DATE__[1] == 'a' ? 1 : 6) \
: __DATE__[2] == 'b' ? 2 \
: __DATE__[2] == 'r' ? (__DATE__[0] == 'M' ? 3 : 4) \
: __DATE__[2] == 'y' ? 5 \
: __DATE__[2] == 'l' ? 7 \
: __DATE__[2] == 'g' ? 8 \
: __DATE__[2] == 'p' ? 9 \
: __DATE__[2] == 't' ? 10 \
: __DATE__[2] == 'v' ? 11 : 12)

#define DAY__ ((__DATE__[4] == ' ' ? 0 : __DATE__[4] - '0') * 10 + (__DATE__[5] - '0'))

#define HOUR__ (((__TIME__[0] - '0') * 10) + (__TIME__[1] - '0'))
#define MIN__  (((__TIME__[3] - '0') * 10) + (__TIME__[4] - '0'))
#define SEC__  (((__TIME__[6] - '0') * 10) + (__TIME__[7] - '0'))


namespace advi3pp
{
    const uint16_t advi3_pp_version = 0x403;
    const uint16_t advi3_pp_oldest_lcd_compatible_version = 0x403;
    const uint16_t advi3_pp_newest_lcd_compatible_version = 0x403;
    const uint16_t settings_version = 0x0003; // Version of ADVi3++ settings stored in EEPROM
}


#endif //ADVI3PP_MARLIN_ADVI3PP_VERSIONS_H
