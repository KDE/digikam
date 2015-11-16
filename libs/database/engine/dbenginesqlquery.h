/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-09-27
 * Description : Databse engine SQL query
 *
 * Copyright (C) 2009-2010 by Holger Foerster <Hamsi2k at freenet dot de>
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

#ifndef DATABASE_ENGINE_SQL_QUERY_H
#define DATABASE_ENGINE_SQL_QUERY_H

// Qt includes

#include <QMetaType>
#include <QString>
#include <QSqlQuery>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT DbEngineSqlQuery : public QSqlQuery
{

public:

    explicit DbEngineSqlQuery(const QSqlQuery& other);
    explicit DbEngineSqlQuery(const QSqlDatabase& db);
    virtual ~DbEngineSqlQuery();

    virtual DbEngineSqlQuery& operator=(const DbEngineSqlQuery& other);
    virtual bool prepare(const QString& query);
    virtual QString lastQuery() const;

private:

    QString m_query;
};

}  // namespace Digikam

#endif /* DATABASE_ENGINE_SQL_QUERY_H */
