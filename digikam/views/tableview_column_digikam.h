/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-02-28
 * Description : Table view column helpers: Digikam properties
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

#ifndef TABLEVIEW_COLUMN_DIGIKAM_H
#define TABLEVIEW_COLUMN_DIGIKAM_H

// Qt includes

#include <QObject>
#include <QPainter>
#include <QStringList>

// KDE includes

#include <kglobal.h>
#include <klocale.h>

// local includes

#include "tableview_columnfactory.h"

namespace Digikam
{

namespace TableViewColumns
{

class ColumnDigikamProperties : public TableViewColumn
{
    Q_OBJECT

public:

    enum SubColumn
    {
        SubColumnRating = 0,
        SubColumnPickLabel = 1,
        SubColumnColorLabel = 2
    };

private:
    SubColumn subColumn;

public:

    explicit ColumnDigikamProperties(
            TableViewShared* const tableViewShared,
            const TableViewColumnConfiguration& pConfiguration,
            QObject* const parent = 0
        );
    virtual ~ColumnDigikamProperties();

    static TableViewColumnDescription getDescription();
    static QStringList getSubColumns();
    virtual QString getTitle() const;

    virtual ColumnFlags getColumnFlags() const;

    virtual QVariant data(const QModelIndex& sourceIndex, const int role) const;

    virtual ColumnCompareResult compare(const QModelIndex& sourceA, const QModelIndex& sourceB) const;
};

} /* namespace TableViewColumns */

} /* namespace Digikam */

#endif // TABLEVIEW_COLUMN_DIGIKAM_H

