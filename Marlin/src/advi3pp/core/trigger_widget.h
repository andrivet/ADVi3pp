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

#include "../../inc/MarlinConfig.h"
#include "core.h"

namespace ADVi3pp {

  enum class TRANSITION: uint8_t {
    LOW_2_HIGH = 0,
    HIGH_2_LOW = 1
  };

  // N: (non-inverted) signal transition, T: trigger variable, S: signal variable, U: pull variable, P: signal pin
  template <TRANSITION N, Variable T, Variable S = Variable::Value0, Variable U = Variable::Value1, int P = 0>
  struct TriggerWidget {
    static constexpr TRANSITION INVERTED = (N == TRANSITION::LOW_2_HIGH) ? TRANSITION::HIGH_2_LOW : TRANSITION::LOW_2_HIGH;

    void set_inverted(bool inverted);
    [[nodiscard]] bool get_inverted() const;
    void set_trigger_state(TRANSITION state);
    [[nodiscard]] TRANSITION get_trigger_state() const;
    [[nodiscard]] bool get_signal_state() const;
    [[nodiscard]] Core::PIN_PULL get_signal_pull() const;
    void send_signal_state() const;
    void high2low_command();
    void low2high_command();

  private:
    void send_trigger_state();

  private:
    TRANSITION transition_ = N;
  };

  template <TRANSITION N, Variable T, Variable S, Variable U, int P>
  inline void TriggerWidget<N, T, S, U, P>::set_inverted(bool inverted) {
    transition_ = inverted ? INVERTED : N;
  }

  template <TRANSITION N, Variable T, Variable S, Variable U, int P>
  inline bool TriggerWidget<N, T, S, U, P>::get_inverted() const {
    return transition_ != N;
  }

  template <TRANSITION N, Variable T, Variable S, Variable U, int P>
  inline void TriggerWidget<N, T, S, U, P>::set_trigger_state(TRANSITION state) {
    transition_ = state;
  }

  template <TRANSITION N, Variable T, Variable S, Variable U, int P>
  inline TRANSITION TriggerWidget<N, T, S, U, P>::get_trigger_state() const {
    return transition_;
  }

  template <TRANSITION N, Variable T, Variable S, Variable U, int P>
  bool TriggerWidget<N, T, S, U, P>::get_signal_state() const {
    static_assert(P != 0);
    return Core::get_pin_state(P) == Core::PIN_STATE::High;
  }

  template <TRANSITION N, Variable T, Variable S, Variable U, int P>
  inline Core::PIN_PULL TriggerWidget<N, T, S, U, P>::get_signal_pull() const {
    static_assert(P != 0);
    return Core::get_pin_pull(P);
  }

  template <TRANSITION N, Variable T, Variable S, Variable U, int P>
  void TriggerWidget<N, T, S, U, P>::high2low_command() {
    set_trigger_state(TRANSITION::HIGH_2_LOW);
    send_trigger_state();
  }

  template <TRANSITION N, Variable T, Variable S, Variable U, int P>
  void TriggerWidget<N, T, S, U, P>::low2high_command() {
    set_trigger_state(TRANSITION::LOW_2_HIGH);
    send_trigger_state();
  }

  template <TRANSITION N, Variable T, Variable S, Variable U, int P>
  void TriggerWidget<N, T, S, U, P>::send_signal_state() const {
    static_assert(P != 0);
    WriteRamRequest{S}.write_word(get_signal_state());
    WriteRamRequest{U}.write_word(get_signal_pull());
  }

  template <TRANSITION N, Variable T, Variable S, Variable U, int P>
  void TriggerWidget<N, T, S, U, P>::send_trigger_state() {
    WriteRamRequest{T}.write_word(get_trigger_state());
  }

}
