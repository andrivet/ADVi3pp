/**
 * ADVi3++ Firmware For Wanhao Duplicator i3 Plus (based on Marlin)
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

#include "../../../lcd/extui/ui_api.h"
#include "../../lib/ADVstd/ADVcrtp.h"
#include "../../core/settings.h"
#include "../../core/enums.h"
#include "../../core/eeprom.h"
#include "../../core/pages.h"

namespace ADVi3pp {


//! Handle inputs from the LCD Panel
template<typename Self>
struct Screen: adv::Crtp<Self, Screen>
{
public:
    void handle(KeyValue value);
    void show(ShowOptions options);

    bool dispatch(KeyValue value) { return this->self().do_dispatch(value); }
    void show_command() { this->self().do_show_command(); }
    void save_command() { this->self().do_save_command(); }
    void back_command() { this->self().do_back_command(); }
    Page prepare_page() { return this->self().do_prepare_page(); }
    void write(EepromWrite& eeprom) const { this->self().do_write(eeprom); }
    void read(EepromRead& eeprom) { this->self().do_read(eeprom); }
    bool validate(EepromRead& eeprom) { return this->self().do_validate(eeprom); }
    void reset() { this->self().do_reset(); }
    uint16_t size_of() const { return this->self().do_size_of(); }

protected:
    Page do_prepare_page();
    bool do_dispatch(KeyValue value);
    void do_show_command();
    void do_save_command();
    void do_back_command();
    void invalid(KeyValue value);

private:
    void do_write(EepromWrite& eeprom) const {}
    void do_read(EepromRead& eeprom) {}
    bool do_validate(EepromRead& eeprom) { return true; }
    void do_reset() {}
    uint16_t do_size_of() const { return 0; }
};

// --------------------------------------------------------------------
// Screen implementation
// --------------------------------------------------------------------

template<typename Self>
Page Screen<Self>::do_prepare_page()
{
    return Page::None;
}

template<typename Self>
void Screen<Self>::handle(KeyValue value)
{
    if(!dispatch(value))
        invalid(value);
}

template<typename Self>
bool Screen<Self>::do_dispatch(KeyValue value)
{
    switch(value)
    {
        case KeyValue::Show: show_command(); break;
        case KeyValue::Save: save_command(); break;
        case KeyValue::Back: back_command(); break;
        default: return false;
    }

    return true;
}

template<typename Self>
void Screen<Self>::invalid(KeyValue value)
{
    Log::error() << F("Invalid key value ") << static_cast<uint16_t>(value) << Log::endl();
}

template<typename Self>
void Screen<Self>::show(ShowOptions options)
{
    Page page = prepare_page();
    if(page != Page::None)
        pages.show_page(page, options);
}

template<typename Self>
void Screen<Self>::do_show_command()
{
    show(ShowOptions::SaveBack);
}

template<typename Self>
void Screen<Self>::do_save_command()
{
    settings.save();
    pages.show_forward_page();
}

template<typename Self>
void Screen<Self>::do_back_command()
{
    pages.show_back_page();
}

}
