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

namespace adv {

  template <typename... Ts>
  struct tuple;

  template <typename T, typename... R>
  struct tuple<T, R...> {
    T value;
    tuple<R...> rest;

    tuple() = default;
    tuple(T v, R... rest_vals): value(v), rest(rest_vals...) {}
  };

  template <>
  struct tuple<> {}; // Empty tuple

  template <size_t index, typename T, typename... R>
  struct tuple_element {
    using type = typename tuple_element<index - 1, R...>::type;
  };

  template <typename T, typename... R>
  struct tuple_element<0, T, R...> {
    using type = T;
  };

  // get<index>(tuple)
  template <size_t index, typename T, typename... R>
  auto& get(tuple<T, R...>& t) {
    if constexpr (index == 0)
      return t.value;
    else
      return get<index - 1>(t.rest);
  }

  // get<index>(tuple)
  template <size_t index, typename T, typename... R>
  const auto& get(const tuple<T, R...>& t) {
    if constexpr (index == 0)
      return t.value;
    else
      return get<index - 1>(t.rest);
  }

  // make_tuple
  template <typename... Ts>
  auto make_tuple(Ts&&... args) {
    return tuple<decay_t<Ts>...>(forward<Ts>(args)...);
  }

  template <typename T>
  struct tuple_size;

  template <typename... Ts>
  struct tuple_size<tuple<Ts...>> : adv::integral_constant<size_t, sizeof...(Ts)> {};
}
