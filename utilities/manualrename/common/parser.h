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
#include "parseresults.h"
#include "subparser.h"
#include "modifier.h"

class QStringList;

namespace Digikam
{

class SubParser;
class Modifier;
class ParserPriv;

class Parser
{

public:

    Parser();
    virtual ~Parser();

    QString       parse(const QString& parseString, ParseInformation& info);

    SubParserList subParsers() const;
    ModifierList  modifiers()  const;

    bool          tokenAtPosition(const QString& parseString, int pos);
    bool          tokenAtPosition(const QString& parseString, int pos, int& start, int& length);

protected:

    void registerSubParser(SubParser* parser);
    void registerModifier(Modifier* modifier);

private:

    ParseResults parseResults(const QString& parseString);
    void         applyModifiers(const QString& parseString, ParseResults& results);
    QString      parseOperation(const QString& parseString, ParseInformation& info, ParseResults& results,
                                bool replace = true);

private:

    ParserPriv* const d;
};

}  // namespace Digikam


#endif /* PARSER_H */
