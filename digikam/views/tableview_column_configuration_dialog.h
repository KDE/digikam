/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-03-02
 * Description : Table view: Column configuration dialog
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

#ifndef TABLEVIEW_COLUMN_CONFIGURATION_DIALOG_H
#define TABLEVIEW_COLUMN_CONFIGURATION_DIALOG_H

// Qt includes

#include <QTreeView>
#include <QWidget>
#include <QItemDelegate>

// KDE includes

#include "kcategorizedsortfilterproxymodel.h"
#include "kdialog.h"

// local includes

/// @todo clean up includes and use forward-declarations where possible
#include "statesavingobject.h"
#include "digikam_export.h"
#include "imagealbummodel.h"
#include "thumbnailloadthread.h"
#include "imagefiltermodel.h"
#include "tableview_columnfactory.h"
#include "tableview_shared.h"

class KMenu;
class QContextMenuEvent;

namespace Digikam
{

class TableViewConfigurationDialog : public KDialog
{
    Q_OBJECT

public:

    explicit TableViewConfigurationDialog(TableViewShared* const sharedObject, const int columnIndex, QWidget* const parentWidget);
    virtual ~TableViewConfigurationDialog();

    TableViewColumnConfiguration getNewConfiguration() const;

private:

    class Private;
    const QScopedPointer<Private> d;
    TableViewShared* const s;
};

} /* namespace Digikam */

#endif // TABLEVIEW_COLUMN_CONFIGURATION_DIALOG_H
