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

namespace Digikam
{

void ParseResults::addEntry(int pos, const QString& token, const QString& result)
{
    if (token.isEmpty())
        return;

    ResultsKey key(pos, token.count());
    ResultsValue value(token, result);
    m_results.insert(key, value);
}

void ParseResults::addModifier(int pos)
{
    m_modifiers.insert(pos);
}

QString ParseResults::result(int pos, int length)
{
    if (m_results.isEmpty())
        return QString();

    ResultsKey key(pos, length);
    QString result = m_results.value(key).second;
    return result;
}

QString ParseResults::token(int pos, int length)
{
    if (m_results.isEmpty())
        return QString();

    ResultsKey key(pos, length);
    QString token = m_results.value(key).first;
    return token;
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

bool ParseResults::isKeyAtPosition(int pos)
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

bool ParseResults::isKeyAtApproximatePosition(int pos)
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
        if (isKeyAtPosition(i))
        {
            ResultsKey key     = keyAtPosition(i);
            ResultsValue value = m_results.value(key);
            tmp.append(value.second);
            i += key.second;
        }
        else if (isModifier(i))
        {
            ++i;
        }
        else
        {
            tmp.append(markedString[i]);
            ++i;
        }
    }
    return tmp;
}

bool ParseResults::isModifier(int pos)
{
    return m_modifiers.contains(pos);
}

} // namespace Digikam
