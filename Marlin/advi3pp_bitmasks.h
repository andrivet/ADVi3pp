/**
 * Copyright (C) 2018 Sebastien Andrivet [https://github.com/andrivet/]
 * Copyright (C) 2015 Just Software Solutions Ltd
 *
 * Distributed under the Boost Software License, Version 1.0.
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or
 * organization obtaining a copy of the software and accompanying
 * documentation covered by this license (the "Software") to use,
 * reproduce, display, distribute, execute, and transmit the
 * Software, and to prepare derivative works of the Software, and
 * to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire
 * statement, including the above license grant, this restriction
 * and the following disclaimer, must be included in all copies
 * of the Software, in whole or in part, and all derivative works
 * of the Software, unless such copies or derivative works are
 * solely in the form of machine-executable object code generated
 * by a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
 * KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE
 * LIABLE FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */
#ifndef MARLIN_ADVI3PP_BITMASKS_H
#define MARLIN_ADVI3PP_BITMASKS_H

namespace advi3pp {

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

#endif //MARLIN_ADVI3PP_BITMASKS_H
