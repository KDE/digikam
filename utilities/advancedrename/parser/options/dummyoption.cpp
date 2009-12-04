/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 200X-XX-XX
 * Description : an option to provide <FILL IN PURPOSE> information to the parser
 *
 * Copyright (C) 2009 by YOUR NAME <YOUR MAIL ADDRESS>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "dummyoption.moc"

// Qt includes

#include <QFileInfo>

// KDE includes

#include <kiconloader.h>
#include <klocale.h>

namespace Digikam
{

DummyOption::DummyOption()
           : Option(i18n("BUTTON TEXT"),
                    i18n("TOOLTIP TEXT FOR BUTTON"),
                    SmallIcon("ICON"))
{
    /*
     * Example initialization
     *
     * Use addToken() to provide at least one entry in the tooltip for the AdvancedRename utility.
     * It is possible to call this method more than one time, to have additional token information.
     * If you want to add parameters to your rename option, enclose them in pipe characters, to have them
     * properly marked in the tooltip, e.g.
     *
     * addToken("[myoption:|parameter|]", i18nc("my rename option", "MyOption"),
     *
     * Use setRegExp() to define the regular expression that identifies the parse option and its parameters
     */

    addToken("[myoption]", i18nc("my rename option", "MyOption"), i18n("my option description"));

    // --------------------------------------------------------

    QRegExp reg("<PARSING REGEXP>");

    // decide if the regexp is case sensitive
    reg.setCaseSensitivity(Qt::CaseInsensitive);

    // decide if the regexp matching is greedy
    reg.setMinimal(false);

    setRegExp(reg);
}

void DummyOption::parseOperation(const QRegExp& regExp, ParseSettings& settings)
{
    QRegExp reg = regExp();

    // --------------------------------------------------------

    // the string that will hold the information you want to be returned by the rename option
    QString tmp;

    /*
     * Always use the PARSE_LOOP_START / PARSE_LOOP_END macros, do not remove them.
     * Just fill in the code between these macros to extract the values from the regexp and provide the
     * information needed by this rename option.
     */
    PARSE_LOOP_START(parseString, reg)
    {
        /*
         * THE REAL PARSING HAPPENS IN HERE
         *
         * For example:
         */

        if (reg.cap(1) == QString("[myoption]"))
        {
            tmp = doSomething();
        }
    }
    PARSE_LOOP_END(parseString, reg, tmp, results)
}

} // namespace Digikam
