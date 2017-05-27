/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-09-14
 * Description : a class to build the tooltip for a renameparser and its options
 *
 * Copyright (C) 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
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
#include <QPalette>
#include <QApplication>
#include <QIcon>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "modifier.h"
#include "option.h"
#include "parser.h"

namespace Digikam
{

TooltipCreator& TooltipCreator::getInstance()
{
    static TooltipCreator m_instance;
    return m_instance;
}

QString TooltipCreator::additionalInformation()
{
    QStringList infoItems;
    infoItems << i18n("Modifiers can be applied to every renaming option.");
    infoItems << i18n("It is possible to assign multiple modifiers to an option, "
                      "they are applied in the order you assign them.");
    infoItems << i18n("Be sure to use the quick access buttons: They might provide "
                      "additional information about renaming and modifier options.");
    infoItems << i18n("The file list can be sorted, just right-click on it to see the sort criteria (album UI only).");

    QString information;
    information += QString::fromUtf8("<div style='margin-top:20px;'");

    information += tableStart(90);
    information += QString::fromUtf8("<tr><td style='vertical-align:top;'><img src='") + getInfoIconResourceName() +
                                                                                         QString::fromUtf8("' /></td>");
    information += QString::fromUtf8("<td><ol>");

    foreach(const QString& infoItem, infoItems)
    {
        information += QString::fromUtf8("<li>") + infoItem + QString::fromUtf8("</li>");

    }

    information += QString::fromUtf8("</ol></td></tr>");
    information += tableEnd();

    information += QString::fromUtf8("</div>");

    return information;
}

QString TooltipCreator::getInfoIconResourceName()
{
    return QLatin1String("mydata://info.png");
}

QIcon TooltipCreator::getInfoIcon()
{
    return QIcon::fromTheme(QLatin1String("folder-open")); //image-stack-open
}

QString TooltipCreator::tooltip(Parser* parser)
{
    if (!parser)
    {
        return QString();
    }

    QString tooltip;
    tooltip += QString::fromUtf8("<html><head><title></title></head>");
    tooltip += QString::fromUtf8("<body>");

    tooltip += tableStart();
    tooltip += createSection(i18n("Options"),   parser->options());
    tooltip += createSection(i18n("Modifiers"), parser->modifiers(), true);
    tooltip += tableEnd();

    if (!parser->modifiers().isEmpty())
    {
        tooltip += additionalInformation();
    }

    tooltip += QString::fromUtf8("</body>");
    tooltip += QString::fromUtf8("</html>");

    return tooltip;
}

QString TooltipCreator::tableStart(int widthPercentage)
{
    QString w = QString::number(widthPercentage) + QLatin1Char('%');
    return QString::fromUtf8("<table width=\"%1\" cellspacing=\"0\" cellpadding=\"0\" border=\"0\">").arg(w);
}

QString TooltipCreator::tableStart()
{
    return tableStart(100);
}

QString TooltipCreator::tableEnd()
{
    return QString::fromUtf8("</table>");
}

QString TooltipCreator::markOption(const QString& str)
{
    QString result = str;

    QRegExp optionsRegExp(QLatin1String("\\|\\|(.*)\\|\\|"));
    optionsRegExp.setMinimal(true);

    result.replace(optionsRegExp, QString::fromUtf8("<i><font color=\"%1\">\\1</font></i>")
                   .arg(qApp->palette().color(QPalette::Link).name()));
    return result;
}

QString TooltipCreator::createHeader(const QString& str)
{
    QString result;
    QString templateStr = QString::fromUtf8("<tr><td style=\"background-color: %1; padding:0.25em;\" colspan=\"2\">"
                                  "<nobr><font color=\"%2\"><center><b>%3"
                                  "</b></center></font></nobr></td></tr>")
                          .arg(qApp->palette().color(QPalette::Highlight).name())
                          .arg(qApp->palette().color(QPalette::HighlightedText).name());

    result += templateStr.arg(str);
    return result;
}

QString TooltipCreator::createEntries(const RulesList &data)
{
    QString result;

    foreach(Rule* const t, data)
    {
        foreach(Token* const token, t->tokens())
        {
            result += QString::fromUtf8("<tr>"
                              "<td style=\"background-color: %1;\">"
                              "<font color=\"%2\"><b>&nbsp;%3&nbsp;</b></font></td>"
                              "<td>&nbsp;%4&nbsp;</td></tr>")
                      .arg(qApp->palette().color(QPalette::Base).name())
                      .arg(qApp->palette().color(QPalette::Text).name())
                      .arg(markOption(token->id()))
                      .arg(markOption(token->description()));
        }
    }

    return result;
}

QString TooltipCreator::createSection(const QString& sectionName, const RulesList &data, bool lastSection)
{
    if (data.isEmpty())
    {
        return QString();
    }

    QString result;

    result += createHeader(sectionName);
    result += createEntries(data);

    if (!lastSection)
    {
        result += QString::fromUtf8("<tr></tr>");
    }

    return result;
}

} // namespace Digikam
