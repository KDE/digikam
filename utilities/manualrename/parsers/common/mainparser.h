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

#ifndef MAINPARSER_H
#define MAINPARSER_H

// Qt includes

#include <QList>
#include <QMap>
#include <QString>

// Local includes

#include "parser.h"

class QStringList;

namespace Digikam
{
namespace ManualRename
{

class Parser;
class ParseInformation;

class MainParser
{

public:

    typedef QMap<QString, QString> TokenMap;

public:

    MainParser();
    virtual ~MainParser();

    QString    parse(const QString& parseString, ParseInformation& info);
    ParserList parsers() const;
    TokenMap   tokenMap(const QString& parseString);

protected:

    int  extractTokens(QString& parseString, QStringList& tokens);
    void replaceMatchingTokens(QString& parseString, QStringList& tokens, TokenMap* map = 0);
    void addTokenMapItem(int index, int length, const QString& value, TokenMap* map);
    void registerParser(Parser* parser);

protected:

    QString    m_parseString;
    ParserList m_parsers;
};

}  // namespace ManualRename
}  // namespace Digikam


#endif /* MAINPARSER_H */
