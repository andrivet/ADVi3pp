/**
 * Marlin 3D Printer Firmware For Wanhao Duplicator i3 Plus (ADVi3++)
 *
 * Copyright (C) 2017-2019 Sebastien Andrivet [https://github.com/andrivet/]
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
#ifndef ADV_I3_PLUS_PLUS_STACK_H
#define ADV_I3_PLUS_PLUS_STACK_H

#include <stddef.h>
#include "advi3pp_log.h"

namespace advi3pp {

// --------------------------------------------------------------------
// Stack - Simple Stack
// --------------------------------------------------------------------

template<typename T, size_t S>
struct Stack
{
    void push(T e);
    T pop();
    bool is_empty() const;
    bool contains(T e) const;

private:
    T elements_[S];
    size_t top_ = 0;
};

template<typename T, size_t S>
void Stack<T, S>::push(T e)
{
    assert(top_ <= S);
    elements_[top_++] = e;
}

template<typename T, size_t S>
T Stack<T, S>::pop()
{
    assert(!is_empty());
    return elements_[--top_];
}

template<typename T, size_t S>
bool Stack<T, S>::is_empty() const
{
    return top_ == 0;
}

template<typename T, size_t S>
bool Stack<T, S>::contains(T e) const
{
    for(size_t i = 0; i < top_; ++i)
        if(elements_[top_ - i - 1] == e)
            return true;
    return false;
}


// --------------------------------------------------------------------

}

#endif //ADV_I3_PLUS_PLUS_STACK_H
