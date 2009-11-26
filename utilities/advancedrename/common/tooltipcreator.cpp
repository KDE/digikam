/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-09-14
 * Description : a class to build the tooltip for a renameparser and its options
 *
 * Copyright (C) 2009 by Andi Clemens <andi dot clemens at gmx dot net>
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

#include "tooltipcreator.h"

// Qt includes

#include <QRegExp>

// KDE includes

#include <klocale.h>

// Local includes

#include "themeengine.h"
#include "parser.h"
#include "option.h"
#include "modifier.h"

using namespace Digikam;

namespace Digikam
{

TooltipCreator::TooltipCreator(Parser* _parser)
              : parser(_parser)
{
}

TooltipCreator::~TooltipCreator()
{
}

QString TooltipCreator::tooltip()
{
    if (!parser)
    {
        return QString();
    }

    QString tooltip;
    tooltip += QString("<qt><table cellspacing=\"0\" cellpadding=\"0\" border=\"0\">");

    // --------------------------------------------------------

    tooltip += createHeader(i18n("Renaming Options"));
    OptionsList op = parser->options();
    tooltip += createEntries(op);

    tooltip += QString("<tr></tr>");

    tooltip += createHeader(i18n("Modifiers"));
    ModifierList mod = parser->modifiers();
    tooltip += createEntries(mod);

    // --------------------------------------------------------

    tooltip += QString("</table></qt>");
    tooltip += i18n("<p><i>Modifiers can be applied to every renaming option. <br/>"
            "It is possible to assign multiple modifiers to an option, "
            "they are applied in the order you assign them.</i></p>");

    return tooltip;
}

QString TooltipCreator::markOption(const QString& str)
{
    QString tmp = str;

    QRegExp optionsRegExp("\\|(.*)\\|");
    optionsRegExp.setMinimal(true);

    tmp.replace(optionsRegExp, QString("<i><font color=\"%1\">\\1</font></i>")
                               .arg(ThemeEngine::instance()->textSpecialRegColor().name()));
    return tmp;
}

QString TooltipCreator::createHeader(const QString& str)
{
    QString tmp;
    tmp += QString("<tr bgcolor=\"%1\"><td colspan=\"2\">"
                   "<nobr><font color=\"%2\"><center><b>")
                   .arg(ThemeEngine::instance()->baseColor().name())
                   .arg(ThemeEngine::instance()->textRegColor().name());
    tmp += QString(str);
    tmp += QString("</b></center></font></nobr></td></tr>");
    return tmp;
}

template <class T>
QString TooltipCreator::createEntries(QList<T*> &data)
{
    QString tmp;

    foreach (T* t, data)
    {
        foreach (Token* token, t->tokens())
        {
            tmp += QString("<tr>"
                           "<td bgcolor=\"%1\">"
                           "<font color=\"%2\"><b>&nbsp;%3&nbsp;</b></font></td>"
                           "<td>&nbsp;%4&nbsp;</td></tr>")
                           .arg(ThemeEngine::instance()->baseColor().name())
                           .arg(ThemeEngine::instance()->textRegColor().name())
                           .arg(markOption(token->id()))
                           .arg(markOption(token->description()));
        }
    }

    return tmp;
}

} // namespace Digikam
