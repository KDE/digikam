/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-11
 * Description : the main parser object for the AdvancedRename utility
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

#include "parser.h"

// Qt includes

#include <QFileInfo>

namespace Digikam
{

class ParserPriv
{
public:

    ParserPriv() :
        counter(0)
    {}

    ParseSettings settings;
    OptionsList   options;
    int           counter;
};

// --------------------------------------------------------

Parser::Parser()
      : d(new ParserPriv)
{
    init();
}

Parser::~Parser()
{
    foreach (Option* option, d->options)
    {
        delete option;
    }
    d->options.clear();

    delete d;
}

void Parser::init(const ParseSettings& settings)
{
    d->settings = settings;
    d->counter  = 1;
}

void Parser::reset()
{
    init(ParseSettings());
    foreach (Option* option, d->options)
    {
        option->reset();

        foreach (Modifier* modifier, option->modifiers())
        {
            modifier->reset();
        }
    }
}

bool Parser::stringIsValid(const QString& str)
{
    QRegExp invalidString("^\\s*$");
    if (str.isEmpty() || invalidString.exactMatch(str))
    {
        return false;
    }
    return true;
}

OptionsList Parser::options() const
{
    return d->options;
}

ModifierList Parser::modifiers() const
{
    ModifierList modifierList;
    if (!d->options.isEmpty())
    {
        modifierList = d->options.first()->modifiers();
    }
    return modifierList;
}

void Parser::registerOption(Option* option)
{
    if (!option || !option->isValid())
    {
        return;
    }

    d->options.append(option);
}

ParseResults Parser::results(const QString& parseString, bool modify)
{
    ParseResults  results;
    ParseSettings settings;

    parseOperation(parseString, settings, results, modify);
    return results;
}

QString Parser::parse(const QString& parseString, ParseSettings& settings)
{
    ParseResults results;
    return parseOperation(parseString, settings, results);
}

QString Parser::parseOperation(const QString& parseString, ParseSettings& settings, ParseResults& results, bool modify)
{
    QFileInfo fi(settings.fileUrl.toLocalFile());

    if (!stringIsValid(parseString))
    {
        return fi.fileName();
    }

    settings.startIndex   = d->settings.startIndex;
    settings.currentIndex = d->counter++;

    foreach (Option* option, d->options)
    {
        option->parse(parseString, settings, modify);

        ParseResults r = option->results(modify);
        results.append(r);
    }

    QString parsed;
    if (modify)
    {
        parsed = results.replaceTokens(parseString);
    }

    QString newname = parsed;
    if (newname.isEmpty())
    {
        return fi.fileName();
    }

    if (settings.useOriginalFileExtension)
    {
        newname.append('.').append(fi.suffix());
    }

    return newname;
}

bool Parser::tokenAtPosition(TokenType type, const QString& parseString, int pos)
{
    int start;
    int length;
    return tokenAtPosition(type, parseString, pos, start, length);
}

bool Parser::tokenAtPosition(TokenType type, const QString& parseString, int pos, int& start, int& length)
{
    bool found = false;

    ParseResults r;

    switch (type)
    {
        case OptionToken:
            r = results(parseString, false);
            break;
        case OptionModifiersToken:
            r = results(parseString, true);
            break;
        default:
            break;
    }

    ParseResults::ResultsKey key = r.keyAtApproximatePosition(pos);
    start  = key.first;
    length = key.second;

    if ((pos >= start) && (pos <= start + length))
    {
        found = true;
    }
    return found;
}

}  // namespace Digikam
