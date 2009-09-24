/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-08
 * Description : an abstract subparser class
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

#ifndef SUBPARSER_H
#define SUBPARSER_H

// Local includes

#include "parseobject.h"
#include "parseresults.h"
#include "modifier.h"

namespace Digikam
{

class SubParserPriv;

class SubParser : public ParseObject
{
    Q_OBJECT

public:

    SubParser(const QString& name, const QString& description, const QIcon& icon = QIcon());
    virtual ~SubParser();

    ModifierList modifiers() const;

    ParseResults parseResults();
    ParseResults modifiedResults();

    /**
     * Use this method to define whether the parser should use modifiers or not
     * @param value boolean parameter to allow modifiers
     */
    void setUseModifiers(bool value);
    bool useModifiers() const;

public Q_SLOTS:

    virtual void parse(const QString& parseString, const ParseInformation& info = ParseInformation());

protected:

    virtual void parseOperation(const QString& parseString, const ParseInformation& info, ParseResults& results) = 0;

    /**
     * register a modifier to the parser class
     * @param modifier the modifier to add to the parser
     */
    void registerModifier(Modifier* modifier);

private:

    ParseResults applyModifiers(const QString& parseString, ParseResults& results);

private:

    SubParserPriv* const d;
};

typedef QList<SubParser*> SubParserList;

} // namespace Digikam

#endif /* SUBPARSER_H */
