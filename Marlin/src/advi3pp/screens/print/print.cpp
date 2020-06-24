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

#include "../../parameters.h"
#include "print.h"
#include "../../core/core.h"
#include "../../core/status.h"
#include "../core/wait.h"


namespace ADVi3pp {

Print print;

//! Handle print commands.
//! @param key_value    The sub-action to handle
//! @return             True if the action was handled
bool Print::do_dispatch(KeyValue value)
{
    if(Parent::do_dispatch(value))
        return true;

    switch(value)
    {
        case KeyValue::PrintStop:           stop_command(); break;
        case KeyValue::PrintPause:          pause_resume_command(); break;
        case KeyValue::PrintAdvancedPause:  advanced_pause_command(); break;
        default:                            return false;
    }

    return true;
}

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page Print::do_prepare_page()
{
    return Page::Print;
}

//! Stop printing
void Print::stop_command()
{
    if(!ExtUI::isPrinting())
        return;

    wait.show(F("Stop printing..."), ShowOptions::SaveBack);
    core.inject_commands(F("A1")); // TODO review this
}

//! Pause printing
void Print::pause_resume_command()
{
    if(!ExtUI::isPrinting())
        return;

    wait.show(F("Pause printing..."), ShowOptions::SaveBack);
    core.inject_commands(F("A0")); // TODO review this
}

//! Advanced Pause for filament change
void Print::advanced_pause_command()
{
    if(!ExtUI::isPrinting())
        return;

    wait.show(F("Pausing..."), ShowOptions::SaveBack);
    core.inject_commands(F("M600"));
}

//! Process Stop (A1) code and actually stop the print (if any running).
void Print::process_stop_code()
{
    ExtUI::stopPrint();

    status.set(F("Print Stopped"));
    pages.show_back_page();
    pages.show_back_page();
}

//! Process Pause (A0) code and actually pause the print (if any running).
void Print::process_pause_resume_code()
{
    core.inject_commands(F("M600"));
}

void Print::pause_finished()
{
    pages.show_back_page();
}


}
