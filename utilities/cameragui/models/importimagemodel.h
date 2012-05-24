/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-05-22
 * Description : Qt item model for camera entries
 *
 * Copyright (C) 2009-2012 by Islam Wazery <wazery at ubuntu dot com>
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

#ifndef IMPORTIMAGEMODEL_H
#define IMPORTIMAGEMODEL_H

// Qt includes

#include <QAbstractListModel>

// KDE includes

#include <kurl.h>

// Local includes

#include "dragdropimplementations.h"
#include "cameracontroller.h"
#include "camiteminfo.h"

namespace Digikam
{
class AbstractItemDragDropHandler;

class ImportImageModel : public QAbstractListModel, public DragDropModelImplementation
{
    Q_OBJECT

public:

    enum ImportImageModelRoles
    {
        /// An ImportImageModel* pointer to this model
        ImportImageModelPointerRole   = Qt::UserRole,
        ImportImageModelInternalId    = Qt::UserRole + 1,

        /// Returns a thumbnail pixmap. May be implemented by subclasses.
        /// Returns either a valid pixmap or a null QVariant.
        ThumbnailRole   = Qt::UserRole + 2
    };

    ImportImageModel(Digikam::CameraController* controller, QObject* parent = 0);
    ~ImportImageModel();

    /** Returns the CamItemInfo object, reference from the underlying data pointed to by the index.
     *  For camItemInfo and camItemInfoId If the index is not valid they will return a null CamItemInfo, and 0
     *  respectively, camItemInfoRef must not be called with an invalid index as it will crash. */
    CamItemInfo      camItemInfo(const QModelIndex& index) const;
    CamItemInfo&     camItemInfoRef(const QModelIndex& index) const;
    qlonglong        camItemId(const QModelIndex& index) const;
    CamItemInfoList  camItemInfos(const QList<QModelIndex>& indexes) const;
    QList<qlonglong> camItemIds(const QList<QModelIndex>& indexes) const;

    /** Returns the CamItemInfo object, reference from the underlying data of
     *  the given row (parent is the invalid QModelIndex, column is 0).
     *  Note that camItemInfoRef must not be called with an invalid index as it will crash. */
    CamItemInfo  camItemInfo(int row) const;
    CamItemInfo& camItemInfoRef(int row) const;
    qlonglong    camItemId(int row) const;

    /** Return the index of a given CamItemInfo, it it exists in the model. */
    QModelIndex        indexForCamItemInfo(const CamItemInfo& info) const;
    QList<QModelIndex> indexesForCamItemInfo(const CamItemInfo& info) const;
    QModelIndex        indexForCamItemId(qlonglong id) const;
    QList<QModelIndex> indexesForCamItemId(qlonglong id) const;

    int numberOfIndexesForCamItemInfo(const CamItemInfo& info) const;
    int numberOfIndexesForCamItemId(qlonglong id) const;

    /** Returns the index or CamItemInfo object from the underlying data for
     *  the given file url. In case of multiple occurrences of the same file, the simpler
     *  overrides returns any one found first, use the QList methods to retrieve all occurrences. */
    QModelIndex        indexForUrl(const KUrl& fileUrl) const;
    QList<QModelIndex> indexesForUrl(const KUrl& fileUrl) const;
    CamItemInfo        camItemInfo(const KUrl& fileUrl) const;
    QList<CamItemInfo> camItemInfos(const KUrl& fileUrl) const;

    void addCamItemInfo(const CamItemInfo& info);
    void addCamItemInfos(const QList<CamItemInfo>& infos);

    /** Clears the CamItemInfos and resets the model.*/
    void clearCamItemInfos();

    /** Clears and adds infos. */
    void setCamItemInfos(const QList<CamItemInfo>& infos);

    /**
     * Remove the given infos or indexes directly from the model.
     */
    void removeIndex(const QModelIndex& indexes);
    void removeIndexs(const QList<QModelIndex>& indexes);
    void removeCamItemInfo(const CamItemInfo& info);
    void removeCamItemInfos(const QList<CamItemInfo>& infos);

    /** Retrieve the CamItemInfo object from the data() function of the given index
     *  The index may be from a QSortFilterProxyModel as long as an ImportImageModel is at the end. */
    static CamItemInfo retrieveCamItemInfo(const QModelIndex& index);
    static qlonglong   retrieveCamItemId(const QModelIndex& index);

    bool isEmpty() const;

    // QAbstractListModel implementation
    virtual int rowCount(const QModelIndex &parent) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual QModelIndex index(int row, int column, const QModelIndex &parent) const;

    // DragDrop methods
    DECLARE_MODEL_DRAG_DROP_METHODS

Q_SIGNALS:

    /** Informs that ItemInfos will be added to the model.
     *  This signal is sent before the model data is changed and views are informed. */
    void itemInfosAboutToBeAdded(const QList<CamItemInfo>& infos);

    /** Informs that ItemInfos have been added to the model.
     *  This signal is sent after the model data is changed and views are informed. */
    void itemInfosAdded(const QList<CamItemInfo>& infos);

public:

    //FIXME: Declared public because it's used in ImageModelIncrementalUpdater class
    class ImportImageModelPriv;

private:

    ImportImageModelPriv* const d;
};

} // namespace Digikam

Q_DECLARE_METATYPE(Digikam::ImportImageModel*)

#endif // IMPORTIMAGEMODEL_H
