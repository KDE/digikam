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

#include "manualrenameparser.h"

// Qt includes

#include <QFileInfo>

// Libkexiv2 includes

#include <libkexiv2/version.h>

// Local includes

#include "cameranameparser.h"
#include "dateparser.h"
#include "directorynameparser.h"
#include "filenameparser.h"
#include "metadataparser.h"
#include "sequencenumberparser.h"

namespace Digikam
{
namespace ManualRename
{

ManualRenameParser::ManualRenameParser()
{
    /*
     * Register all sub-parsers here (found in the directory 'utilities/manualrename/parsers').
     * This list will be used in here for the parse method and also in the ManualRenameWidget,
     * to create the buttons and menu entries as well as the tooltip.
     */

    registerParser(new FilenameParser());
    registerParser(new DirectoryNameParser());
    registerParser(new SequenceNumberParser());
    registerParser(new CameraNameParser());
    registerParser(new DateParser());

#if KEXIV2_VERSION >= 0x010000
    registerParser(new MetadataParser());
#endif
}

ManualRenameParser::~ManualRenameParser()
{
    foreach (Parser* parser, m_parsers)
    {
        delete parser;
    }

    m_parsers.clear();
}

ParserList ManualRenameParser::parsers() const
{
    return m_parsers;
}

void ManualRenameParser::registerParser(Parser* parser)
{
    if (!parser)
        return;

    m_parsers.append(parser);
}

ManualRenameParser::TokenMap ManualRenameParser::tokenMap(const QString& parseString)
{
    TokenMap tokenMap;

    if (!Parser::stringIsValid(parseString))
        return tokenMap;

    QString parsed = parseString;
    if (!m_parsers.isEmpty())
    {
        QStringList tokens;
        ParseInformation info;

        // parse and extract matching tokens
        foreach (Parser* parser, m_parsers)
        {
            parser->parse(parsed, info);
            extractTokens(parsed, tokens);
        }
        replaceMatchingTokens(parsed, tokens, &tokenMap);
    }

    return tokenMap;
}

QString ManualRenameParser::parse(const QString& parseString, ParseInformation& info)
{
    if (!Parser::stringIsValid(parseString))
    {
        QFileInfo fi(info.filePath);
        QString baseName = fi.baseName();
        return baseName;
    }

    QString parsed = parseString;
    if (!m_parsers.isEmpty())
    {
        QStringList tokens;

        // parse and extract matching tokens
        foreach (Parser* parser, m_parsers)
        {
            parser->parse(parsed, info);
            extractTokens(parsed, tokens);
        }
        replaceMatchingTokens(parsed, tokens);
    }
    return parsed;
}

void ManualRenameParser::replaceMatchingTokens(QString& parseString, QStringList& tokens, TokenMap* map)
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

            if (map)
            {
                addTokenMapItem(relIndex, length, token, map);
            }
        }
    }
}

int ManualRenameParser::extractTokens(QString& parseString, QStringList& tokens)
{
    if (parseString.isEmpty())
        return 0;

    QRegExp regExp(Parser::resultsExtractor());
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

            if (result == Parser::emptyTokenMarker())
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

void ManualRenameParser::addTokenMapItem(int index, int length, const QString& value, TokenMap* map)
{
    if (!map)
        return;

    QString key = QString("%1:%2").arg(QString::number(index))
                                  .arg(QString::number(length));
    map->insert(key, value);
}

}  // namespace ManualRename
}  // namespace Digikam
