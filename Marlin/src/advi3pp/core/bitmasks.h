/**
 * ADVi3++ Firmware For Wanhao Duplicator i3 Plus (based on Marlin)
 *
 * Copyright (C) 2017-2020 Sebastien Andrivet [https://github.com/andrivet/]
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

#include <stdint.h>

namespace ADVi3pp {

template<bool B, class T = void>
struct enable_if {};

template<class T>
struct enable_if<true, T> { typedef T type; };


template<typename E>
struct enable_bitmask_operators
{
    static const bool enable = false;
};

template<typename E>
typename enable_if<enable_bitmask_operators<E>::enable, E>::type
operator |(E lhs, E rhs)
{
    return static_cast<E>(static_cast<unsigned>(lhs) |  static_cast<unsigned>(rhs));
}

template<typename E>
typename enable_if<enable_bitmask_operators<E>::enable, E>::type
operator &(E lhs, E rhs)
{
    return static_cast<E>(static_cast<unsigned>(lhs) & static_cast<unsigned>(rhs));
}

template<typename E>
typename enable_if<enable_bitmask_operators<E>::enable, E>::type
operator ^(E lhs, E rhs)
{
    return static_cast<E>(static_cast<unsigned>(lhs) ^ static_cast<unsigned>(rhs));
}

template<typename E>
typename enable_if<enable_bitmask_operators<E>::enable, E>::type
operator ~(E rhs)
{
    return static_cast<E>(~static_cast<unsigned>(rhs));
}

template<typename E>
typename enable_if<enable_bitmask_operators<E>::enable, E&>::type
operator |=(E &lhs, E rhs)
{
    lhs = static_cast<E>(static_cast<unsigned>(lhs) | static_cast<unsigned>(rhs));
    return lhs;
}

template<typename E>
typename enable_if<enable_bitmask_operators<E>::enable, E&>::type
operator &=(E &lhs, E rhs)
{
    lhs = static_cast<E>(static_cast<unsigned>(lhs) & static_cast<unsigned>(rhs));

    return lhs;
}

template<typename E>
typename enable_if<enable_bitmask_operators<E>::enable, E&>::type
operator ^=(E &lhs, E rhs)
{
    lhs = static_cast<E>(static_cast<unsigned>(lhs) ^ static_cast<unsigned>(rhs));
    return lhs;
}

template<typename E>
typename enable_if<enable_bitmask_operators<E>::enable, bool>::type
test_all_bits(E lhs, E rhs)
{
    return (lhs & rhs) == rhs;
}

template<typename E>
typename enable_if<enable_bitmask_operators<E>::enable, bool>::type
test_one_bit(E lhs, E rhs)
{
    return static_cast<uint16_t>(lhs & rhs) != 0;
}

template<typename E>
typename enable_if<enable_bitmask_operators<E>::enable, E&>::type
flip_bits(E& lhs, E rhs)
{
    lhs ^= rhs;
    return lhs;
}

template<typename E>
typename enable_if<enable_bitmask_operators<E>::enable, E&>::type
set_bits(E& lhs, E rhs)
{
    lhs |= rhs;
    return lhs;
}

template<typename E>
typename enable_if<enable_bitmask_operators<E>::enable, E&>::type
clear_bits(E& lhs, E rhs)
{
    lhs &= ~rhs;
    return lhs;
}


#define ENABLE_BITMASK_OPERATOR(E) \
template<> struct enable_bitmask_operators<E> { static const bool enable = true; }

}