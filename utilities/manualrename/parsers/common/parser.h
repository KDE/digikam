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

#ifndef PARSER_H
#define PARSER_H

// Qt includes

#include <QList>
#include <QMap>
#include <QString>

// Local includes

#include "parseinformation.h"
#include "subparser.h"

class QStringList;

namespace Digikam
{

typedef QMap<QString, QString> ParseResultsMap;

class SubParser;
class Parser
{

public:

    Parser();
    virtual ~Parser();

    QString         parse(const QString& parseString, ParseInformation& info);
    SubParserList   subParsers() const;
    ParseResultsMap parseResultsMap(const QString& parseString);

protected:

    int  extractTokens(QString& parseString, QStringList& tokens);
    void replaceMatchingTokens(QString& parseString, QStringList& tokens, ParseResultsMap* map = 0);
    void addTokenMapItem(int index, int length, const QString& value, ParseResultsMap* map);
    void registerSubParser(SubParser* parser);

protected:

    QString       m_parseString;
    SubParserList m_subparsers;
};

}  // namespace Digikam


#endif /* PARSER_H */
