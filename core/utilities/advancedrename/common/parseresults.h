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

#ifndef PARSERESULTS_H
#define PARSERESULTS_H

// Qt includes

#include <QMap>
#include <QPair>
#include <QString>

namespace Digikam
{

class ParseResults
{
public:

    typedef QPair<int, int>                ResultsKey;
    typedef QPair<QString, QString>        ResultsValue;
    typedef QMap<ResultsKey, ResultsValue> ResultsMap;

public:

    ParseResults()  {};
    ~ParseResults() {};

    void addEntry(const ResultsKey& key, const ResultsValue& value);
    void deleteEntry(const ResultsKey& key);

    QList<ResultsKey>   keys()   const;
    QList<ResultsValue> values() const;

    bool       hasKey(const ResultsKey& key);

    QString    result(const ResultsKey& key) const;
    QString    token(const ResultsKey& key)  const;

    int        offset(const ResultsKey& key) const;

    ResultsKey keyAtPosition(int pos)    const;
    bool       hasKeyAtPosition(int pos) const;

    ResultsKey keyAtApproximatePosition(int pos)    const;
    bool       hasKeyAtApproximatePosition(int pos) const;

    bool       isEmpty() const;

    void       append(const ParseResults &results);
    void       clear();

    QString    replaceTokens(const QString& markedString) const;

    void       debug() const;

private:

    ResultsKey createInvalidKey() const;
    bool       keyIsValid(const ResultsKey& key) const;

    ResultsMap  m_results;
};

} // namespace Digikam


#endif /* PARSERESULTS_H */
