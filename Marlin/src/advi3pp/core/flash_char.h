/**
 * ADVi3++ Firmware For Wanhao Duplicator i3 Plus (based on Marlin 2)
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

#ifndef ADV_UNIT_TESTS

#include <WString.h>

namespace ADVi3pp {

using FlashChar = __FlashStringHelper;

inline PGM_P from_flash(const FlashChar* str) { return reinterpret_cast<PGM_P>(str); }
inline const FlashChar* to_flash(PGM_P str) { return reinterpret_cast<const FlashChar*>(str); }

}

#else

namespace ADVi3pp {

struct FlashChar { const char* str; };

inline const char* from_flash(const FlashChar* str) { return reinterpret_cast<const char*>(str); }
inline const FlashChar* to_flash(const char* str) { return reinterpret_cast<const FlashChar*>(str); }

}

#endif
