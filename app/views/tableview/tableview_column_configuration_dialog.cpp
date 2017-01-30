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

#include "tableview_column_configuration_dialog.h"

// Qt includes

#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QPushButton>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "tableview_columnfactory.h"
#include "tableview_model.h"
#include "tableview_selection_model_syncer.h"
#include "thumbnailloadthread.h"
#include "digikam_debug.h"

namespace Digikam
{

class ImageAlbumModel;
class ImageFilterModel;

class TableViewConfigurationDialog::Private
{
public:

    Private()
      : columnIndex(0),
        buttons(0),
        columnObject(0),
        columnConfigurationWidget(0)
    {
    }

    int                                 columnIndex;

    QDialogButtonBox*                   buttons;

    TableViewColumn*                    columnObject;
    TableViewColumnConfigurationWidget* columnConfigurationWidget;
};

TableViewConfigurationDialog::TableViewConfigurationDialog(TableViewShared* const sharedObject,
                                                           const int columnIndex,
                                                           QWidget* const parentWidget)
    : QDialog(parentWidget),
      d(new Private()),
      s(sharedObject)
{
    d->columnIndex               = columnIndex;
    d->columnObject              = s->tableViewModel->getColumnObject(d->columnIndex);
    d->columnConfigurationWidget = d->columnObject->getConfigurationWidget(this);

    if (d->columnObject)
    {
        setWindowTitle(i18n("Configure column \"%1\"", d->columnObject->getTitle()));
    }
    else
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "Column object from TableView is null";
    }

    d->buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    d->buttons->button(QDialogButtonBox::Ok)->setDefault(true);

    QVBoxLayout* const vbx = new QVBoxLayout(this);
    vbx->addWidget(d->columnConfigurationWidget);
    vbx->addWidget(d->buttons);
    setLayout(vbx);

    connect(d->buttons->button(QDialogButtonBox::Ok), SIGNAL(clicked()),
            this, SLOT(accept()));

    connect(d->buttons->button(QDialogButtonBox::Cancel), SIGNAL(clicked()),
            this, SLOT(reject()));
}

TableViewConfigurationDialog::~TableViewConfigurationDialog()
{
}

TableViewColumnConfiguration TableViewConfigurationDialog::getNewConfiguration() const
{
    if (d->columnConfigurationWidget)
    {
        return d->columnConfigurationWidget->getNewConfiguration();
    }
    else
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "Configuration widget from TableView is null";
        return TableViewColumnConfiguration();
    }
}

} // namespace Digikam
