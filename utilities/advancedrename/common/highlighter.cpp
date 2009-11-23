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
        int index = expression.indexIn(text);
        while (index >= 0)
        {
            int length = expression.matchedLength();
            setFormat(index, length, rule.format);

            // highlight parameters in options and modifiers
            if ((rule.type == OptionPattern)
                && expression.numCaptures() > 0 && !expression.cap(1).isEmpty())
            {
                QString fullmatched  = expression.cap(0);
                QString parameters   = expression.cap(1);
                if (parameters.startsWith(':'))
                {
                    parameters.remove(0, 1);
                    int pindex = fullmatched.indexOf(parameters);
                    while (pindex >= 0)
                    {
                        int plength = parameters.length();
                        setFormat(index + pindex, plength, parameterFormat);
                        pindex = fullmatched.indexOf(parameters, pindex + plength);
                    }
                }
            }

            // hightlight quoted text in options and modifiers
            if ((rule.type == OptionPattern || rule.type == ModifierPattern)
                && expression.numCaptures() > 0)
            {
                QRegExp quotationExp = quotationRule.pattern;
                QString fullmatched  = expression.cap(0);
                int qindex           = quotationExp.indexIn(fullmatched);
                while (qindex >= 0)
                {
                    int qlength = quotationExp.matchedLength();
                    setFormat(index + qindex, qlength, quotationFormat);
                    qindex = quotationExp.indexIn(fullmatched, qindex + qlength);
                }
            }
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

    optionFormat.setForeground((Qt::GlobalColor)OptionColor);

    foreach (Option* option, parser->options())
    {
        QRegExp r    = option->regExp();
        rule.type    = OptionPattern;
        rule.pattern = r;
        rule.format  = optionFormat;
        highlightingRules.append(rule);
    }

    // --------------------------------------------------------

    modifierFormat.setForeground((Qt::GlobalColor)ModifierColor);

    if (!parser->options().isEmpty())
    {
        Option* option = parser->options().first();
        foreach (Modifier* modifier, option->modifiers())
        {
            QRegExp r    = modifier->regExp();
            rule.type    = ModifierPattern;
            rule.pattern = r;
            rule.format  = modifierFormat;
            highlightingRules.append(rule);
        }
    }

    // --------------------------------------------------------

    quotationFormat.setForeground((Qt::GlobalColor)QuotedTextColor);
    quotationFormat.setFontItalic(true);
    quotationRule.pattern = QRegExp("\".*\"");
    quotationRule.pattern.setMinimal(true);
    quotationRule.format = quotationFormat;
    quotationRule.type   = QuotedTextPattern;

    // --------------------------------------------------------

    parameterFormat.setForeground((Qt::GlobalColor)ParameterColor);
    parameterFormat.setFontItalic(true);
}

} // namespace Digikam
