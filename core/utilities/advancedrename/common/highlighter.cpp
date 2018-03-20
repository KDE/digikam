/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-19
 * Description : syntax highlighter for AdvancedRename utility
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

#include "highlighter.h"

// Qt includes

#include <QTextDocument>

// Local includes

#include "parser.h"

namespace Digikam
{

Highlighter::Highlighter(QTextDocument *document, Parser* _parser)
    : QSyntaxHighlighter(document), parser(_parser)

{
    setupHighlightingGrammar();
}

Highlighter::~Highlighter()
{
}

void Highlighter::highlightBlock(const QString& text)
{
    foreach(const HighlightingRule& rule, highlightingRules)
    {
        QRegExp expression(rule.pattern);
        int index = expression.indexIn(text);

        while (index >= 0)
        {
            int length = expression.matchedLength();
            setFormat(index, length, rule.format);

            switch (rule.type)
            {
                case OptionPattern:
                case ModifierPattern:
                {
                    // highlight parameters in options and modifiers
                    if (expression.captureCount() > 0 && !expression.cap(1).isEmpty())
                    {
                        QString fullmatched  = expression.cap(0);
                        QString parameters   = expression.cap(1);

                        if (parameters.startsWith(QLatin1Char(':')))
                        {
                            parameters.remove(0, 1);

                            if (!parameters.isEmpty())
                            {
                                int pindex = fullmatched.indexOf(parameters);

                                while (pindex >= 0)
                                {
                                    int plength = parameters.length();
                                    setFormat(index + pindex, plength, parameterFormat);
                                    pindex = fullmatched.indexOf(parameters, pindex + plength);
                                }
                            }
                        }
                    }

                    break;
                }

                default:
                    break;
            }

            index = expression.indexIn(text, index + length);
        }
    }

    // mark invalid modifiers in the parse string
    ParseSettings settings;
    settings.parseString = text;
    ParseResults invalid = parser->invalidModifiers(settings);
    foreach(const ParseResults::ResultsKey& key, invalid.keys())
    {
        setFormat(key.first, key.second, errorFormat);
    }

    // highlight quoted text in options and modifiers
    {
        QRegExp expression(quotationRule.pattern);
        int index = expression.indexIn(text);

        while (index >= 0)
        {
            QString fullmatched  = expression.cap(0);
            int qlength = expression.matchedLength();
            setFormat(index, qlength, quotationFormat);
            index = expression.indexIn(text, index + qlength);
        }
    }
}

void Highlighter::setupHighlightingGrammar()
{
    if (!parser)
    {
        return;
    }

    HighlightingRule rule;

    // --------------------------------------------------------

    optionFormat.setForeground(Qt::red);

    foreach(Rule* option, parser->options())
    {
        QRegExp r    = option->regExp();
        rule.type    = OptionPattern;
        rule.pattern = r;
        rule.format  = optionFormat;
        highlightingRules.append(rule);
    }

    // --------------------------------------------------------

    modifierFormat.setForeground(Qt::darkGreen);

    foreach(Rule* modifier, parser->modifiers())
    {
        QRegExp r    = modifier->regExp();
        rule.type    = ModifierPattern;
        rule.pattern = r;
        rule.format  = modifierFormat;
        highlightingRules.append(rule);
    }

    // --------------------------------------------------------

    quotationFormat.setForeground(QColor("#5050ff")); // light blue
    quotationFormat.setFontItalic(true);
    quotationRule.pattern = QRegExp(QLatin1String("\".*\""));
    quotationRule.pattern.setMinimal(true);
    quotationRule.format = quotationFormat;
    quotationRule.type   = QuotedTextPattern;

    // --------------------------------------------------------

    parameterFormat.setForeground(Qt::darkYellow);
    parameterFormat.setFontItalic(true);

    // --------------------------------------------------------

    errorFormat.setForeground(Qt::white);
    errorFormat.setBackground(Qt::red);
}

} // namespace Digikam
