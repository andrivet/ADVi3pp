/**
 * ADVstd - Minimalist entities from the Standard Library
 *
 * Copyright (C) 2020 Sebastien Andrivet [https://github.com/andrivet/]
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

#include "ADVstd.h"
#include "tuple.h"
#include "iterator.h"

namespace adv {

// From https://en.cppreference.com/w/cpp/container/array
// C++14 because C++17 or higher currently not supported by avr-gcc
// Since Exception are not supported, at and similar functions are not checking their arguments

template<typename T, size_t N>
struct array_const_iterator_
{
    using iterator_category = random_access_iterator_tag;
    using value_type        = T;
    using difference_type   = ptrdiff_t;
    using pointer           = const T*;
    using reference         = const T&;

    array_const_iterator_() noexcept: ptr_{}, index_{} {}
    explicit array_const_iterator_(pointer ptr, size_t offset = 0) noexcept: ptr_{ptr}, index_{offset} {}

    reference operator*() const noexcept                                    { return *operator->(); }
    pointer operator->() const noexcept                                     { return ptr_ + index_; }

    array_const_iterator_& operator++() noexcept                            { ++index_; return *this; }
    array_const_iterator_ operator++(int) noexcept                          { array_const_iterator_ tmp = *this; ++*this; return tmp; }

    array_const_iterator_& operator--() noexcept                            { --index_; return *this; }
    array_const_iterator_ operator--(int) noexcept                          { array_const_iterator_ tmp = *this; --*this; return tmp; }

    array_const_iterator_& operator+=(const ptrdiff_t offset) noexcept      { index_ += offset; return *this; }
    array_const_iterator_ operator+(const ptrdiff_t offset) const noexcept  { array_const_iterator_ tmp = *this; return tmp += offset; }
    array_const_iterator_& operator-=(const ptrdiff_t offset) noexcept      { return *this += -offset; }
    array_const_iterator_ operator-(const ptrdiff_t offset) const noexcept  { array_const_iterator_ tmp = *this; return tmp -= offset; }
    ptrdiff_t operator-(const array_const_iterator_& right) const noexcept  { return static_cast<ptrdiff_t>(index_ - right.index_); }

    reference operator[](const ptrdiff_t offset) const noexcept             { return *(*this + offset); }

    bool operator==(const array_const_iterator_& right) const noexcept      { return index_ == right.index_; }
    bool operator!=(const array_const_iterator_& right) const noexcept      { return !(*this == right); }
    bool operator<(const array_const_iterator_& right) const noexcept       { return index_ < right.index_; }
    bool operator>(const array_const_iterator_& right) const noexcept       { return right < *this; }
    bool operator<=(const array_const_iterator_& right) const noexcept      { return !(right < *this); }
    bool operator>=(const array_const_iterator_& right) const noexcept      { return !(*this < right); }

private:
    pointer ptr_;
    size_t index_;
};


template<typename T, size_t N>
struct array_iterator_
{
    using iterator_category = random_access_iterator_tag;
    using value_type        = T;
    using difference_type   = ptrdiff_t;
    using pointer           = T*;
    using reference         = T&;

    array_iterator_() noexcept: ptr_{}, index_{} {}
    explicit array_iterator_(pointer ptr, size_t offset = 0) noexcept: ptr_{ptr}, index_{offset} {}

    reference operator*() const noexcept                                    { return *operator->(); }
    pointer operator->() const noexcept                                     { return ptr_ + index_; }

    array_iterator_& operator++() noexcept                                  { ++index_; return *this; }
    array_iterator_ operator++(int) noexcept                                { array_iterator_ tmp = *this; ++*this; return tmp; }

    array_iterator_& operator--() noexcept                                  { --index_; return *this; }
    array_iterator_ operator--(int) noexcept                                { array_iterator_ tmp = *this; --*this; return tmp; }

    array_iterator_& operator+=(const ptrdiff_t offset) noexcept            { index_ += offset; return *this; }
    array_iterator_ operator+(const ptrdiff_t offset) const noexcept        { array_iterator_ tmp = *this; return tmp += offset; }
    array_iterator_& operator-=(const ptrdiff_t offset) noexcept            { return *this += -offset; }
    array_iterator_ operator-(const ptrdiff_t offset) const noexcept        { array_iterator_ tmp = *this; return tmp -= offset; }
    ptrdiff_t operator-(const array_iterator_& right) const noexcept        { return static_cast<ptrdiff_t>(index_ - right.index_); }

    reference operator[](const ptrdiff_t offset) const noexcept             { return *(*this + offset); }

    bool operator==(const array_iterator_& right) const noexcept            { return index_ == right.index_; }
    bool operator!=(const array_iterator_& right) const noexcept            { return !(*this == right); }
    bool operator<(const array_iterator_& right) const noexcept             { return index_ < right.index_; }
    bool operator>(const array_iterator_& right) const noexcept             { return right < *this; }
    bool operator<=(const array_iterator_& right) const noexcept            { return !(right < *this); }
    bool operator>=(const array_iterator_& right) const noexcept            { return !(*this < right); }

private:
    pointer ptr_;
    size_t index_;
};



template<typename T, size_t N>
struct array
{
    using value_T                   = T;
    using size_type                 = size_t;
    using difference_type           = ptrdiff_t;
    using pointer                   = T*;
    using const_pointer             = const T*;
    using reference                 = T&;
    using const_reference           = const T&;
    using iterator                  = array_iterator_<T, N>;
    using const_iterator            = array_const_iterator_<T, N>;
    using reverse_iterator          = adv::reverse_iterator<iterator>;
    using const_reverse_iterator    = adv::reverse_iterator<const_iterator>;

    // Element access
    reference at(size_type pos)                                 { return elements_[pos]; } // WARNING: No check
    constexpr const_reference at(size_type pos) const           { return elements_[pos]; } // WARNING: No check
    reference operator[](size_type pos)                         { return elements_[pos]; }
    constexpr const_reference operator[](size_type pos) const   { return elements_[pos]; }
    reference front()                                           { return elements_[0]; }
    constexpr const_reference front() const                     { return elements_[0]; }
    reference back()                                            { return elements_[N - 1]; }
    constexpr const_reference back() const                      { return elements_[N - 1]; }
    T* data() noexcept                                          { return elements_; }
    constexpr const T* data() const noexcept                    { return elements_; }

    // Iterators
    iterator begin() noexcept                                   { return iterator(elements_, 0); }
    constexpr const_iterator begin() const noexcept             { return const_iterator(elements_, 0); }
    constexpr const_iterator cbegin() const noexcept            { return begin(); }
    iterator end() noexcept                                     { return iterator(elements_, N); }
    constexpr const_iterator end() const noexcept               { return const_iterator(elements_, N); }
    constexpr const_iterator cend() const noexcept              { return end(); }
    reverse_iterator rbegin() noexcept                          { return reverse_iterator(end()); }
    constexpr const_reverse_iterator rbegin() const noexcept    { return const_reverse_iterator(end()); }
    constexpr const_reverse_iterator crbegin() const noexcept   { return rbegin(); }
    reverse_iterator rend() noexcept                            { return reverse_iterator(begin()); }
    constexpr const_reverse_iterator rend() const noexcept      { return const_reverse_iterator(begin()); }
    constexpr const_reverse_iterator crend() const noexcept     { return rend(); }

    // Capacity
    constexpr bool empty() const noexcept                       { return false; }
    constexpr size_type size() const noexcept                   { return N; }
    constexpr size_type max_size() const noexcept               { return N; }

    // Operations
    void fill(const T& value);
    void swap(array& other) noexcept(noexcept(swap(adv::declval<T&>(), adv::declval<T&>())));

    T elements_[N ? N : 1];
};


// Implementations

template<typename T, size_t N>
void array<T, N>::fill(const T& value)
{
    for(size_t i = 0; i < N; ++i)
        elements_[i] = value;
}

template<typename T, size_t N>
void array<T, N>::swap(array& other) noexcept(noexcept(swap(adv::declval<T&>(), adv::declval<T&>())))
{
    for(size_t i = 0; i < N; ++i)
        adv::swap(elements_[i], other.elements_[i]);
}

// Non-member functions

template<typename T, size_t N>
constexpr bool operator==(const adv::array<T, N>& lhs, const adv::array<T, N>& rhs);

template<typename T, size_t N>
constexpr bool operator!=(const adv::array<T, N>& lhs, const adv::array<T, N>& rhs);

template<typename T, size_t N>
constexpr bool operator<(const adv::array<T, N>& lhs, const adv::array<T, N>& rhs);

template<typename T, size_t N>
constexpr bool operator<=(const adv::array<T, N>& lhs, const adv::array<T, N>& rhs);

template<typename T, size_t N>
constexpr bool operator>(const adv::array<T, N>& lhs, const adv::array<T, N>& rhs);

template<typename T, size_t N>
constexpr bool operator>=(const adv::array<T, N>& lhs, const adv::array<T, N>& rhs);

// Helper classes
template<typename T, size_t N>
struct tuple_size<adv::array<T, N>>: adv::integral_constant<size_t, N> {};

template<size_t I, typename T, size_t N>
struct tuple_element<I, adv::array<T, N>>
{
    using type = T;
};

template<size_t I, typename T, size_t N>
constexpr T& get(adv::array<T, N>& a) noexcept          { return a.elements_[I]; }

template<size_t I, typename T, size_t N>
constexpr const T& get(const array<T, N>& a) noexcept   { return a.elements_[I]; }

template<size_t I, typename T, size_t N>
constexpr T&& get(array<T, N>&& a) noexcept             { return adv::move(a.elements_[I]); }

template<size_t I, typename T, size_t N>
constexpr const T&& get(const array<T, N>&& a) noexcept { return adv::move(a.elements_[I]); }

template<typename T, size_t N>
void swap(adv::array<T, N>& lhs, adv::array<T, N>& rhs);

}
