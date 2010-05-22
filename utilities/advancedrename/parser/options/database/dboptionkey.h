/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-05-19
 * Description : a db option key
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

#ifndef DBOPTIONKEY_H
#define DBOPTIONKEY_H

// Qt includes

#include <QMap>

// local includes

#include "parsesettings.h"

namespace Digikam
{

typedef QMap<QString, QString> DbKeyIdsMap;

class DbOptionKey
{
public:

    DbOptionKey();
    virtual ~DbOptionKey();

    QString getValue(const QString& key, ParseSettings& settings);
    DbKeyIdsMap ids() const;

protected:

    virtual QString getDbValue(const QString& key, ParseSettings& settings) = 0;
    void addKey(const QString& key, const QString& description);

private:

    DbKeyIdsMap m_keywords;
};

} // namespace Digikam

#endif /* DBOPTIONKEY_H */
