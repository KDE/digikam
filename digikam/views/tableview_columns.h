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

#ifndef TABLEVIEW_COLUMNS_H
#define TABLEVIEW_COLUMNS_H

// Qt includes

#include <QObject>
#include <QPainter>
#include <QStringList>

// KDE includes

#include <kglobal.h>
#include <klocale.h>

// local includes

#include "tableview_columnfactory.h"
#include <libkgeomap/geocoordinates.h>
#include "thumbnailloadthread.h"

namespace Digikam
{

namespace TableViewColumns
{

class ColumnFileProperties : public TableViewColumn
{
    Q_OBJECT

public:

    enum SubColumn
    {
        SubColumnName = 0,
        SubColumnSize = 1
    };

private:
    SubColumn subColumn;

public:

    explicit ColumnFileProperties(
            TableViewShared* const tableViewShared,
            const TableViewColumnConfiguration& pConfiguration,
            QObject* const parent = 0
        );
    virtual ~ColumnFileProperties() { }

    static TableViewColumnDescription getDescription();
    static QStringList getSubColumns();
    virtual TableViewColumnConfigurationWidget* getConfigurationWidget(QWidget* const parentWidget) const;
    virtual void setConfiguration(const TableViewColumnConfiguration& newConfiguration);

    virtual QString getTitle();

    virtual ColumnFlags getColumnFlags() const;

    virtual QVariant data(const QModelIndex& sourceIndex, const int role);

    virtual ColumnCompareResult compare(const QModelIndex& sourceA, const QModelIndex& sourceB) const;

};

class ColumnFileConfigurationWidget : public TableViewColumnConfigurationWidget
{
    Q_OBJECT

public:
    explicit ColumnFileConfigurationWidget(
            TableViewShared* const sharedObject,
            const TableViewColumnConfiguration& columnConfiguration,
            QWidget* const parentWidget
        );
    virtual ~ColumnFileConfigurationWidget();

    virtual TableViewColumnConfiguration getNewConfiguration();

private:

    ColumnFileProperties::SubColumn subColumn;
    QComboBox* selectorSizeType;
};

class ColumnItemProperties : public TableViewColumn
{
    Q_OBJECT

private:

    enum SubColumn
    {
        SubColumnWidth = 0,
        SubColumnHeight = 1
    } subColumn;

public:

    explicit ColumnItemProperties(
            TableViewShared* const tableViewShared,
            const TableViewColumnConfiguration& pConfiguration,
            QObject* const parent = 0
        );
    virtual ~ColumnItemProperties();

    static TableViewColumnDescription getDescription();

    virtual QString getTitle();

    virtual ColumnFlags getColumnFlags() const;

    virtual QVariant data(const QModelIndex& sourceIndex, const int role);

    virtual ColumnCompareResult compare(const QModelIndex& sourceA, const QModelIndex& sourceB) const;
};

class ColumnGeoProperties : public TableViewColumn
{
    Q_OBJECT

public:

    enum SubColumn
    {
        SubColumnHasCoordinates = 0,
        SubColumnCoordinates = 1,
        SubColumnAltitude = 2
    } subColumn;

    explicit ColumnGeoProperties(
            TableViewShared* const tableViewShared,
            const TableViewColumnConfiguration& pConfiguration,
            QObject* const parent = 0
        );
    virtual ~ColumnGeoProperties();
    static TableViewColumnDescription getDescription();

    virtual QString getTitle();

    virtual ColumnFlags getColumnFlags() const;

    virtual QVariant data(const QModelIndex& sourceIndex, const int role);

    virtual ColumnCompareResult compare(const QModelIndex& sourceA, const QModelIndex& sourceB) const;

    virtual TableViewColumnConfigurationWidget* getConfigurationWidget(QWidget* const parentWidget) const;
};

class ColumnGeoConfigurationWidget : public TableViewColumnConfigurationWidget
{
    Q_OBJECT

public:
    explicit ColumnGeoConfigurationWidget(
            TableViewShared* const sharedObject,
            const TableViewColumnConfiguration& columnConfiguration,
            QWidget* const parentWidget
        );
    virtual ~ColumnGeoConfigurationWidget();

    virtual TableViewColumnConfiguration getNewConfiguration();

private:

    ColumnGeoProperties::SubColumn subColumn;
};

class ColumnThumbnail : public TableViewColumn
{
    Q_OBJECT

public:

    explicit ColumnThumbnail(
            TableViewShared* const tableViewShared,
            const TableViewColumnConfiguration& pConfiguration,
            QObject* const parent = 0
        );

    virtual ~ColumnThumbnail();

    static TableViewColumnDescription getDescription();

    virtual ColumnFlags getColumnFlags() const;

    virtual QString getTitle();

    virtual QVariant data(const QModelIndex& sourceIndex, const int role);

    virtual bool paint(QPainter* const painter, const QStyleOptionViewItem& option, const QModelIndex& sourceIndex) const;

    virtual QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& sourceIndex) const;

private Q_SLOTS:

    void slotThumbnailLoaded(const LoadingDescription& loadingDescription, const QPixmap& thumb);
};

} /* namespace TableViewColumns */

} /* namespace Digikam */

#endif // TABLEVIEW_COLUMNS_H

