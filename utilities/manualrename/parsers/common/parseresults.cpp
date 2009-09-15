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

    Key key(pos, token.count());
    Value value(token, result);
    m_map.insert(key, value);
}

void ParseResults::addModifier(int pos)
{
    m_modifiers.insert(pos);
}

QString ParseResults::result(int pos, int length)
{
    if (m_map.isEmpty())
        return QString();

    Key key(pos, length);
    QString result = m_map.value(key).second;
    return result;
}

QString ParseResults::token(int pos, int length)
{
    if (m_map.isEmpty())
        return QString();

    Key key(pos, length);
    QString token = m_map.value(key).first;
    return token;
}

ParseResults::Key ParseResults::keyAtPosition(int pos)
{
    foreach (const Key& key, m_map.keys())
    {
        if (pos == key.first)
            return key;
    }

    return createInvalidKey();
}

bool ParseResults::isKeyAtPosition(int pos)
{
    Key key = keyAtPosition(pos);
    if (keyIsValid(key))
        return true;
    return false;
}

ParseResults::Key ParseResults::keyAtApproximatePosition(int pos)
{
    foreach (const Key& key, m_map.keys())
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
    Key key = keyAtApproximatePosition(pos);
    if (keyIsValid(key))
        return true;
    return false;
}

void ParseResults::clear()
{
    m_map.clear();
}

bool ParseResults::isEmpty()
{
    return m_map.isEmpty();
}

ParseResults::Key ParseResults::createInvalidKey()
{
    return Key(-1, -1);
}

bool ParseResults::keyIsValid(const Key& key)
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
            Key key     = keyAtPosition(i);
            Value value = m_map.value(key);
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

QString ParseResults::keyString(const Key& key)
{
    QString marker = QString("[map:%1:%2]")
                        .arg(QString::number(key.first))
                        .arg(QString::number(key.second));
    return marker;
}

} // namespace Digikam
