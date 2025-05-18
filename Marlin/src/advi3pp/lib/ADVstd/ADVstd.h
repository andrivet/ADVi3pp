/**
 * ADVstd - Minimalist entities from the Standard Library
 *
 * Copyright (C) 2018-2020 Sebastien Andrivet [https://github.com/andrivet/]
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

#if !defined(ADV_STD_SIZE_T)
using size_t = unsigned int;
using ptrdiff_t = int;
using nullptr_t = decltype(nullptr);

// Default placement new
inline void* operator new(size_t, void* p) noexcept { return p; }
inline void* operator new[](size_t, void* p) noexcept { return p; }
inline void operator delete(void*, unsigned int) { }
#endif

namespace adv {

  // Not in the standard, but useful
  template <typename T, size_t N>
  constexpr size_t count_of(T const (&)[N]) noexcept { return N; }

  template<typename T> struct remove_reference { using type = T; };
  template<typename T> struct remove_reference<T&> { using type = T; };
  template<typename T> struct remove_reference<T&&> { using type = T; };

  template <typename T> struct remove_cv { using type = T; };
  template <typename T> struct remove_cv<const T> { using type = T; };
  template <typename T> struct remove_cv<volatile T> { using type = T; };
  template <typename T> struct remove_cv<const volatile T> { using type = T; };

  template <typename T>
  struct remove_cvref {
    using type = typename remove_cv<typename remove_reference<T>::type>::type;
  };

  template<typename T, T v>
  struct integral_constant
  {
      static constexpr const T value = v;
      typedef T value_type;
      typedef integral_constant type;
      inline constexpr explicit operator value_type() const noexcept {return value;}
      inline constexpr value_type operator ()() const noexcept {return value;}
  };

  template<bool B>
  using bool_constant = integral_constant<bool, B>;

  using true_type  = bool_constant<true>;
  using false_type = bool_constant<false>;

  template<typename T> struct is_lvalue_reference     : false_type {};
  template<typename T> struct is_lvalue_reference<T&> : true_type {};

  template<typename T> struct is_integral           : false_type {};
  template<> struct is_integral<bool>               : true_type {};
  template<> struct is_integral<char>               : true_type {};
  template<> struct is_integral<signed char>        : true_type {};
  template<> struct is_integral<unsigned char>      : true_type {};
  template<> struct is_integral<wchar_t>            : true_type {};
  template<> struct is_integral<char16_t>           : true_type {};
  template<> struct is_integral<char32_t>           : true_type {};
  template<> struct is_integral<short>              : true_type {};
  template<> struct is_integral<unsigned short>     : true_type {};
  template<> struct is_integral<int>                : true_type {};
  template<> struct is_integral<unsigned int>       : true_type {};
  template<> struct is_integral<long>               : true_type {};
  template<> struct is_integral<unsigned long>      : true_type {};
  template<> struct is_integral<long long>          : true_type {};
  template<> struct is_integral<unsigned long long> : true_type {};

  template<typename T>
  struct is_void { static constexpr bool value = false;};
  template<> struct is_void<void> { static constexpr bool value = true; };

  template<typename...> using void_t = void;

  template <class T> struct is_enum: public integral_constant<bool, __is_enum(T)> {};

  template <typename T>
  struct is_array {
    static constexpr bool value = false;
  };

  template <typename T>
  struct is_array<T[]> {
    static constexpr bool value = true;
  };

  template <typename T, size_t N>
  struct is_array<T[N]> {
    static constexpr bool value = true;
  };

  template <typename T>
  struct remove_extent {
    using type = T;
  };

  template <typename T>
  struct remove_extent<T[]> {
    using type = T;
  };

  template <typename T, size_t N>
  struct remove_extent<T[N]> {
    using type = T;
  };

  template <typename T>
  using remove_extent_t = typename remove_extent<T>::type;

  template <typename T>
  struct is_function {
    static constexpr bool value = false;
  };

  template <typename Ret, typename... Args>
  struct is_function<Ret(Args...)> {
    static constexpr bool value = true;
  };

  template <typename Ret, typename... Args>
  struct is_function<Ret(Args...) noexcept> {
    static constexpr bool value = true;
  };

  template <typename Ret, typename... Args>
  struct is_function<Ret(Args..., ...)> {
    static constexpr bool value = true;
  };

  template <typename Ret, typename... Args>
  struct is_function<Ret(Args..., ...) noexcept> {
    static constexpr bool value = true;
  };

  template<class T, class U>
  struct is_same : false_type {};

  template<class T>
  struct is_same<T, T> : true_type {};

  template<bool C, typename T, typename F>
  struct conditional {
    using type = T;
  };

  template<typename T, typename F>
  struct conditional<false, T, F> {
    using type = F;
  };

  template<bool C, typename T, typename F>
  using conditional_t = typename conditional<C, T, F>::type;

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

  template<typename T, typename> struct alr_ { using type = T; };
  template<typename T>           struct alr_<T, void_t<T&>> { using type = T&; };
  template<typename T, typename> struct arr_ { using type = T; };
  template<typename T>           struct arr_<T, void_t<T&&>> { using type = T&&; };
  template<typename T, typename> struct ap_ { using type = T; };
  template<typename T>           struct ap_<T, void_t<T*>> { using type = T*; };

  template<typename T>           struct add_lvalue_reference: alr_<T, void> {};
  template<typename T>           struct add_rvalue_reference_t: arr_<T, void> {};
  template<typename T>           struct add_pointer: ap_<T, void> {};

  template<typename T> void swap(T& a, T& b) { T c{move(a)}; a = move(b); b = move(c); }

  template<typename T> auto declval() noexcept -> add_rvalue_reference_t<T>;

  template <typename T>
  struct decay {
  private:
    using U = typename remove_cvref<T>::type;
  public:
    using type = conditional_t<
      is_array<U>::value,
      typename remove_extent<U>::type*,
      conditional_t<is_function<U>::value, U*, U>
    >;
  };

  template <typename T>
  using decay_t = typename decay<T>::type;

  template<bool, typename T = void> struct enable_if {};
  template<typename T> struct enable_if<true, T> { using type = T; };
  template< bool B, typename T = void > using enable_if_t = typename enable_if<B, T>::type;

  template<typename I, typename O>
  inline O copy(I first, I last, O d_first)
  {
      while(first != last)
          *(d_first++) = *(first++);
      return d_first;
  }

  template<typename I, typename O>
  inline O reverse_copy(I first, I last, O d_first)
  {
      while(first != last)
          *(d_first++) = *(--last);
      return d_first;
  }

  template<typename T>
  struct assert_false: false_type {};

  template <class T, bool = is_enum<T>::value>
  struct underlying_type_impl;

  template <class T>
  struct underlying_type_impl<T, false> {};

  template <class T> struct underlying_type_impl<T, true>
  {
      typedef __underlying_type(T) type;
  };

  template <class T>
  struct underlying_type: underlying_type_impl<T, is_enum<T>::value> {};

  template <class T> using underlying_type_t = typename underlying_type<T>::type;

  namespace detail {
      struct ignore_t {
          template <typename T>
          constexpr void operator=(T&&) const noexcept {}
      };
  }

  constexpr detail::ignore_t ignore;

}
