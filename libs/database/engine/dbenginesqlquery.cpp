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

#include "dbenginesqlquery.h"

namespace Digikam
{

DbEngineSqlQuery::DbEngineSqlQuery(const QSqlQuery& other)
    : QSqlQuery(other)
{
}

DbEngineSqlQuery::DbEngineSqlQuery(const QSqlDatabase& db)
    : QSqlQuery(db)
{
}

DbEngineSqlQuery::~DbEngineSqlQuery()
{
}

DbEngineSqlQuery& DbEngineSqlQuery::operator=(const DbEngineSqlQuery& other)
{
    QSqlQuery::operator=(other);
    m_query = other.m_query;
    return *this;
}

bool DbEngineSqlQuery::prepare(const QString& query)
{
    bool result = QSqlQuery::prepare(query);
    m_query     = query;
    return result;
}

QString DbEngineSqlQuery::lastQuery() const
{
    return m_query;
}

} // namespace Digikam
