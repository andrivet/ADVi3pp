/**
 * ADVcrtp - Helpers to implement compile-time polymorphism
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

#include "ADVstd.h"

#ifndef ADVLIB_ADVPOLY_H
#define ADVLIB_ADVPOLY_H

namespace adv {

// --------------------------------------------------------------------
// Curiously Recurring Template Pattern
// More or less from: https://www.fluentcpp.com/2017/05/19/crtp-helper/
// --------------------------------------------------------------------

template <typename Self, template<typename> class P>
struct Crtp
{
    Self& self()             { return static_cast<Self&>(*this); }
    Self const& self() const { return static_cast<Self const&>(*this); }

protected:
    using Parent = P<Self>;

private:
    Crtp() = default;
    friend Parent;
};

}

#endif //ADVLIB_ADVPOLY_H
