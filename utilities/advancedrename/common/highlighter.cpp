/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-19
 * Description : syntax highlighter for AdvancedRename utility
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

#include "highlighter.h"

// Qt includes

#include <QFileInfo>
#include <QTextEdit>

// Local includes

#include "parser.h"

namespace Digikam
{

Highlighter::Highlighter(QTextEdit* parent, Parser* parser)
           : QSyntaxHighlighter(parent)

{
    setupHighlightingGrammar(parser);
}

Highlighter::~Highlighter()
{
}

void Highlighter::highlightBlock(const QString& text)
{
    foreach (const HighlightingRule &rule, highlightingRules)
    {
        QRegExp expression(rule.pattern);
        expression.setMinimal(true);
        int index = expression.indexIn(text);
        while (index >= 0)
        {
            int length = expression.matchedLength();
            setFormat(index, length, rule.format);
            index = expression.indexIn(text, index + length);
        }
    }
}

void Highlighter::setupHighlightingGrammar(Parser* parser)
{
    if (!parser)
    {
        return;
    }

    HighlightingRule rule;

    // --------------------------------------------------------

    optionFormat.setForeground((Qt::GlobalColor)OptionColorBackground);
//    optionFormat.setFontWeight(QFont::Bold);

    foreach (Option* option, parser->options())
    {
        QRegExp r = option->regExp();
        r.setMinimal(true);
        rule.pattern = r;
        rule.format  = optionFormat;
        highlightingRules.append(rule);
    }

    // --------------------------------------------------------

    modifierFormat.setForeground((Qt::GlobalColor)ModifierColorBackground);
//    modifierFormat.setFontWeight(QFont::Bold);

    if (!parser->options().isEmpty())
    {
        Option* option = parser->options().first();
        foreach (Modifier* modifier, option->modifiers())
        {
            QRegExp r = modifier->regExp();
            r.setMinimal(true);
            rule.pattern = r;
            rule.format  = modifierFormat;
            highlightingRules.append(rule);
        }
    }

    // --------------------------------------------------------

    quotationFormat.setForeground((Qt::GlobalColor)QuotedTextColor);
    quotationFormat.setFontItalic(true);
    rule.pattern = QRegExp("\".*\"");
    rule.pattern.setMinimal(true);
    rule.format = quotationFormat;
    highlightingRules.append(rule);
}

} // namespace Digikam
