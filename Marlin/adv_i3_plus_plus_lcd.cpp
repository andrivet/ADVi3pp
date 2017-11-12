/**
 * Marlin 3D Printer Firmware For Wanhao Duplicator i3 Plus (ADVi3++)
 *
 * Copyright (C) 2017 Sebastien Andrivet [https://github.com/andrivet/]
 *
 * Copyright (C) 2016 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Based on Sprinter and grbl.
 * Copyright (C) 2011 Camiel Gubbels / Erik van der Zalm
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

#include "Marlin.h"
#include "adv_i3_plus_plus.h"
#include "adv_i3_plus_plus_impl.h"

namespace advi3pp {

// --------------------------------------------------------------------
// LCD
// --------------------------------------------------------------------

namespace { LCDImpl lcd; }

void LCD::update()
{
    lcd.update();
}

void LCD::init()
{
    lcd.init();
}

bool LCD::has_status()
{
    return lcd.has_status();
}

void LCD::set_status(const char* message, bool /*persist*/)
{
    lcd.set_status(message);
}

void LCD::set_status_PGM(const char* message, int8_t /*level*/)
{
    lcd.set_status_PGM(message);
}

void LCD::set_alert_status_PGM(const char* message)
{
    lcd.set_alert_status_PGM(message);
}

void LCD::status_printf_P(int8_t /*level*/, const char * const fmt, va_list args)
{
    lcd.status_printf_P(fmt, args);
}

void LCD::buttons_update()
{
    lcd.buttons_update();
}

void LCD::reset_alert_level()
{
    lcd.reset_alert_level();
}

bool LCD::detected()
{
    return lcd.detected();
}

void LCD::refresh()
{
    lcd.refresh();
}

// --------------------------------------------------------------------
// LCDImpl
// --------------------------------------------------------------------

LCDImpl& LCDImpl::instance()
{
    return lcd;
}

void LCDImpl::update()
{
    /* Do nothing */
}

void LCDImpl::init()
{
    /* Do nothing */
}

bool LCDImpl::has_status()
{
    return message_.length() > 0 || messagePGM_.length() > 0;
}

void LCDImpl::set_status(const char* message)
{
	ADVi3PP_LOG(F("STATUS: ") << message);
    message_ = message;
	recompute_whole_message_ = true;
}

void LCDImpl::set_status_PGM(const char* message)
{
    ADVi3PP_LOG(F("STATUS PGM: ") << message);
    messagePGM_ = message;
	recompute_whole_message_ = true;
}

void LCDImpl::set_alert_status_PGM(const char* message)
{
    set_status_PGM(message);
}

void LCDImpl::status_printf_P(const char * fmt, va_list argp)
{
    messagePGM_ = StringPrintWithFormat(fmt, argp);
	recompute_whole_message_ = true;
    ADVi3PP_LOG(F("STATUS V: ") << message_);
}

void LCDImpl::buttons_update()
{
    /* Do nothing */
}

void LCDImpl::reset_alert_level()
{
    /* Do nothing */
}

bool LCDImpl::detected()
{
    return true;
}

void LCDImpl::refresh()
{
    /* Do nothing */
}

const String& LCDImpl::get_message() const
{
	if(recompute_whole_message_)
	{
		whole_message_ = message_;
		if(message_.length() > 0 && messagePGM_.length() > 0)
			whole_message_ += " - ";
		whole_message_ += messagePGM_;
		recompute_whole_message_ = false;
	}
    return whole_message_;
}

}