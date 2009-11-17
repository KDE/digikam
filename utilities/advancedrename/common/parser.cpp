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

    ParserPriv() {}
    OptionsList options;
};

// --------------------------------------------------------

Parser::Parser()
      : d(new ParserPriv)
{
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

void Parser::registerOption(Option* parser)
{
    if (!parser)
    {
        return;
    }

    if (!parser->isValid())
    {
        return;
    }

    d->options.append(parser);
}

ParseResults Parser::parseResults(const QString& parseString)
{
    ParseResults results;
    ParseInformation info;

    parseOperation(parseString, info, results, false);
    return results;
}

ParseResults Parser::modifierResults(const QString& parseString)
{
    ParseResults results;
    ParseInformation info;

    parseOperation(parseString, info, results, true);
    return results;
}

QString Parser::parse(const QString& parseString, ParseInformation& info)
{
    ParseResults results;
    return parseOperation(parseString, info, results);
}

QString Parser::parseOperation(const QString& parseString, ParseInformation& info, ParseResults& results,
                               bool modify)
{
    QFileInfo fi(info.filePath);

    if (!stringIsValid(parseString))
    {
        return fi.fileName();
    }

    if (!d->options.isEmpty())
    {
        foreach (Option* option, d->options)
        {
            option->parse(parseString, info);

            ParseResults r;
            if (modify)
            {
                r = option->modifiedResults();
            }
            else
            {
                r = option->parseResults();
            }

            results.append(r);
        }
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

    if (info.useFileExtension)
    {
        newname.append('.').append(fi.suffix());
    }

    return newname;
}

bool Parser::tokenAtPosition(Type type, const QString& parseString, int pos)
{
    int start;
    int length;
    return tokenAtPosition(type, parseString, pos, start, length);
}

bool Parser::tokenAtPosition(Type type, const QString& parseString, int pos, int& start, int& length)
{
    bool found = false;

    ParseResults results;

    switch (type)
    {
        case Token:
            results = parseResults(parseString);
            break;
        case TokenAndModifiers:
            results = modifierResults(parseString);
            break;
        default:
            break;
    }

    ParseResults::ResultsKey key = results.keyAtApproximatePosition(pos);
    start  = key.first;
    length = key.second;

    if ((pos >= start) && (pos <= start + length))
    {
        found = true;
    }
    return found;
}

}  // namespace Digikam
