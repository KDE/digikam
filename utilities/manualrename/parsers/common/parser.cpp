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

namespace Digikam
{

Parser::Parser()
{
    /*
     * Register all sub-parsers here (found in the directory 'utilities/manualrename/parsers').
     * This list will be used in here for the parse method and also in the ManualRenameWidget,
     * to create the buttons and menu entries as well as the tooltip.
     */
}

Parser::~Parser()
{
    foreach (SubParser* subparser, m_subparsers)
    {
        delete subparser;
    }

    m_subparsers.clear();
}

SubParserList Parser::subParsers() const
{
    return m_subparsers;
}

void Parser::registerSubParser(SubParser* parser)
{
    if (!parser)
        return;

    m_subparsers.append(parser);
}

Parser::ParseResultsMap Parser::parseResultsMap(const QString& parseString)
{
    ParseResultsMap resultsMap;

    if (!SubParser::stringIsValid(parseString))
        return resultsMap;

    QString parsed = parseString;
    if (!m_subparsers.isEmpty())
    {
        QStringList tokens;
        ParseInformation info;

        // parse and extract matching tokens
        foreach (SubParser* subparser, m_subparsers)
        {
            subparser->parse(parsed, info);
            extractTokens(parsed, tokens);
        }
        replaceMatchingTokens(parsed, tokens, &resultsMap);
    }

    return resultsMap;
}

QString Parser::parse(const QString& parseString, ParseInformation& info)
{
    if (!SubParser::stringIsValid(parseString))
    {
        QFileInfo fi(info.filePath);
        QString baseName = fi.baseName();
        return baseName;
    }

    QString parsed = parseString;
    if (!m_subparsers.isEmpty())
    {
        QStringList tokens;

        // parse and extract matching tokens
        foreach (SubParser* parser, m_subparsers)
        {
            parser->parse(parsed, info);
            extractTokens(parsed, tokens);
        }
        replaceMatchingTokens(parsed, tokens);
    }
    return parsed;
}

void Parser::replaceMatchingTokens(QString& parseString, QStringList& tokens, ParseResultsMap* map)
{
    QRegExp regExp("index:(\\d+):(\\d+)");

    bool firstRun = true;
    int index     = 0;
    int length    = 0;
    int diff      = 0;
    int pos       = 0;
    int relIndex  = 0;

    while (pos > -1)
    {
        pos = regExp.indexIn(parseString, pos);
        if (pos > -1)
        {
            index         = regExp.cap(1).toInt();
            length        = regExp.cap(2).toInt();
            QString token = tokens.at(index);
            parseString.replace(pos, regExp.matchedLength(), token);

            if (map)
            {
                if (firstRun)
                {
                    firstRun = false;
                    relIndex = pos;
                }
                else
                {
                    relIndex = qAbs<int>(pos - diff);
                }
                diff += token.count() - length;
                addTokenMapItem(relIndex, length, token, map);
            }
        }
    }
}

int Parser::extractTokens(QString& parseString, QStringList& tokens)
{
    if (parseString.isEmpty())
        return 0;

    QRegExp regExp(SubParser::resultsExtractor());
    regExp.setMinimal(true);
    int pos       = 0;
    int extracted = 0;

    while (pos > -1)
    {
        pos = regExp.indexIn(parseString, pos);
        if (pos > -1)
        {
            int length     = regExp.cap(1).toInt();
            QString result = regExp.cap(2);

            if (result == SubParser::emptyTokenMarker())
                tokens << QString();
            else
                tokens << result;

            ++extracted;
            int index = tokens.count() - 1;
            parseString.replace(pos, regExp.matchedLength(),
                    QString("index:%1:%2").arg(QString::number(index))
                    .arg(QString::number(length)));
        }
    }
    return extracted;
}

void Parser::addTokenMapItem(int index, int length, const QString& value, ParseResultsMap* map)
{
    if (!map)
        return;

    QString key = QString("%1:%2").arg(QString::number(index))
                                  .arg(QString::number(length));
    map->insert(key, value);
}

bool Parser::tokenAtPosition(const QString& parseString, int pos)
{
    int start;
    int length;
    return tokenAtPosition(parseString, pos, start, length);
}

bool Parser::tokenAtPosition(const QString& parseString, int pos, int& start, int& length)
{
    ParseResultsMap map                = parseResultsMap(parseString);
    ParseResultsMap::const_iterator it = 0;

    bool found = false;

    for (it = map.constBegin(); it != map.constEnd(); ++it)
    {
        QString keys        = it.key();
        QStringList keylist = keys.split(':', QString::SkipEmptyParts);

        if (!keylist.count() == 2)
            continue;

        length = keylist.last().toInt();
        start  = keylist.first().toInt();

        if ((pos >= start) && (pos <= start + length))
        {
            found = true;
            break;
        }
    }
    return found;
}

}  // namespace Digikam
