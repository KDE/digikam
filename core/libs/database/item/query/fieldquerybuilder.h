/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-03-22
 * Description : database SQL queries helper class
 *
 * Copyright (C) 2007-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2012-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_FIELD_QUERY_BUILDER_H
#define DIGIKAM_FIELD_QUERY_BUILDER_H

// Qt includes

#include <QList>
#include <QString>

// Local includes

#include "coredbsearchxml.h"

namespace Digikam
{

class ItemQueryPostHooks;

class Q_DECL_HIDDEN FieldQueryBuilder
{
public:

    FieldQueryBuilder(QString& sql,
                      SearchXmlCachingReader& reader,
                      QList<QVariant>* boundValues,
                      ItemQueryPostHooks* const hooks,
                      SearchXml::Relation relation);

public:

    QString&                sql;
    SearchXmlCachingReader& reader;
    QList<QVariant>*        boundValues;
    ItemQueryPostHooks*     hooks;
    SearchXml::Relation     relation;

public:

    QString prepareForLike(const QString& str) const;

    void addIntField(const QString& name);
    void addDoubleField(const QString& name);
    void addStringField(const QString& name);
    void addDateField(const QString& name);
    void addChoiceIntField(const QString& name);
    void addLongListField(const QString& name);
    void addIntBitmaskField(const QString& name);
    void addChoiceStringField(const QString& name);
    void addPosition();
    void addRectanglePositionSearch(double lon1, double lat1, double lon2, double lat2) const;
};

} // namespace Digikam

#endif // DIGIKAM_FIELD_QUERY_BUILDER_H
