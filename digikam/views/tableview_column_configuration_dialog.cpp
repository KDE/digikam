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

#include "tableview_column_configuration_dialog.moc"

// Qt includes

#include <QContextMenuEvent>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QTreeView>
#include <QVBoxLayout>

// KDE includes

#include <kmenu.h>
#include <kaction.h>
#include <klinkitemselectionmodel.h>

// local includes

/// @todo clean up includes
#include "contextmenuhelper.h"
#include "databasefields.h"
#include "databasewatch.h"
#include "digikam2kgeomap_database.h"
#include "fileactionmngr.h"
#include "imageinfo.h"
#include "imagemodel.h"
#include "imageposition.h"
#include "importfiltermodel.h"
#include "importimagemodel.h"
#include "importui.h"
#include "tableview_columnfactory.h"
#include "tableview_model.h"
#include "tableview_selection_model_syncer.h"
#include "thumbnailloadthread.h"

namespace Digikam
{

class ImageAlbumModel;
class ImageFilterModel;

class TableViewConfigurationDialog::Private
{
public:

    Private()
      : columnIndex(0),
        columnObject(0)
    {
    }

    int columnIndex;
    TableViewColumn* columnObject;
    TableViewColumnConfigurationWidget* columnConfigurationWidget;
};

TableViewConfigurationDialog::TableViewConfigurationDialog(
            TableViewShared* const sharedObject,
            const int columnIndex,
            QWidget* const parentWidget)
  : KDialog(parentWidget),
    d(new Private()),
    s(sharedObject)
{
    d->columnIndex = columnIndex;
    d->columnObject = s->tableViewModel->getColumnObject(d->columnIndex);

    setWindowTitle(i18n("Configure column \"%1\"").arg(d->columnObject->getTitle()));

    d->columnConfigurationWidget = d->columnObject->getConfigurationWidget(this);
    setMainWidget(d->columnConfigurationWidget);
}

TableViewConfigurationDialog::~TableViewConfigurationDialog()
{

}

TableViewColumnConfiguration TableViewConfigurationDialog::getNewConfiguration() const
{
    return d->columnConfigurationWidget->getNewConfiguration();
}

} /* namespace Digikam */
