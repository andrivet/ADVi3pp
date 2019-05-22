/**
 * ADVstd - A simple (and partial) implementation of unique_ptr.
 * This implementation does not support custom deleters.
 * This implementattion does not include arreays.
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

#ifndef ADVCALLBACK_ADVUNIQUE_PTR_H
#define ADVCALLBACK_ADVUNIQUE_PTR_H

#include "ADVstd.h"

namespace adv
{

template<typename T>
class unique_ptr {
public:
    using pointer = T*;
    using element_type = T;

    constexpr unique_ptr() noexcept : ptr_{nullptr} {}
    constexpr unique_ptr(nullptr_t) noexcept : ptr_{nullptr} {}
    explicit unique_ptr(pointer p) noexcept: ptr_{p} {}
    unique_ptr(unique_ptr&& p) noexcept : ptr_{p.release()} {}
    template<typename U> unique_ptr(unique_ptr<U>&& p) noexcept : ptr_{p.release()} {}

    ~unique_ptr() { reset(); }

    unique_ptr& operator=(unique_ptr&& p) noexcept { reset(p.release()); return *this; }
    template<typename U> unique_ptr& operator=(unique_ptr<U>&& p) noexcept { reset(p.release()); return *this; }
    unique_ptr& operator=(nullptr_t) noexcept { reset(); return *this; }

    T& operator*() const noexcept { return *get(); }
    T& operator[](size_t i) const noexcept { return get()[i]; }
    T* operator->() const noexcept { return get(); }
    explicit operator bool() const noexcept { return get() != nullptr; }
    T* get() const noexcept { return ptr_; }
    T* release() noexcept { T* p = ptr_; ptr_ = nullptr; return p; }
    void reset(T* p = nullptr) noexcept { if(p != ptr_) { delete ptr_; ptr_ = p; } }
    void reset(nullptr_t) noexcept { delete ptr_; ptr_ = nullptr; }
    void swap(unique_ptr& p) noexcept { adv::swap(ptr_, p.ptr_); }

    // Disabled
    unique_ptr(const unique_ptr&) = delete;
    template<typename U> unique_ptr(const unique_ptr<U>&) = delete;

    unique_ptr& operator=(const unique_ptr&) = delete;
    template<typename U> unique_ptr& operator=(const unique_ptr<U>&) = delete;

private:
    T* ptr_;
};

template<typename T, typename... A>
unique_ptr<T> make_unique(A&&... args) { return unique_ptr<T>(new T(forward<A>(args)...)); }

template<typename T1, typename T2>
bool operator==(const unique_ptr<T1>& x, const unique_ptr<T2>& y) { return x.get() == y.get(); }

template<typename T>
bool operator==(const unique_ptr<T>& x, nullptr_t) noexcept { return x.get() == nullptr; }

template<typename T>
bool operator==(nullptr_t, const unique_ptr<T>& x) noexcept { return x.get() == nullptr; }

template<typename T1, typename T2>
bool operator!=(const unique_ptr<T1>& x, const unique_ptr<T2>& y) { return x.get() != y.get(); }

template<typename T>
bool operator!=(const unique_ptr<T>& x, nullptr_t) noexcept { return x.get() != nullptr; }

template<typename T>
bool operator!=(nullptr_t, const unique_ptr<T>& x) noexcept { return x.get() != nullptr; }

template<typename T1, typename T2>
bool operator<(const unique_ptr<T1>& x, const unique_ptr<T2>& y) noexcept { return x.get() < y.get(); }

template<typename T1, typename T2>
bool operator<(const unique_ptr<T1>& x, nullptr_t) noexcept { return x.get() < nullptr; }

template<typename T1, typename T2>
bool operator<(nullptr_t, const unique_ptr<T2>& y) noexcept { return nullptr < y.get(); }

template<typename T1, typename T2>
bool operator>(const unique_ptr<T1>& x, const unique_ptr<T2>& y) noexcept { return x.get() > y.get(); }

template<typename T1, typename T2>
bool operator>(const unique_ptr<T1>& x, nullptr_t) noexcept { return x.get() > nullptr; }

template<typename T1, typename T2>
bool operator>(nullptr_t, const unique_ptr<T2>& y) noexcept { return nullptr > y.get(); }

template<typename T1, typename T2>
bool operator<=(const unique_ptr<T1>& x, const unique_ptr<T2>& y) noexcept { return x.get() <= y.get(); }

template<typename T1, typename T2>
bool operator<=(const unique_ptr<T1>& x, nullptr_t) noexcept { return x.get() <= nullptr; }

template<typename T1, typename T2>
bool operator<=(nullptr_t, const unique_ptr<T2>& y) noexcept { return nullptr <= y.get(); }

template<typename T1, typename T2>
bool operator>=(const unique_ptr<T1>& x, const unique_ptr<T2>& y) noexcept { return x.get() >= y.get(); }

template<typename T1, typename T2>
bool operator>=(const unique_ptr<T1>& x, nullptr_t) noexcept { return x.get() >= nullptr; }

template<typename T1, typename T2>
bool operator>=(nullptr_t, const unique_ptr<T2>& y) noexcept { return nullptr >= y.get(); }

}

#endif
