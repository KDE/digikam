/** ===========================================================
 * @file
 *
 * This file is a part of kipi-plugins project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2010-03-22
 * @brief  A view to display a list of images.
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

#ifndef KIPIIMAGELIST_H
#define KIPIIMAGELIST_H

// Qt includes

#include <QItemDelegate>
#include <QTreeView>

// Local includes

#include "kipiimagemodel.h"

class QWheelEvent;
class KConfigGroup;

namespace Digikam
{

class KipiImageSortProxyModel;

class KipiImageListDragDropHandler : public QObject
{
    Q_OBJECT

public:

    explicit KipiImageListDragDropHandler(QObject* const parent = 0);
    virtual ~KipiImageListDragDropHandler();

    virtual QMimeData* createMimeData(const QList<QPersistentModelIndex>& modelIndices) = 0;
};

// -------------------------------------------------------------------------------------------------

class KipiImageList : public QTreeView
{
    Q_OBJECT

public:

    explicit KipiImageList(QWidget* const parent = 0);
    ~KipiImageList();

    void setModelAndSelectionModel(KipiImageModel* const model, QItemSelectionModel* const selectionModel);
    KipiImageModel* getModel() const;
    QItemSelectionModel* getSelectionModel() const;
    void setDragDropHandler(KipiImageListDragDropHandler* const dragDropHandler);
    void setThumbnailSize(const int size);
    KipiImageSortProxyModel* getSortProxyModel() const;

    void saveSettingsToGroup(KConfigGroup* const group);
    void readSettingsFromGroup(const KConfigGroup* const group);
    void setEditEnabled(const bool state);
    void setDragEnabled(const bool state);

Q_SIGNALS:

    void signalImageActivated(const QModelIndex& index);

public Q_SLOTS:

    void slotIncreaseThumbnailSize();
    void slotDecreaseThumbnailSize();
    void slotUpdateActionsEnabled();

private Q_SLOTS:

    void slotThumbnailFromModel(const QPersistentModelIndex& index, const QPixmap& pixmap);
    void slotInternalTreeViewImageActivated(const QModelIndex& index);
    void slotColumnVisibilityActionTriggered(QAction* action);

protected:

    virtual bool eventFilter(QObject* watched, QEvent* event);
    virtual void startDrag(Qt::DropActions supportedActions);
    virtual void wheelEvent(QWheelEvent* we);

private:

    class Private;
    Private* const d;
};

// -------------------------------------------------------------------------------------------------

class KipiImageItemDelegate : public QItemDelegate
{
    Q_OBJECT

public:

    explicit KipiImageItemDelegate(KipiImageList* const imageList, QObject* const parent = 0);
    virtual ~KipiImageItemDelegate();

    void setThumbnailSize(const int size);
    int  getThumbnailSize() const;

    virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& sortMappedindex) const;
    virtual QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& sortMappedindex) const;

private:

    class Private;
    Private* const d;
};

} /* Digikam */

#endif /* KIPIIMAGELIST_H */
