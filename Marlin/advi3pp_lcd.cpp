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
#include "cardreader.h"
#include "advi3pp.h"
#include "advi3pp_impl.h"

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

void LCD::queue_message(const String &message)
{
    lcd.queue_message(message);
}

void LCD::reset_message()
{
    lcd.reset_messaage();
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
    return message_.length() > 0;
}

void LCDImpl::set_status(const char* message)
{
	Log::log() << F("STATUS: ") << message << Log::endl();
    message_ = message;
}

void LCDImpl::set_status_PGM(const char* message)
{
    message_ = String{reinterpret_cast<const __FlashStringHelper*>(message)};
    Log::log() << F("STATUS PGM: ") << message_ << Log::endl();
}

void LCDImpl::set_alert_status_PGM(const char* message)
{
    set_status_PGM(message);
}

void LCDImpl::status_printf_P(const char * fmt, va_list args)
{
    static const size_t MAX_SIZE = 100;
    static char buffer[MAX_SIZE + 1];

    vsnprintf_P(buffer, MAX_SIZE, fmt, args);
    message_ = String{buffer};
    Log::log() << F("STATUS V: ") << message_ << Log::endl();
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
	return message_;
}

void LCDImpl::queue_message(const String &message)
{
    String msg{F("M117 ")}; msg << message;
    enqueue_and_echo_command(msg.c_str());
}

void LCDImpl::reset_messaage()
{
     enqueue_and_echo_commands_P(PSTR("M117"));
}

void LCDImpl::set_progress_name(const String& name)
{
    progress_name_ = name;
    progress_percent_ = "";
    percent_ = -1;
}

const String& LCDImpl::get_progress() const
{
    if(progress_name_.length() <= 0)
        return progress_name_; // i.e. empty
    
    auto done = card.percentDone();
    if(done != percent_)
    {
        progress_percent_ = progress_name_ + " " + done + "%";
        percent_ = done;
    }
    return progress_percent_;
}

void LCDImpl::reset_progress()
{
    progress_name_ = "";
    progress_percent_ = "";
    percent_ = -1;
}

}

void lcd_status_printf_P(const uint8_t /*level*/, const char * const fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    advi3pp::lcd.status_printf_P(fmt, args);
    va_end(args);
}
