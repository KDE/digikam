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
#include "tableview_shared.h"

namespace Digikam
{

class TableViewColumnDescription
{
public:
    explicit TableViewColumnDescription()
      : columnId(),
        columnTitle()
    {
    }

    explicit TableViewColumnDescription(const QString& id, const QString title)
      : columnId(id),
        columnTitle(title)
    {
    }

    QString columnId;
    QString columnTitle;
};

class TableViewColumnDataSource
{
public:
    ImageFilterModel* sourceModel;
};

class TableViewColumnConfiguration
{
public:
    explicit TableViewColumnConfiguration(const QString& id = QString())
      : columnId(id)
    {
    }
    QString columnId;

};

class TableViewColumn
{
protected:
    TableViewShared* const s;
    TableViewColumnConfiguration configuration;

public:

    explicit TableViewColumn(
            TableViewShared* const tableViewShared,
            const TableViewColumnConfiguration& pConfiguration
        );
    virtual ~TableViewColumn();

    static TableViewColumnDescription getDescription();
    virtual QString getTitle();

    virtual QVariant data(const QModelIndex& sourceIndex, const int role);
    virtual bool paint(QPainter* const painter, const QStyleOptionViewItem& option, const QModelIndex& sourceIndex) const;
    virtual QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& sourceIndex) const;
};

class TableViewColumnFactory : public QObject
{
    Q_OBJECT

public:

    explicit TableViewColumnFactory(TableViewShared* const tableViewShared, QObject* parent = 0);

    QList<TableViewColumnDescription> getColumnDescriptionList();
    TableViewColumn* getColumn(const TableViewColumnConfiguration& columnConfiguration);

private:

    class Private;
    const QScopedPointer<Private> d;
    TableViewShared* const s;
};

} /* namespace Digikam */

Q_DECLARE_METATYPE(Digikam::TableViewColumnDescription)

#endif // TABLEVIEW_COLUMNFACTORY_H


