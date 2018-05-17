/**
 * ADVstd - Minimalist entities from the Standard Library
 *
 * Copyright (C) 2018 Sebastien Andrivet [https://github.com/andrivet/]
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

#ifndef ADVSTD_H
#define ADVSTD_H

namespace andrivet {

using size_t = decltype(sizeof(int));
using nullptr_t = decltype(nullptr);

template<typename T> struct remove_reference { using type = T; };
template<typename T> struct remove_reference<T&>  { using type = T; };
template<typename T> struct remove_reference<T&&> { using type = T; };

template<typename T, T v>
struct integral_constant
{
    static constexpr const T value = v;
    typedef T value_type;
    typedef integral_constant type;
    inline constexpr explicit operator value_type() const noexcept {return value;}
    inline constexpr value_type operator ()() const noexcept {return value;}
};

using true_type  = integral_constant<bool, true>;
using false_type = integral_constant<bool, false>;

template<typename T> struct is_lvalue_reference     : false_type {};
template<typename T> struct is_lvalue_reference<T&> : true_type {};

template<typename T>
struct is_void { static const bool value = false;};
template<> struct is_void<void> { static const bool value = true; };

template<typename T>
inline typename remove_reference<T>::type&& move(T&& t) noexcept
{
    typedef typename remove_reference<T>::type U;
    return static_cast<U&&>(t);
}

template<typename T>
inline T&& forward(typename remove_reference<T>::type& t) noexcept
{
    return static_cast<T&&>(t);
}

template<typename T>
inline T&& forward(typename remove_reference<T>::type&& t) noexcept
{
    static_assert(!is_lvalue_reference<T>::value, "can not forward an rvalue as an lvalue");
    return static_cast<T&&>(t);
}

template<typename T>
void swap(T& a, T& b)
{
    T c{move(a)}; a = move(b); b = move(c);
}

}

#endif
