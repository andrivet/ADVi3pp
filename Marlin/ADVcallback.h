/**
 * ADVcallback - Universal Callbacks without Standard Library
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

#ifndef ADV_CALLBACKS_H
#define ADV_CALLBACKS_H

#include "ADVunique_ptr.h"

namespace andrivet
{

template<typename R, typename...A>
struct Callable
{
    virtual Callable* clone() const = 0;
    virtual R operator()(A...args) const = 0;
    virtual ~Callable() = default;
};

template<typename F, typename R, typename...A>
struct CallableFunction: public Callable<R, A...>
{
    using Super = Callable<R, A...>;
    using Self = CallableFunction<F, R, A...>;

    explicit CallableFunction(F f): function_{f} {}
    Self* clone() const override { return new CallableFunction(function_); }

    R operator()(A...args) const override { if(is_void<R>::value) function_(args...); else return function_(args...); }

private:
    F function_;
};

template<typename O, typename R, typename...A>
struct CallableMethod: public Callable<R, A...>
{
    using M = R(O::*)(A...);
    using Super = Callable<R, A...>;
    using Self = CallableMethod<O, R, A...>;

    CallableMethod(O& o, M m): object_{o}, method_{m} {}
    Self* clone() const override { return new Self(object_, method_); }

    R operator()(A...args) const override { if(is_void<R>::value) (object_.*method_)(args...); else return (object_.*method_)(args...); }

private:
    O& object_;
    M method_;
};

template<typename O, typename R, typename...A>
struct CallableConstMethod: public Callable<R, A...>
{
    using M = R(O::*)(A...);
    using Super = Callable<R, A...>;
    using Self = CallableMethod<O, R, A...>;

    CallableConstMethod(const O& o, M m): object_{o}, method_{m} {}
    Self* clone() const override { return new Self(object_, method_); }

    R operator()(A...args) const override { if(is_void<R>::value) (object_.*method_)(args...); else return (object_.*method_)(args...); }

private:
    const O& object_;
    M method_;
};

template<typename>
struct Callback;

template <typename R, typename... A>
struct Callback<R(*)(A...)>
{
    using F = R(*)(A...);
    using CB = Callback<F>;

    // Empty callback
    Callback() noexcept = default;
    explicit Callback(nullptr_t) noexcept {}

    // From a function
    explicit Callback(F f): callable_{new CallableFunction<F, R, A...>(f)} {}

    // From a member function
    template <typename O, typename M>
    Callback(O& o, const M& m): callable_{new CallableMethod<O, R, A...>(o, m)} {}

    // From a const member function
    template <typename O, typename M>
    Callback(const O& o, const M& m): callable_{new CallableConstMethod<O, R, A...>(o, m)} {}

    // Captured lambda specialization
    template<typename L>
    explicit Callback(const L& l): callable_{new CallableFunction<decltype(l), R, A...>(l)} {}

    // From another Callback
    Callback(const CB& cb): callable_{cb.callable_ != nullptr ? cb.callable_->clone() : nullptr} {}

    // Assignment
    Callback& operator=(const CB& c) { callable_.reset(c.callable_ != nullptr ? c.callable_->clone() : nullptr); return *this; }
    Callback& operator=(nullptr_t) { callable_ = nullptr; return *this; }

    // Call
    R operator()(A... args)
    { if(is_void<R>::value) { if(callable_) (*callable_)(args...); } else { return callable_ ? (*callable_)(args...) : R(); } }

    // Boolean
    explicit operator bool() const noexcept { return callable_ != nullptr; }

private:
    unique_ptr<Callable<R, A...>> callable_;
};

}

#endif // ADV_CALLBACKS_H
