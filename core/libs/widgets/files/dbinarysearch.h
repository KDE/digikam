/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-01-05
 * Description : a widget to find missing binaries.
 *
 * Copyright (C) 2012-2012 by Benjamin Girault <benjamin dot girault at gmail dot com>
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

#ifndef DBINARY_SEARCH_H
#define DBINARY_SEARCH_H

// Qt includes

#include <QString>
#include <QTreeWidget>

// Local includes

#include "digikam_export.h"
#include "dbinaryiface.h"

namespace Digikam
{

/**
 * This class has nothing to do with a binary search, it is a widget to search for binaries.
 */
class DIGIKAM_EXPORT DBinarySearch : public QTreeWidget
{
    Q_OBJECT

public:

    enum ColumnType
    {
        Status = 0,
        Binary,
        Version,
        Button,
        Link
    };

public:

    explicit DBinarySearch(QWidget* const parent);
    virtual ~DBinarySearch();

    void addBinary(DBinaryIface& binary);
    void addDirectory(const QString& dir);
    bool allBinariesFound();

public Q_SLOTS:

    void slotAreBinariesFound();

Q_SIGNALS:

    void signalBinariesFound(bool);
    void signalAddDirectory(const QString& dir);
    void signalAddPossibleDirectory(const QString& dir);

private:

    struct Private;
    Private* const d;
};

} // namespace Digikam

#endif // DBINARY_SEARCH_H
