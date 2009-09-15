/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-09-14
 * Description : a parse results map for token management
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

#ifndef PARSERESULTSMAP_H
#define PARSERESULTSMAP_H

// Qt includes

#include <QPair>
#include <QMap>
#include <QSet>
#include <QString>

namespace Digikam
{

class ParseResultsMap
{
public:

    typedef QPair<int, int>         Key;
    typedef QPair<QString, QString> Value;
    typedef QMap<Key, Value>        Map;
    typedef QSet<int>               ModifierIDs;

public:

    ParseResultsMap();
    ~ParseResultsMap();

    void addEntry(int pos, const QString& token, const QString& result);
    void addModifier(int pos);

    QString result(int pos, int length);
    QString token(int pos, int length);


    Key  keyAtPosition(int pos);
    bool isKeyAtPosition(int pos);
    Key  keyAtApproximatePosition(int pos);
    bool isKeyAtApproximatePosition(int pos);

    bool isModifier(int pos);

    bool isEmpty();
    void clear();

    QString replaceTokens(const QString& markedString);

private:

    Key     createInvalidKey();
    bool    keyIsValid(const Key& key);
    QString keyString(const Key& key);

private:

    Map         m_map;
    ModifierIDs m_modifiers;
};

} // namespace Digikam


#endif /* PARSERESULTSMAP_H */
