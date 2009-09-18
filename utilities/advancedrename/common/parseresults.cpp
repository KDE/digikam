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

#include "parseresults.h"

// KDE includes

#include <kdebug.h>

namespace Digikam
{

void ParseResults::addEntry(const ResultsKey& key, const ResultsValue& value)
{
    m_results.insert(key, value);
}

void ParseResults::deleteEntry(const ResultsKey& key)
{
    m_results.remove(key);
}

QList<ParseResults::ResultsKey> ParseResults::keys() const
{
    return m_results.keys();
}

QList<ParseResults::ResultsValue> ParseResults::values() const
{
    return m_results.values();
}

QString ParseResults::result(const ResultsKey& key)
{
    if (m_results.isEmpty())
        return QString();

    QString result = m_results.value(key).second;
    return result;
}

QString ParseResults::token(const ResultsKey& key)
{
    if (m_results.isEmpty())
        return QString();

    QString token = m_results.value(key).first;
    return token;
}

int ParseResults::offset(const ResultsKey& key)
{
    int pos    = key.first;
    int length = key.second;

    if (hasKeyAtPosition(pos))
    {
        return (pos+length);
    }
    else if (hasKeyAtApproximatePosition(pos))
    {
        ResultsKey key = keyAtApproximatePosition(pos);
        return ((key.first+key.second)-pos);
    }
    return -1;
}

ParseResults::ResultsKey ParseResults::keyAtPosition(int pos)
{
    foreach (const ResultsKey& key, m_results.keys())
    {
        if (pos == key.first)
            return key;
    }

    return createInvalidKey();
}

bool ParseResults::hasKeyAtPosition(int pos)
{
    ResultsKey key = keyAtPosition(pos);
    if (keyIsValid(key))
        return true;
    return false;
}

ParseResults::ResultsKey ParseResults::keyAtApproximatePosition(int pos)
{
    foreach (const ResultsKey& key, m_results.keys())
    {
        int start  = key.first;
        int length = key.second;
        if ((pos >= start) && (pos <= start + length))
        {
            return key;
        }
    }

    return createInvalidKey();
}

bool ParseResults::hasKeyAtApproximatePosition(int pos)
{
    ResultsKey key = keyAtApproximatePosition(pos);
    if (keyIsValid(key))
        return true;
    return false;
}

void ParseResults::clear()
{
    m_results.clear();
}

void ParseResults::append(ParseResults& results)
{
    m_results.unite(results.m_results);
}

bool ParseResults::isEmpty()
{
    return m_results.isEmpty();
}

ParseResults::ResultsKey ParseResults::createInvalidKey()
{
    return ResultsKey(-1, -1);
}

bool ParseResults::keyIsValid(const ResultsKey& key)
{
    if (key.first == -1 || key.second == -1)
        return false;
    return true;
}

QString ParseResults::replaceTokens(const QString& markedString)
{
    QString tmp;

    for (int i = 0; i < markedString.count();)
    {
        if (hasKeyAtPosition(i))
        {
            ResultsKey key     = keyAtPosition(i);
            ResultsValue value = m_results.value(key);
            tmp.append(value.second);
            i += key.second;
        }
        else
        {
            tmp.append(markedString[i]);
            ++i;
        }
    }
    return tmp;
}

void ParseResults::debug()
{
    foreach (const ResultsKey& key, m_results.keys())
    {
        QString t = token(key);
        QString r = result(key);

        kDebug(50003) << "(" << key.first << ":" << key.second << ") => "
                      << "(" << t         << ":" << r          << ")";
    }
}

} // namespace Digikam
