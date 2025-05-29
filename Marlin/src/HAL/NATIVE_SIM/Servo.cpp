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

/**
 * servo.cpp - Interrupt driven Servo library for Arduino using 16 bit timers- Version 2
 * Copyright (c) 2009 Michael Margolis.  All right reserved.
 */

/**
 * A servo is activated by creating an instance of the Servo class passing the desired pin to the attach() method.
 * The servos are pulsed in the background using the value most recently written using the write() method
 *
 * Note that analogWrite of PWM on pins associated with the timer are disabled when the first servo is attached.
 * Timers are seized as needed in groups of 12 servos - 24 servos use two timers, 48 servos will use four.
 *
 * The methods are:
 *
 * Servo - Class for manipulating servo motors connected to Arduino pins.
 *
 * attach(pin)           - Attach a servo motor to an i/o pin.
 * attach(pin, min, max) - Attach to a pin, setting min and max values in microseconds
 *                         Default min is 544, max is 2400
 *
 * write()               - Set the servo angle in degrees. (Invalid angles —over MIN_PULSE_WIDTH— are treated as µs.)
 * writeMicroseconds()   - Set the servo pulse width in microseconds.
 * move(pin, angle)      - Sequence of attach(pin), write(angle), safe_delay(servo_delay[servoIndex]).
 *                         With DEACTIVATE_SERVOS_AFTER_MOVE it detaches after servo_delay[servoIndex].
 * read()                - Get the last-written servo pulse width as an angle between 0 and 180.
 * readMicroseconds()    - Get the last-written servo pulse width in microseconds.
 * attached()            - Return true if a servo is attached.
 * detach()              - Stop an attached servo from pulsing its i/o pin.
 */

#include "../../inc/MarlinConfig.h"

#if HAS_SERVOS

#include "../shared/servo.h"
#include "../shared/servo_private.h"

void initISR(const timer16_Sequence_t timer_index) {
}

void finISR(const timer16_Sequence_t timer_index) {
}

#endif // HAS_SERVOS

