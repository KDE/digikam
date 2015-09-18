/** ===========================================================
 * @file
 *
 * This file is a part of kipi-plugins project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2010-06-01
 * @brief  A widget to search for places.
 *
 * @author Copyright (C) 2010 by Michael G. Hansen
 *         <a href="mailto:mike at mghansen dot de">mike at mghansen dot de</a>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef SEARCHWIDGET_H
#define SEARCHWIDGET_H

// Qt includes

#include <QAbstractItemModel>
#include <QWidget>

// libkgeomap includes

#include <KGeoMap/ModelHelper>

// local includes

#include "searchbackend.h"

class QEvent;
class QItemSelection;
class KConfigGroup;

namespace KGeoMap
{
    class MapWidget;
}

namespace Digikam
{

class GPSBookmarkOwner;
class GPSUndoCommand;
class SearchResultItem;

class SearchResultModel : public QAbstractItemModel
{
    Q_OBJECT

public:

    class SearchResultItem
    {
    public:

        SearchBackend::SearchResult result;
    };

public:

    explicit SearchResultModel(QObject* const parent = 0);
    ~SearchResultModel();

    void addResults(const SearchBackend::SearchResult::List& results);
    SearchResultItem resultItem(const QModelIndex& index) const;
    bool getMarkerIcon(const QModelIndex& index, QPoint* const offset, QSize* const size, QPixmap* const pixmap, QUrl* const url) const;
    void setSelectionModel(QItemSelectionModel* const selectionModel);
    void clearResults();
    void removeRowsByIndexes(const QModelIndexList& rowsList);
    void removeRowsBySelection(const QItemSelection& selection);

    // QAbstractItemModel:
    virtual int columnCount(const QModelIndex& parent = QModelIndex() ) const;
    virtual bool setData(const QModelIndex& index, const QVariant& value, int role);
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex() ) const;
    virtual QModelIndex parent(const QModelIndex& index) const;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
    virtual bool setHeaderData(int section, Qt::Orientation orientation, const QVariant& value, int role);
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    virtual Qt::ItemFlags flags(const QModelIndex& index) const;

private:

    class Private;
    Private* const d;
};

// ------------------------------------------------------------------------------------------------

class SearchResultModelHelper : public KGeoMap::ModelHelper
{
    Q_OBJECT

public:

    SearchResultModelHelper(SearchResultModel* const resultModel,
                            QItemSelectionModel* const selectionModel,
                            GPSImageModel* const imageModel,
                            QObject* const parent = 0);
    ~SearchResultModelHelper();

    void setVisibility(const bool state);

    virtual QAbstractItemModel* model() const;
    virtual QItemSelectionModel* selectionModel() const;
    virtual bool itemCoordinates(const QModelIndex& index, KGeoMap::GeoCoordinates* const coordinates) const;
    virtual bool itemIcon(const QModelIndex& index, QPoint* const offset, QSize* const size, QPixmap* const pixmap, QUrl* const url) const;
    virtual Flags modelFlags() const;
    virtual Flags itemFlags(const QModelIndex& index) const;
    virtual void snapItemsTo(const QModelIndex& targetIndex, const QList<QModelIndex>& snappedIndices);

Q_SIGNALS:

    void signalUndoCommand(GPSUndoCommand* undoCommand);

private:

    class Private;
    Private* const d;
};

// ------------------------------------------------------------------------------------------------

class SearchWidget : public QWidget
{
    Q_OBJECT

public:

    SearchWidget(GPSBookmarkOwner* const gpsBookmarkOwner, GPSImageModel* const kipiImageModel, QItemSelectionModel* const kipiImageSelectionModel, QWidget* parent = 0);
    ~SearchWidget();

    KGeoMap::ModelHelper* getModelHelper();
    void saveSettingsToGroup(KConfigGroup* const group);
    void readSettingsFromGroup(const KConfigGroup* const group);

    void setPrimaryMapWidget(KGeoMap::MapWidget* const mapWidget);

private Q_SLOTS:

    void slotSearchCompleted();
    void slotTriggerSearch();
    void slotCurrentlySelectedResultChanged(const QModelIndex& current, const QModelIndex& previous);
    void slotClearSearchResults();
    void slotVisibilityChanged(bool state);
    void slotCopyCoordinates();
    void slotMoveSelectedImagesToThisResult();
    void slotUpdateActionAvailability();
    void slotRemoveSelectedFromResultsList();

protected:

    virtual bool eventFilter(QObject *watched, QEvent *event);

Q_SIGNALS:

    void signalUndoCommand(GPSUndoCommand* undoCommand);

private:

    class Private;
    Private* const d;
};

} /* namespace Digikam */

#endif /* SEARCHWIDGET_H */
