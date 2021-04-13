/**
 * ADVi3++ Firmware For Wanhao Duplicator i3 Plus (based on Marlin)
 *
 * Copyright (C) 2017-2021 Sebastien Andrivet [https://github.com/andrivet/]
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

#include "../../lib/array.h"

template <typename T>
void endian_swap(T &val, typename std::enable_if<std::is_arithmetic<T>::value, std::nullptr_t>::type = nullptr)
{
    auto ptr = reinterpret_cast<std::uint8_t*>(&val);
    std::array<std::uint8_t, sizeof(T)> raw_src, raw_dst;

    for(std::size_t i = 0; i < sizeof(T); ++i)
        raw_src[i] = ptr[i];

    std::reverse_copy(raw_src.begin(), raw_src.end(), raw_dst.begin());

    for(std::size_t i = 0; i < sizeof(T); ++i)
        ptr[i] = raw_dst[i];
}
