/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-06-18
 * Description :database album interface.
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef SQLQUERY_H
#define SQLQUERY_H

// Qt includes

#include <QSqlQuery>
#include <QtCore/QString>

// KDE includes

// Local includes

namespace Digikam
{

class SqlQuery : public QSqlQuery
{
public:

    SqlQuery() : QSqlQuery()
    {
    }

    SqlQuery(const QSqlQuery &other) : QSqlQuery(other)
    {
    }

    SqlQuery(QSqlDatabase db) : QSqlQuery(db)
    {
    }

    virtual SqlQuery& operator=(const SqlQuery& other)
    {
        QSqlQuery::operator=(other);
        m_query = other.m_query;
        return *this;
    }

    virtual bool prepare(const QString& query)
    {
        bool result = QSqlQuery::prepare(query);
        m_query = query;
        return result;
    }

    virtual QString lastQuery() const
    {
        return m_query;
    }

    virtual ~SqlQuery()
    {
    }

private:

    QString m_query;

};

}  // namespace Digikam

#endif /* SQLQUERY_H */
