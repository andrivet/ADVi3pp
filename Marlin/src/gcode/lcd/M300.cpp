/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2020 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Based on Sprinter and grbl.
 * Copyright (c) 2011 Camiel Gubbels / Erik van der Zalm
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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "../../inc/MarlinConfig.h"

#if HAS_SOUND

#include "../gcode.h"

#include "../../lcd/marlinui.h" // i2c-based BUZZ
#include "../../libs/buzzer.h"  // Buzzer, if possible

/**
 * M300: Play a Tone / Add a tone to the queue
 *
 *  S<frequency> - (Hz) The frequency of the tone. 0 for silence.
 *  P<duration>  - (ms) The duration of the tone.
 *
 * With SOUND_MENU_ITEM:
 *  E<0|1>       - Mute or enable sound
 */
void GcodeSuite::M300() {

  #if ENABLED(SOUND_MENU_ITEM)
    if (parser.seen('E')) {
      ui.sound_on = parser.value_bool();
      return;
    }
  #endif

  uint16_t const frequency = parser.ushortval('S', 0); // @advi3++, 0 means default value
  uint16_t duration = parser.ushortval('P', 0); // @advi3++, 0 means default value

  // Limits the tone duration to 0-5 seconds.
  NOMORE(duration, 5000U);

  // @advi3++
  if(parser.seenval('D'))
    ui.set_tone(frequency, duration, parser.ushortval('D', 0));
  else
    ui.buzz_m300(duration, frequency);
}

void GcodeSuite::M300_report(const bool forReplay/*=true*/) {
  report_heading(forReplay, F("Play beep sound"));
  report_echo_start(forReplay);
  SERIAL_ECHOPGM("  M300");
  SERIAL_ECHOPGM(" P", ui.tone_duration);
  SERIAL_ECHOPGM(" S", ui.tone_frequency);
  SERIAL_ECHOPGM(" D", ui.tone_options);
  SERIAL_EOL();
}

#endif // HAS_SOUND
