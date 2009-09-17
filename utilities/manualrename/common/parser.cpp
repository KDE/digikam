/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-11
 * Description : the main parser object for manual rename
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

// Local includes

#include "lowercasemodifier.h"
#include "uppercasemodifier.h"
#include "firstletterofeachworduppercasemodifier.h"
#include "trimmedmodifier.h"

namespace Digikam
{

class ParserPriv
{
public:

    ParserPriv()
    {
    }

    SubParserList subparsers;
    ModifierList  modifiers;
};

// --------------------------------------------------------

Parser::Parser()
      : d(new ParserPriv)
{
    /**
     * Register all sub-parsers here (found in the directory 'utilities/manualrename/parsers').
     * This list will be used in here for the parse method and also in the ManualRenameWidget,
     * to create the buttons and menu entries as well as the tooltip.
     * The base parser class will not register sub-parsers, this should be done in the derived classes
     * like @see DefaultParser, to have individual parser classes.
     *
     */

    /*
     * MODIFIERS
     *
     * should be used in every parser, so registering them in the base parser is correct.
     */
    registerModifier(new LowerCaseModifier());
    registerModifier(new UpperCaseModifier());
    registerModifier(new FirstLetterEachWordUpperCaseModifier());
    registerModifier(new TrimmedModifier());
}

Parser::~Parser()
{
    foreach (SubParser* subparser, d->subparsers)
    {
        delete subparser;
    }

    foreach (Modifier* modifier, d->modifiers)
    {
        delete modifier;
    }

    d->subparsers.clear();
    d->modifiers.clear();
}

SubParserList Parser::subParsers() const
{
    return d->subparsers;
}

ModifierList Parser::modifiers() const
{
    return d->modifiers;
}

void Parser::registerSubParser(SubParser* parser)
{
    if (!parser)
        return;

    d->subparsers.append(parser);
}

void Parser::registerModifier(Modifier* modifier)
{
    if (!modifier)
        return;

    d->modifiers.append(modifier);
}

ParseResults Parser::parseResults(const QString& parseString)
{
    ParseResults results;
    ParseInformation info;

    parseOperation(parseString, info, results, false);
    return results;
}

QString Parser::parse(const QString& parseString, ParseInformation& info)
{
    ParseResults results;
    return parseOperation(parseString, info, results);
}

QString Parser::parseOperation(const QString& parseString, ParseInformation& info, ParseResults& results,
                               bool replace)
{
    if (!SubParser::stringIsValid(parseString))
    {
        QFileInfo fi(info.filePath);
        QString baseName = fi.baseName();
        return baseName;
    }

    if (!d->subparsers.isEmpty())
    {
        QStringList tokens;

        // parse and extract matching tokens
        foreach (SubParser* parser, d->subparsers)
        {
            parser->parse(parseString, info, results);
        }
    }

    QString parsed;
    if (replace)
    {
        applyModifiers(parseString, results);
        parsed = results.replaceTokens(parseString);
    }
    return parsed;
}

void Parser::applyModifiers(const QString& parseString, ParseResults& results)
{
    if (results.isEmpty())
        return;

    QString tmp = parseString;

    foreach (Modifier* modifier, d->modifiers)
    {
        int pos = 0;
        while (pos > -1)
        {
            pos = parseString.indexOf(modifier->id(), pos);
            if (pos > -1)
            {
                int start  = 0;
                int length = 0;

                if (tokenAtPosition(parseString, pos, start, length))
                {
                    QString token  = results.token(start, length);
                    QString result = results.result(start, length);
                    results.addEntry(start, token, modifier->modify(result));
                    results.addModifier(pos, modifier->id().count());
                }
                ++pos;
            }
        }
    }
}

bool Parser::tokenAtPosition(const QString& parseString, int pos)
{
    int start;
    int length;
    return tokenAtPosition(parseString, pos, start, length);
}

bool Parser::tokenAtPosition(const QString& parseString, int pos, int& start, int& length)
{
    bool found = false;

    ParseResults results         = parseResults(parseString);
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
