/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-02-12
 * Description : Table view column helpers
 *
 * Copyright (C) 2013 by Michael G. Hansen <mike at mghansen dot de>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef TABLEVIEW_COLUMNFACTORY_H
#define TABLEVIEW_COLUMNFACTORY_H

// Qt includes

#include <QObject>
#include <QStringList>
#include <QModelIndex>

// KDE includes

// local includes

#include "imagefiltermodel.h"

namespace Digikam
{

class TableViewColumnDataSource
{
public:
    ImageFilterModel* sourceModel;
};

class TableViewColumn
{
protected:
    TableViewColumnDataSource* const dataSource;

public:

    explicit TableViewColumn(TableViewColumnDataSource* const pDataSource);
    virtual ~TableViewColumn();

    static QString getIdStatic();
    virtual QString getId();
    virtual QString getTitle();

    virtual QVariant data(const QModelIndex& sourceIndex, const int role);
};

class TableViewColumnFactory : public QObject
{
    Q_OBJECT

public:

    explicit TableViewColumnFactory(TableViewColumnDataSource* const dataSource, QObject* parent = 0);

    QStringList getColumnIds();
    TableViewColumn* getColumn(const QString columnId);

private:

    class Private;
    const QScopedPointer<Private> d;
};

} /* namespace Digikam */

#endif // TABLEVIEW_COLUMNFACTORY_H


