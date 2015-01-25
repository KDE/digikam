/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-09-14
 * Description : a parse results map for token management
 *
 * Copyright (C) 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
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

// Local includes

#include "digikam_debug.h"

namespace
{
static const quint8 INVALID_KEY_ID = -1;
}

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

bool ParseResults::hasKey(const ResultsKey& key)
{
    return keys().contains(key);
}

QList<ParseResults::ResultsValue> ParseResults::values() const
{
    return m_results.values();
}

QString ParseResults::result(const ResultsKey& key) const
{
    if (m_results.isEmpty())
    {
        return QString();
    }

    QString result = m_results.value(key).second;
    return result;
}

QString ParseResults::token(const ResultsKey& key) const
{
    if (m_results.isEmpty())
    {
        return QString();
    }

    QString token = m_results.value(key).first;
    return token;
}

int ParseResults::offset(const ResultsKey& key) const
{
    int pos    = key.first;
    int length = key.second;

    if (hasKeyAtPosition(pos))
    {
        return (pos + length);
    }
    else if (hasKeyAtApproximatePosition(pos))
    {
        ResultsKey key = keyAtApproximatePosition(pos);
        return ((key.first + key.second) - pos);
    }

    return INVALID_KEY_ID;
}

ParseResults::ResultsKey ParseResults::keyAtPosition(int pos) const
{
    foreach(const ResultsKey& key, m_results.keys())
    {
        if (pos == key.first)
        {
            return key;
        }
    }

    return createInvalidKey();
}

bool ParseResults::hasKeyAtPosition(int pos) const
{
    ResultsKey key = keyAtPosition(pos);
    return keyIsValid(key);
}

ParseResults::ResultsKey ParseResults::keyAtApproximatePosition(int pos) const
{
    foreach(const ResultsKey& key, m_results.keys())
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

bool ParseResults::hasKeyAtApproximatePosition(int pos) const
{
    ResultsKey key = keyAtApproximatePosition(pos);
    return keyIsValid(key);
}

void ParseResults::clear()
{
    m_results.clear();
}

void ParseResults::append(const ParseResults& results)
{
    m_results.unite(results.m_results);
}

bool ParseResults::isEmpty() const
{
    return m_results.isEmpty();
}

ParseResults::ResultsKey ParseResults::createInvalidKey() const
{
    return ResultsKey(INVALID_KEY_ID, INVALID_KEY_ID);
}

bool ParseResults::keyIsValid(const ResultsKey& key) const
{
    return (key.first != INVALID_KEY_ID && key.second != INVALID_KEY_ID);
}

QString ParseResults::replaceTokens(const QString& markedString) const
{
    QString result;

    for (int i = 0; i < markedString.count();)
    {
        if (hasKeyAtPosition(i))
        {
            ResultsKey key     = keyAtPosition(i);
            ResultsValue value = m_results.value(key);
            result.append(value.second);
            i += key.second;
        }
        else
        {
            result.append(markedString.at(i));
            ++i;
        }
    }

    return result;
}

void ParseResults::debug() const
{
    foreach(const ResultsKey& key, m_results.keys())
    {
        QString _token  = token(key);
        QString _result = result(key);

        qCDebug(DIGIKAM_GENERAL_LOG) << "(" << key.first << ":" << key.second << ") => "
                 << "(" << _token    << ":" << _result    << ")";
    }
}

} // namespace Digikam
