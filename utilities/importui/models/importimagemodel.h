/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-05-22
 * Description : Qt model for camera entries
 *
 * Copyright (C) 2012 by Islam Wazery <wazery at ubuntu dot com>
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
#include <QUrl>

// Local includes

#include "dragdropimplementations.h"
#include "camerathumbsctrl.h"
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
        ImportImageModelPointerRole = Qt::UserRole,
        ImportImageModelInternalId  = Qt::UserRole + 1,

        /// Returns a thumbnail pixmap. May be implemented by subclasses.
        /// Returns either a valid pixmap or a null QVariant.
        ThumbnailRole               = Qt::UserRole + 2,
        /// Return (optional) extraData field
        ExtraDataRole               = Qt::UserRole + 3,

        /// Returns the number of duplicate indexes for the same image id
        ExtraDataDuplicateCount     = Qt::UserRole + 6,

        FilterModelRoles            = Qt::UserRole + 100
    };

public:

    explicit ImportImageModel(QObject* const parent = 0);
    ~ImportImageModel();

    // Used to set the camera controller, and connect with it.
    virtual void setCameraThumbsController(CameraThumbsCtrl* const controller);

    /** If a cache is kept, lookup by file path is fast,
     *  without a cache it is O(n). Default is false. */
    void setKeepsFileUrlCache(bool keepCache);
    bool keepsFileUrlCache() const;

    /**
     *  Returns the CamItemInfo object, reference from the underlying data pointed to by the index.
     *  For camItemInfo and camItemInfoId If the index is not valid they will return a null CamItemInfo, and 0
     *  respectively, camItemInfoRef must not be called with an invalid index as it will crash.
     */
    CamItemInfo      camItemInfo(const QModelIndex& index)           const;
    CamItemInfo&     camItemInfoRef(const QModelIndex& index)        const;
    qlonglong        camItemId(const QModelIndex& index)             const;
    CamItemInfoList  camItemInfos(const QList<QModelIndex>& indexes) const;
    QList<qlonglong> camItemIds(const QList<QModelIndex>& indexes)   const;

    /**
     * Returns the CamItemInfo object, reference from the underlying data of
     * the given row (parent is the invalid QModelIndex, column is 0).
     * Note that camItemInfoRef must not be called with an invalid index as it will crash.
     */
    CamItemInfo  camItemInfo(int row)    const;
    CamItemInfo& camItemInfoRef(int row) const;
    qlonglong    camItemId(int row)      const;

    /**
     * Return the index of a given CamItemInfo, if it exists in the model.
     */
    QModelIndex        indexForCamItemInfo(const CamItemInfo& info)   const;
    QList<QModelIndex> indexesForCamItemInfo(const CamItemInfo& info) const;
    QModelIndex        indexForCamItemId(qlonglong id)                const;
    QList<QModelIndex> indexesForCamItemId(qlonglong id)              const;

    /**
     * Returns the index or CamItemInfo object from the underlying data for
     * the given file url. In case of multiple occurrences of the same file, the simpler
     * overrides returns any one found first, use the QList methods to retrieve all occurrences.
     */
    QModelIndex        indexForUrl(const QUrl& fileUrl)   const;
    QList<QModelIndex> indexesForUrl(const QUrl& fileUrl) const;
    CamItemInfo        camItemInfo(const QUrl& fileUrl)   const;
    QList<CamItemInfo> camItemInfos(const QUrl& fileUrl)  const;

    /**
     * Clears the CamItemInfos and resets the model.
     */
    void clearCamItemInfos();

    /**
     * addCamItemInfo() is asynchronous if a prepocessor is set.
     * This method first adds the info, synchronously.
     * Only afterwards, the preprocessor will have the opportunity to process it.
     * This method also bypasses any incremental updates.
     */
    void addCamItemInfoSynchronously(const CamItemInfo& info);
    void addCamItemInfosSynchronously(const Digikam::CamItemInfoList& infos);

    /**
     * Clears and adds infos.
     */
    void setCamItemInfos(const CamItemInfoList& infos);

    QList<CamItemInfo> camItemInfos()       const;
    QList<qlonglong>   camItemIds()         const;
    QList<CamItemInfo> uniqueCamItemInfos() const;

    bool hasImage(qlonglong id) const;
    bool hasImage(const CamItemInfo& info)  const;

    bool isEmpty() const;

    /**
     * Remove the given infos or indexes directly from the model.
     */
    void removeIndex(const QModelIndex& index);
    void removeIndexs(const QList<QModelIndex>& indexes);
    void removeCamItemInfo(const CamItemInfo& info);
    void removeCamItemInfos(const QList<CamItemInfo>& infos);

    int numberOfIndexesForCamItemInfo(const CamItemInfo& info) const;
    int numberOfIndexesForCamItemId(qlonglong id)              const;

    /**
     * Retrieve the CamItemInfo object from the data() function of the given index
     * The index may be from a QSortFilterProxyModel as long as an ImportImageModel is at the end.
     */
    static CamItemInfo retrieveCamItemInfo(const QModelIndex& index);
    static qlonglong   retrieveCamItemId(const QModelIndex& index);

    // QAbstractListModel implementation
    virtual int           rowCount(const QModelIndex& parent)                            const;
    virtual QVariant      data(const QModelIndex& index, int role)                       const;
    virtual QVariant      headerData(int section, Qt::Orientation orientation, int role) const;
    virtual Qt::ItemFlags flags(const QModelIndex& index)                                const;
    virtual QModelIndex   index(int row, int column, const QModelIndex& parent)          const;

    // DragDrop methods
    DECLARE_MODEL_DRAG_DROP_METHODS

    /**
     * Enable sending of itemInfosAboutToBeRemoved and itemsInfosRemoved signals.
     * Default: false
     */
    void setSendRemovalSignals(bool send);

    /**
     * Returns true if this model is currently refreshing.
     * For a preprocessor this means that, although the preprocessor may currently have
     * processed all it got, more batches are to be expected.
     */
    bool isRefreshing() const;

Q_SIGNALS:

    /**
     * Informs that ItemInfos will be added to the model.
     * This signal is sent before the model data is changed and views are informed.
     */
    void itemInfosAboutToBeAdded(const QList<CamItemInfo>& infos);

    /**
     * Informs that ItemInfos have been added to the model.
     * This signal is sent after the model data is changed and views are informed.
     */
    void itemInfosAdded(const QList<CamItemInfo>& infos);

    /**
     * Informs that CamItemInfos will be removed from the model.
     * This signal is sent before the model data is changed and views are informed.
     * Note: You need to explicitly enable sending of this signal. It is not sent
     * in clearCamItemInfos().
     */
    void itemInfosAboutToBeRemoved(const QList<CamItemInfo>& infos);

    /**
     * Informs that CamItemInfos have been removed from the model.
     * This signal is sent after the model data is changed and views are informed.
     * Note: You need to explicitly enable sending of this signal. It is not sent
     * in clearCamItemInfos().
     */
    void itemInfosRemoved(const QList<CamItemInfo>& infos);

    /**
     * Connect to this signal only if you are the current preprocessor.
     */
    void preprocess(const QList<CamItemInfo>& infos);
    void processAdded(const QList<CamItemInfo>& infos);

    /**
     * Signals that the model is right now ready to start an incremental refresh.
     * This is guaranteed only for the scope of emitting this signal.
     */
    void readyForIncrementalRefresh();

    /**
     * Signals that the model has finished currently with all scheduled
     * refreshing, full or incremental, and all preprocessing.
     * The model is in polished, clean situation right now.
     */
    void allRefreshingFinished();

public Q_SLOTS:

    void reAddCamItemInfos(const CamItemInfoList& infos);
    void reAddingFinished();
    void slotFileDeleted(const QString& folder, const QString& file, bool status);
    void slotFileUploaded(const CamItemInfo& info);
    void addCamItemInfo(const CamItemInfo& info);
    void addCamItemInfos(const CamItemInfoList& infos);

protected:

    /**
     * Subclasses that add CamItemInfos in batches shall call startRefresh()
     * when they start sending batches and finishRefresh() when they have finished.
     * No incremental refreshes will be started while listing.
     * A clearCamItemInfos() always stops listing, calling finishRefresh() is then not necessary.
     */
    void startRefresh();
    void finishRefresh();

    /**
     * As soon as the model is ready to start an incremental refresh, the signal
     * readyForIncrementalRefresh() will be emitted. The signal will be emitted inline
     * if the model is ready right now.
     */
    void requestIncrementalRefresh();
    bool hasIncrementalRefreshPending() const;

    /**
     * Starts an incremental refresh operation. You shall only call this method from a slot
     * connected to readyForIncrementalRefresh(). To initiate an incremental refresh,
     * call requestIncrementalRefresh().
     */
    void startIncrementalRefresh();
    void finishIncrementalRefresh();

    void emitDataChangedForAll();
    void emitDataChangedForSelections(const QItemSelection& selection);

    // Called when the internal storage is cleared.
    virtual void camItemInfosCleared() {};

    // Called before rowsAboutToBeRemoved
    virtual void itemInfosAboutToBeRemoved(int /*begin*/, int /*end*/) {};

private:

    void appendInfos(const CamItemInfoList& infos);
    void publiciseInfos(const CamItemInfoList& infos);
    void cleanSituationChecks();
    void removeRowPairs(const QList<QPair<int, int> >& toRemove);
    void removeRowPairsWithCheck(const QList<QPair<int, int> >& toRemove);

public:

    // NOTE: Declared public because it's used in ImageModelIncrementalUpdater class
    class Private;

private:

    Private* const d;
};

} // namespace Digikam

Q_DECLARE_METATYPE(Digikam::ImportImageModel*)

#endif // IMPORTIMAGEMODEL_H
