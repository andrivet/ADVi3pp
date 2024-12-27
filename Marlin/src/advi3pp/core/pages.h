/**
 * ADVi3++ Firmware For Wanhao Duplicator i3 Plus (based on Marlin)
 *
 * Copyright (C) 2017-2025 Sebastien Andrivet [https://github.com/andrivet/]
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

#include "enums.h"
#include "logging.h"
#include "task.h"

namespace ADVi3pp::Pages {
  enum class SAVE_OPTIONS { NOTHING = 0x0000, SETTINGS = 0x0001, MESSAGE = 0x0002 };
  enum class BACK_OPTIONS { NONE = 0x0000, FINISH_MOVE = 0x0001 };
  enum class BACK_ALL_OPTIONS { NONE = 0x0000, SHOW_MAIN = 0x0001, SEND_BACK = 0x0002 };

  void show(Page page);
  Page get_current_page();
  void clear_temporaries(bool show = true);
  void clear_current();
  void back_all(BACK_ALL_OPTIONS options);
  void save_forward_page();
  void save(SAVE_OPTIONS save, BACK_OPTIONS options);
  void back(BACK_OPTIONS options);

  // Temporary means it can be removed when an action is done.
  inline bool is_temporary(Page page) {
    return
      page == Page::Temperatures ||
      page == Page::SdCard ||
      page == Page::WaitBack ||
      page == Page::WaitBackContinue ||
      page == Page::Wait ||
      page == Page::WaitContinue;
  }
}

ENABLE_BITMASK_OPERATOR(ADVi3pp::Pages::SAVE_OPTIONS);
ENABLE_BITMASK_OPERATOR(ADVi3pp::Pages::BACK_OPTIONS);
ENABLE_BITMASK_OPERATOR(ADVi3pp::Pages::BACK_ALL_OPTIONS);
