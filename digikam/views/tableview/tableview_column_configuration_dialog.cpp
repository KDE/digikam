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

#include <QItemSelectionModel>
#include <QVBoxLayout>

// KDE includes

#include <klauncher_iface.h>
#include <klocale.h>

// local includes

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
        columnObject(0),
        columnConfigurationWidget(0)
    {
    }

    int                                 columnIndex;
    TableViewColumn*                    columnObject;
    TableViewColumnConfigurationWidget* columnConfigurationWidget;
};

TableViewConfigurationDialog::TableViewConfigurationDialog(TableViewShared* const sharedObject,
                                                           const int columnIndex,
                                                           QWidget* const parentWidget)
    : KDialog(parentWidget), d(new Private()), s(sharedObject)
{
    d->columnIndex  = columnIndex;
    d->columnObject = s->tableViewModel->getColumnObject(d->columnIndex);

    setWindowTitle(i18n("Configure column \"%1\"", d->columnObject->getTitle()));

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
