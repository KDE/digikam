/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-07-05
 * Description : Qt model for Showfoto entries
 *
 * Copyright (C) 2013 by Mohamed Anwer <m dot anwer at gmx dot com>
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

#ifndef SHOWFOTOIMAGEMODEL_H
#define SHOWFOTOIMAGEMODEL_H

// Qt includes

#include <QAbstractListModel>
#include <QUrl>

// Local includes

#include "dragdropimplementations.h"
#include "abstractitemdragdrophandler.h"
#include "showfotoiteminfo.h"

using namespace Digikam;

namespace ShowFoto
{

typedef QPair<int, int> IntPair;

class ShowfotoImageModel : public QAbstractListModel, public DragDropModelImplementation
{
    Q_OBJECT

public:
    enum ShowfotoImageModelRoles
    {
        /// An ShowfotoImageModel* pointer to this model
        ShowfotoImageModelPointerRole = Qt::UserRole,
        ShowfotoImageModelInternalId  = Qt::UserRole + 1,

        /// Returns a thumbnail pixmap. May be implemented by subclasses.
        /// Returns either a valid pixmap or a null QVariant.
        ThumbnailRole                 = Qt::UserRole + 2,
        /// Return (optional) extraData field
        ExtraDataRole                 = Qt::UserRole + 3,

        /// Returns the number of duplicate indexes for the same image id
        ExtraDataDuplicateCount       = Qt::UserRole + 6,

        FilterModelRoles              = Qt::UserRole + 100
    };

public:

     explicit ShowfotoImageModel(QObject* const parent);
    ~ShowfotoImageModel();

    /** If a cache is kept, lookup by file path is fast,
     *  without a cache it is O(n). Default is false.
     */
    void setKeepsFileUrlCache(bool keepCache);

    /**
     *  Returns the ShowfotoItemInfo object, reference from the underlying data pointed to by the index.
     *  For ShowfotoItemInfo and ShowfotoItemInfoId If the index is not valid they will return a null ShowfotoItemInfo, and 0
     *  respectively, ShowfotoItemInfoRef must not be called with an invalid index as it will crash.
     */
    ShowfotoItemInfo      showfotoItemInfo(const QModelIndex& index)           const;
    ShowfotoItemInfo&     showfotoItemInfoRef(const QModelIndex& index)        const;
    qlonglong             showfotoItemId(const QModelIndex& index)             const;
    ShowfotoItemInfoList  showfotoItemInfos(const QList<QModelIndex>& indexes) const;
    QList<qlonglong>      showfotoItemIds(const QList<QModelIndex>& indexes)   const;

    /**
     * Returns the ShowfotoItemInfo object, reference from the underlying data of
     * the given row (parent is the invalid QModelIndex, column is 0).
     * Note that ShowfotoItemInfoRef must not be called with an invalid index as it will crash.
     */
    ShowfotoItemInfo  showfotoItemInfo(int row)    const;
    ShowfotoItemInfo& showfotoItemInfoRef(int row) const;
    qlonglong         showfotoItemId(int row)      const;

    /**
     * Return the index of a given ShowfotoItemInfo, if it exists in the model.
     */
    QModelIndex        indexForShowfotoItemInfo(const ShowfotoItemInfo& info)   const;
    QList<QModelIndex> indexesForShowfotoItemInfo(const ShowfotoItemInfo& info) const;
    QModelIndex        indexForShowfotoItemId(qlonglong id)                     const;
    QList<QModelIndex> indexesForShowfotoItemId(qlonglong id)                   const;

    /**
     * Returns the index or ShowfotoItemInfo object from the underlying data for
     * the given file url. In case of multiple occurrences of the same file, the simpler
     * overrides returns any one found first, use the QList methods to retrieve all occurrences.
     */
    QModelIndex             indexForUrl(const QUrl& fileUrl)        const;
    QList<QModelIndex>      indexesForUrl(const QUrl& fileUrl)      const;
    ShowfotoItemInfo        showfotoItemInfo(const QUrl& fileUrl)   const;
    QList<ShowfotoItemInfo> showfotoItemInfos(const QUrl& fileUrl)  const;

    void addShowfotoItemInfo(const ShowfotoItemInfo& info);
    void addShowfotoItemInfos(const QList<ShowfotoItemInfo>& infos);

    /**
     * Clears the ShowfotoItemInfos and resets the model.
     */
    void clearShowfotoItemInfos();

    /**
     * addShowfotoItemInfo() is asynchronous if a prepocessor is set.
     * This method first adds the info, synchronously.
     * Only afterwards, the preprocessor will have the opportunity to process it.
     * This method also bypasses any incremental updates.
     */
    void addShowfotoItemInfoSynchronously(const ShowfotoItemInfo& info);
    void addShowfotoItemInfosSynchronously(const QList<ShowfotoItemInfo>& infos);

    /**
     * Clears and adds infos.
     */
    void setShowfotoItemInfos(const QList<ShowfotoItemInfo>& infos);

    QList<ShowfotoItemInfo> showfotoItemInfos()       const;
    QList<qlonglong>        showfotoItemIds()         const;
    QList<ShowfotoItemInfo> uniqueShowfotoItemInfos() const;

    bool hasImage(qlonglong id) const;
    bool hasImage(const ShowfotoItemInfo& info)  const;

    bool isEmpty() const;

    /**
     * Remove the given infos or indexes directly from the model.
     */
    void removeIndex(const QModelIndex& index);
    void removeIndexs(const QList<QModelIndex>& indexes);
    void removeShowfotoItemInfo(const ShowfotoItemInfo& info);
    void removeShowfotoItemInfos(const QList<ShowfotoItemInfo>& infos);

    int numberOfIndexesForShowfotoItemInfo(const ShowfotoItemInfo& info) const;
    int numberOfIndexesForShowfotoItemId(qlonglong id)                       const;

    /**
     * Retrieve the ShowfotoItemInfo object from the data() function of the given index
     * The index may be from a QSortFilterProxyModel as long as an ShowfotoImageModel is at the end.
     */
    static ShowfotoItemInfo retrieveShowfotoItemInfo(const QModelIndex& index);
    static qlonglong        retrieveShowfotoItemId(const QModelIndex& index);

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

Q_SIGNALS:

    /**
     * Informs that ItemInfos will be added to the model.
     * This signal is sent before the model data is changed and views are informed.
     */
    void itemInfosAboutToBeAdded(const QList<ShowfotoItemInfo>& infos);

    /**
     * Informs that ItemInfos have been added to the model.
     * This signal is sent after the model data is changed and views are informed.
     */
    void itemInfosAdded(const QList<ShowfotoItemInfo>& infos);


    /**
     * Informs that ShowfotoItemInfos will be removed from the model.
     * This signal is sent before the model data is changed and views are informed.
     * Note: You need to explicitly enable sending of this signal. It is not sent
     * in clearShowfotoItemInfos().
     */
    void itemInfosAboutToBeRemoved(const QList<ShowfotoItemInfo>& infos);

    /**
     * Informs that ShowfotoItemInfos have been removed from the model.
     * This signal is sent after the model data is changed and views are informed.
     * Note: You need to explicitly enable sending of this signal. It is not sent
     * in clearShowfotoItemInfos().
     */
    void itemInfosRemoved(const QList<ShowfotoItemInfo>& infos);

    /**
     * Connect to this signal only if you are the current preprocessor.
     */
    void preprocess(const QList<ShowfotoItemInfo>& infos);
    void processAdded(const QList<ShowfotoItemInfo>& infos);

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

    void reAddShowfotoItemInfos(ShowfotoItemInfoList& infos);
    void reAddingFinished();
    void slotFileDeleted(const QString& folder, const QString& file, bool status);
    void slotFileUploaded(const ShowfotoItemInfo& info);

protected:

    /**
     * As soon as the model is ready to start an incremental refresh, the signal
     * readyForIncrementalRefresh() will be emitted. The signal will be emitted inline
     * if the model is ready right now.
     */
    void requestIncrementalRefresh();

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
    virtual void showfotoItemInfosCleared() {};

    // Called before rowsAboutToBeRemoved
    virtual void showfotoItemInfosAboutToBeRemoved(int /*begin*/, int /*end*/) {};

private:

    void appendInfos(const QList<ShowfotoItemInfo>& infos);
    void publiciseInfos(const QList<ShowfotoItemInfo>& infos);
    // void cleanSituationChecks();
    void removeRowPairs(const QList<QPair<int, int> >& toRemove);
    // void removeRowPairsWithCheck(const QList<QPair<int, int> >& toRemove);
    static QList<IntPair> toContiguousPairs(const QList<int>& unsorted);

public:

    // NOTE: Declared public because it's used in ImageModelIncrementalUpdater class
    class Private;

private:

    Private* const d;
};
}

Q_DECLARE_METATYPE(ShowFoto::ShowfotoImageModel*)

#endif // SHOWFOTOIMAGEMODEL_H
