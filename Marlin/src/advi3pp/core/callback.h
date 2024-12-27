/**
 * ADVi3++ Firmware For Wanhao Duplicator i3 Plus (based on Marlin 2)
 *
 * Copyright (C) 2017-2020 Sebastien Andrivet [https://github.com/andrivet/]
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

#include <stdint.h>
#include <stddef.h>
#include "../lib/ADVstd/bitmasks.h"
#include "logging.h"

namespace ADVi3pp {
  enum struct CALLBACK_SOURCE: uint8_t { BACK, SAVE };
  enum struct CALLBACK_RESULT: uint8_t { STOP, CONTINUE };
  enum struct CALLBACK_KIND: uint8_t { PARAM_0 = 0x00, PARAM_1 = 0x01, RETURN = 0x02 };
}

ENABLE_BITMASK_OPERATOR(ADVi3pp::CALLBACK_KIND);

namespace ADVi3pp {

  // Similar to a std::variant
  struct DynamicCallback {
    DynamicCallback(): kind_{CALLBACK_KIND::PARAM_0}, func0_{nullptr} {}
    explicit DynamicCallback(nullptr_t): kind_{CALLBACK_KIND::PARAM_0}, func0_{nullptr} {}

    operator bool() const { return func0_ != nullptr; }

    DynamicCallback& operator=(nullptr_t) {
      kind_ = CALLBACK_KIND::PARAM_0;
      func0_ = nullptr;
      return *this;
    }

    DynamicCallback& operator=(void (*cb)()) {
      kind_ = CALLBACK_KIND::PARAM_0;
      func0_ = cb;
      return *this;
    }

    DynamicCallback& operator=(void (*cb)(CALLBACK_SOURCE)) {
      kind_ = CALLBACK_KIND::PARAM_1;
      func1_ = cb;
      return *this;
    }

    DynamicCallback& operator=(CALLBACK_RESULT (*cb)()) {
      kind_ = CALLBACK_KIND::PARAM_0 | CALLBACK_KIND::RETURN;
      func2_ = cb;
      return *this;
    }

    DynamicCallback& operator=(CALLBACK_RESULT (*cb)(CALLBACK_SOURCE)) {
      kind_ = CALLBACK_KIND::PARAM_1 | CALLBACK_KIND::RETURN;
      func3_ = cb;
      return *this;
    }

    CALLBACK_RESULT operator()() {
      assert(!test_one_bit(kind_, CALLBACK_KIND::PARAM_1));
      if(test_one_bit(kind_, CALLBACK_KIND::RETURN)) return func2_ ? func2_() : CALLBACK_RESULT::CONTINUE;
      if(func0_) func0_();
      return CALLBACK_RESULT::CONTINUE;
    }

    CALLBACK_RESULT operator()(CALLBACK_SOURCE src) {
      auto param1 = test_one_bit(kind_, CALLBACK_KIND::PARAM_1);
      auto ret = test_one_bit(kind_, CALLBACK_KIND::RETURN);
      if(param1) {
        if(ret) return func3_ ? func3_(src) : CALLBACK_RESULT::CONTINUE;
        if(func1_) func1_(src);
        return CALLBACK_RESULT::CONTINUE;
      }
      if(ret) return func2_ ? func2_() : CALLBACK_RESULT::CONTINUE;
      if(func0_) func0_();
      return CALLBACK_RESULT::CONTINUE;
    }

  private:
    CALLBACK_KIND kind_;
    union {
      void (*func0_)();
      void (*func1_)(CALLBACK_SOURCE);
      CALLBACK_RESULT (*func2_)();
      CALLBACK_RESULT (*func3_)(CALLBACK_SOURCE);
    };
  };

}
