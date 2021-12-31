/**
 * endian - Helpers to handle little and big endian
 *
 * Copyright (C) 2018 Sebastien Andrivet [https://github.com/andrivet/]
 * Copyright (C) 2017 Jonathan Boccara [https://www.fluentcpp.com/2017/05/19/crtp-helper/]
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

namespace adv {

inline uint16_t word_from_bytes(uint8_t high, uint8_t low)
{
    return static_cast<uint16_t>(high) << 8 | low;
}

template<typename T>
T endian_swap(T)
{
    static_assert(assert_false<T>::value, "Not implemented for this type");
}

template<>
inline uint16_t endian_swap<uint16_t>(uint16_t value)
{
    return ((value & 0xFF00) >> 8) | ((value & 0x00FF) << 8);
}

template<>
inline int16_t endian_swap<int16_t>(int16_t value)
{
    return ((value & 0xFF00) >> 8) | ((value & 0x00FF) << 8);
}

template<>
inline uint32_t endian_swap(uint32_t value)
{
    return
      ((value & 0x000000FF) << 24) |
      ((value & 0x0000FF00) <<  8) |
      ((value & 0x00FF0000) >>  8) |
      ((value & 0xFF000000) >> 24);
}

template<>
inline int32_t endian_swap(int32_t value)
{
    return
      ((value & 0x000000FF) << 24) |
      ((value & 0x0000FF00) <<  8) |
      ((value & 0x00FF0000) >>  8) |
      ((value & 0xFF000000) >> 24);
}

template<>
inline uint64_t endian_swap(uint64_t value)
{
    return
      ((value & 0x00000000000000FF) << 56) |
      ((value & 0x000000000000FF00) << 40) |
      ((value & 0x0000000000FF0000) << 24) |
      ((value & 0x00000000FF000000) <<  8) |
      ((value & 0x000000FF00000000) >>  8) |
      ((value & 0x0000FF0000000000) >> 24) |
      ((value & 0x00FF000000000000) >> 40) |
      ((value & 0xFF00000000000000) >> 56);
}

template<>
inline int64_t endian_swap(int64_t value)
{
    return
      ((value & 0x00000000000000FF) << 56) |
      ((value & 0x000000000000FF00) << 40) |
      ((value & 0x0000000000FF0000) << 24) |
      ((value & 0x00000000FF000000) <<  8) |
      ((value & 0x000000FF00000000) >>  8) |
      ((value & 0x0000FF0000000000) >> 24) |
      ((value & 0x00FF000000000000) >> 40) |
      ((value & 0xFF00000000000000) >> 56);
}

}
