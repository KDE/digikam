/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-03-22
 * Description : A view to display a list of images.
 *
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2010      by Michael G. Hansen <mike at mghansen dot de>
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

#ifndef GPSIMAGELIST_H
#define GPSIMAGELIST_H

// Qt includes

#include <QTreeView>

// Local includes

#include "gpsimagemodel.h"
#include "gpsimagesortproxymodel.h"

class QWheelEvent;
class KConfigGroup;

namespace Digikam
{

class ImageListDragDropHandler;

class GPSImageList : public QTreeView
{
    Q_OBJECT

public:

    explicit GPSImageList(QWidget* const parent = 0);
    ~GPSImageList();

    void setModelAndSelectionModel(GPSImageModel* const model, QItemSelectionModel* const selectionModel);
    GPSImageModel* getModel() const;
    QItemSelectionModel* getSelectionModel() const;
    void setDragDropHandler(ImageListDragDropHandler* const dragDropHandler);
    void setThumbnailSize(const int size);
    GPSImageSortProxyModel* getSortProxyModel() const;

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

} // namespace Digikam

#endif // GPSIMAGELIST_H
