/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-03-14
 * Description : Table view column helpers: Photo properties
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

#ifndef TABLEVIEW_COLUMN_PHOTO_H
#define TABLEVIEW_COLUMN_PHOTO_H

// Qt includes

#include <QStringList>

// Local includes

#include "tableview_columnfactory.h"

class QComboBox;

namespace Digikam
{

namespace TableViewColumns
{

class ColumnPhotoProperties : public TableViewColumn
{
    Q_OBJECT

public:

    enum SubColumn
    {
        SubColumnCameraMaker  = 0,
        SubColumnCameraModel  = 1,
        SubColumnLens         = 2,
        SubColumnAperture     = 3,
        SubColumnFocal        = 4,
        SubColumnExposure     = 5,
        SubColumnSensitivity  = 6,
        SubColumnModeProgram  = 7,
        SubColumnFlash        = 8,
        SubColumnWhiteBalance = 9
    };

private:

    SubColumn subColumn;

public:

    explicit ColumnPhotoProperties(TableViewShared* const tableViewShared,
                                   const TableViewColumnConfiguration& pConfiguration,
                                   const SubColumn pSubColumn,
                                   QObject* const parent = 0);
    virtual ~ColumnPhotoProperties();

    virtual QString getTitle() const;
    virtual ColumnFlags getColumnFlags() const;
    virtual QVariant data(TableViewModel::Item* const item, const int role) const;
    virtual ColumnCompareResult compare(TableViewModel::Item* const itemA, TableViewModel::Item* const itemB) const;
    virtual TableViewColumnConfigurationWidget* getConfigurationWidget(QWidget* const parentWidget) const;
    virtual void setConfiguration(const TableViewColumnConfiguration& newConfiguration);

    static TableViewColumnDescription getDescription();
    static QStringList getSubColumns();
};

// ----------------------------------------------------------------------------------------------------------------------

class ColumnPhotoConfigurationWidget : public TableViewColumnConfigurationWidget
{
    Q_OBJECT

public:

    explicit ColumnPhotoConfigurationWidget(TableViewShared* const sharedObject,
                                            const TableViewColumnConfiguration& columnConfiguration,
                                            QWidget* const parentWidget);
    virtual ~ColumnPhotoConfigurationWidget();

    virtual TableViewColumnConfiguration getNewConfiguration();

private Q_SLOTS:

    void slotUpdateUI();

private:

    ColumnPhotoProperties::SubColumn subColumn;
    QComboBox*                       selectorExposureTimeFormat;
    QComboBox*                       selectorExposureTimeUnit;
};

} /* namespace TableViewColumns */

} /* namespace Digikam */

#endif // TABLEVIEW_COLUMN_PHOTO_H
