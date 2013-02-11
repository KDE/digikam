/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-02-11
 * Description : Table view
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

#ifndef TABLEVIEW_H
#define TABLEVIEW_H

// Qt includes

#include <QWidget>

// KDE includes

#include "kcategorizedsortfilterproxymodel.h"

// local includes

/// @todo clean up includes and use forward-declarations where possible
#include "statesavingobject.h"
#include "digikam_export.h"
#include "imagealbummodel.h"
#include "thumbnailloadthread.h"
#include "imagefiltermodel.h"

namespace Digikam
{

class TableView : public QWidget, public StateSavingObject
{
    Q_OBJECT

public:

    TableView(
            QItemSelectionModel* const selectionModel,
            KCategorizedSortFilterProxyModel* const imageFilterModel,
            QWidget* const parent
        );
    virtual ~TableView();

protected:

    void doLoadState();
    void doSaveState();

private:

    class Private;
    const QScopedPointer<Private> d;
};

} /* namespace Digikam */

#endif // TABLEVIEW_H
