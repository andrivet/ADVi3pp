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

#include "ADVstd.h"

namespace adv
{

namespace internal {
    template<typename I>
    void copy_data(const I& i, void* o)
    {
        auto p = reinterpret_cast<const char*>(&i);
        copy(p, p + sizeof(I), reinterpret_cast<char*>(o));
    }
}

template<typename R, typename...A>
struct Callable
{
    virtual void clone(void* dest) const = 0;
    virtual R operator()(A&&...args) const = 0;
    virtual ~Callable() = default;
};

template<typename R, typename...A>
struct CallableFunction: public Callable<R, A...>
{
    using FP = R(*)(A...);
    using Super = Callable<R, A...>;
    using Self = CallableFunction<R, A...>;

    explicit CallableFunction(FP f): function_{f} {}
    void clone(void* dest) const override { internal::copy_data(function_, dest); }

    R operator()(A&&...args) const override { if(is_void<R>::value) function_(forward<A>(args)...);
        else return function_(forward<A>(args)...); }

private:
    FP function_;
};

template<typename O, typename R, typename...A>
struct CallableMethod: public Callable<R, A...>
{
    using MP = R(O::*)(A...); // Pointer to member function type
    using Super = Callable<R, A...>;
    using Self = CallableMethod<O, R, A...>;

    CallableMethod(O& o, MP m): f_{o, m} {}
    void clone(void* dest) const override { internal::copy_data(f_, dest); }

    R operator()(A&&...args) const override { if(is_void<R>::value) (f_.object_.*f_.method_)(forward<A>(args)...);
        else return (f_.object_.*f_.method_)(forward<A>(args)...); }

private:
    struct fields { O& object_; MP method_; } f_{};
};

template<typename O, typename R, typename...A>
struct CallableConstMethod: public Callable<R, A...>
{
    using MP = R(O::*)(A...) const; // Pointer to member function type
    using Super = Callable<R, A...>;
    using Self = CallableMethod<O, R, A...>;

    CallableConstMethod(const O& o, MP m): f_{o, m} {}
    void clone(void* dest) const override { internal::copy_data(f_, dest); }

    R operator()(A&&...args) const override { if(is_void<R>::value) (f_.object_.*f_.method_)(forward<A>(args)...);
        else return (f_.object_.*f_.method_)(forward<A>(args)...); }

private:
    struct fields { const O& object_; MP method_; } f_{};
};

template<typename>
struct Callback;

template <typename R, typename... A>
struct Callback<R(*)(A...)>
{
    using FP = R(*)(A...);
    using Self = Callback<R(*)(A...)>;

    // Empty callback
    Callback() noexcept = default;
    explicit Callback(nullptr_t) noexcept {}

    // From a function
    explicit Callback(FP f): isNull_{false} { place<CallableFunction<R, A...>>(f); }

    // From a member function and object reference
    template <typename O>
    Callback(O& o, R(O::*m)(A...)): isNull_{false}  { place<CallableMethod<O, R, A...>>(o, m); }

    // From a member function and object pointer
    template <typename O>
    Callback(O* o, R(O::*m)(A...)): isNull_{false}  { place<CallableMethod<O, R, A...>>(*o, m); }

    // From a const member function and object reference
    template <typename O>
    Callback(const O& o, R(O::*m)(A...)): isNull_{false}  { place<CallableConstMethod<O, R, A...>>(o, m); }

    // From a const member function and object pointer
    template <typename O>
    Callback(const O* o, R(O::*m)(A...)): isNull_{false}  { place<CallableConstMethod<O, R, A...>>(*o, m); }

    // Captured lambda specialization
    template<typename L>
    explicit Callback(const L& l): isNull_{false}  { place<CallableFunction<R, A...>>(l); }

    // From another Callback
    Callback(const Self& cb): isNull_{cb.isNull_}  { copy_buffer(cb.buffer_); }

    // Assignment
    Callback& operator=(const Self& cb) { if(&cb != this) { isNull_ = cb.isNull_;  copy_buffer(cb.buffer_); } return *this; };
    Callback& operator=(nullptr_t) { isNull_ = true; return *this; }

    // Call
    R operator()(A&&... args)
        { if(is_void<R>::value) { if(!isNull_) (*callable())(forward<A>(args)...); }
          else { return !isNull_ ? (*callable())(forward<A>(args)...) : R(); } }

    // Boolean
    explicit operator bool() const noexcept { return !isNull_; }

private:
    void copy_buffer(const char* from) { copy(from, from + BUFFER_SIZE, buffer_); }
    Callable<R, A...>* callable() { return reinterpret_cast<Callable<R, A...>*>(buffer_); }

    template<typename T, typename... Args> void place(Args&&... args)
    { static_assert(sizeof(T) <= BUFFER_SIZE, "Buffer is too small"); new(buffer_) T(forward<Args>(args)...); }

private:
    static const size_t BUFFER_SIZE = 8;
    char buffer_[BUFFER_SIZE] = {};
    bool isNull_ = true;
};

}

#endif // ADV_CALLBACKS_H
