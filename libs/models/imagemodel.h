/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-03-05
 * Description : Qt item model for database entries
 *
 * Copyright (C) 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef IMAGEMODEL_H
#define IMAGEMODEL_H

// Qt includes

#include <QAbstractListModel>

// Local includes

#include "dragdropimplementations.h"
#include "imageinfo.h"
#include "digikam_export.h"

class QItemSelection;

namespace Digikam
{

class ImageChangeset;
class ImageTagChangeset;

namespace DatabaseFields
{
class Set;
}

class DIGIKAM_DATABASE_EXPORT ImageModel : public QAbstractListModel, public DragDropModelImplementation
{
    Q_OBJECT

public:

    enum ImageModelRoles
    {
        /// An ImageModel* pointer to this model
        ImageModelPointerRole   = Qt::UserRole,
        ImageModelInternalId    = Qt::UserRole + 1,
        /// Returns a thumbnail pixmap. May be implemented by subclasses.
        /// Returns either a valid pixmap or a null QVariant.
        ThumbnailRole           = Qt::UserRole + 2,
        /// Returns a QDateTime with the creation date
        CreationDateRole        = Qt::UserRole + 3,
        /// Return (optional) extraData field
        ExtraDataRole           = Qt::UserRole + 5,
        /// Returns the number of duplicate indexes for the same image id
        ExtraDataDuplicateCount = Qt::UserRole + 6,

        // Roles which are defined here but not implemented by ImageModel
        /// Returns position of item in Left Light Table preview.
        LTLeftPanelRole         = Qt::UserRole + 50,
        /// Returns position of item in Right Light Table preview.
        LTRightPanelRole        = Qt::UserRole + 51,

        // For use by subclasses
        SubclassRoles           = Qt::UserRole + 100,
        // For use by filter models
        FilterModelRoles        = Qt::UserRole + 500
    };

public:

    explicit ImageModel(QObject* parent = 0);
    ~ImageModel();

    /** If a cache is kept, lookup by file path is fast,
     *  without a cache it is O(n). Default is false.
     */
    void setKeepsFilePathCache(bool keepCache);
    bool keepsFilePathCache() const;

    /** Set a set of database fields to watch.
     *  If either of these is changed, dataChanged() will be emitted.
     *  Default is no flag (no signal will be emitted).
     */
    void setWatchFlags(const DatabaseFields::Set& set);

    /** Returns the ImageInfo object, reference or image id from the underlying data
     *  pointed to by the index.
     *  If the index is not valid, imageInfo will return a null ImageInfo, imageId will
     *  return 0, imageInfoRef must not be called with an invalid index.
     */
    ImageInfo        imageInfo(const QModelIndex& index) const;
    ImageInfo&       imageInfoRef(const QModelIndex& index) const;
    qlonglong        imageId(const QModelIndex& index) const;
    QList<ImageInfo> imageInfos(const QList<QModelIndex>& indexes) const;
    QList<qlonglong> imageIds(const QList<QModelIndex>& indexes) const;

    /** Returns the ImageInfo object, reference or image id from the underlying data
     *  of the given row (parent is the invalid QModelIndex, column is 0).
     *  Note that imageInfoRef will crash if index is invalid.
     */
    ImageInfo  imageInfo(int row) const;
    ImageInfo& imageInfoRef(int row) const;
    qlonglong  imageId(int row) const;

    /** Return the index for the given ImageInfo or id, if contained in this model.
     */
    QModelIndex        indexForImageInfo(const ImageInfo& info) const;
    QModelIndex        indexForImageInfo(const ImageInfo& info, const QVariant& extraValue) const;
    QModelIndex        indexForImageId(qlonglong id) const;
    QModelIndex        indexForImageId(qlonglong id, const QVariant& extraValue) const;
    QList<QModelIndex> indexesForImageInfo(const ImageInfo& info) const;
    QList<QModelIndex> indexesForImageId(qlonglong id) const;

    int numberOfIndexesForImageInfo(const ImageInfo& info) const;
    int numberOfIndexesForImageId(qlonglong id) const;

    /** Returns the index or ImageInfo object from the underlying data
     *  for the given file path. This is fast if keepsFilePathCache is enabled.
     *  The file path is as returned by ImageInfo.filePath().
     *  In case of multiple occurrences of the same file, the simpler variants return
     *  any one found first, use the QList methods to retrieve all occurrences.
     */
    QModelIndex        indexForPath(const QString& filePath) const;
    ImageInfo          imageInfo(const QString& filePath) const;
    QList<QModelIndex> indexesForPath(const QString& filePath) const;
    QList<ImageInfo>   imageInfos(const QString& filePath) const;

    /** Main entry point for subclasses adding image infos to the model.
     *  If you list entries not unique per image id, you must add an extraValue
     *  so that every entry is unique by imageId and extraValues.
     *  Please note that these methods do not prevent addition of duplicate entries.
     */
    void addImageInfo(const ImageInfo& info);
    void addImageInfos(const QList<ImageInfo>& infos);
    void addImageInfos(const QList<ImageInfo>& infos, const QList<QVariant>& extraValues);

    /** Clears image infos and resets model.
     */
    void clearImageInfos();

    /** Clears and adds the infos.
     */
    void setImageInfos(const QList<ImageInfo>& infos);

    /**
     * Directly remove the given indexes or infos from the model.
     */
    void removeIndex(const QModelIndex& indexes);
    void removeIndexes(const QList<QModelIndex>& indexes);
    void removeImageInfo(const ImageInfo& info);
    void removeImageInfos(const QList<ImageInfo>& infos);
    void removeImageInfos(const QList<ImageInfo>& infos, const QList<QVariant>& extraValues);

    /**
     * addImageInfo() is asynchronous if a prepocessor is set.
     * This method first adds the info, synchronously.
     * Only afterwards, the preprocessor will have the opportunity to process it.
     * This method also bypasses any incremental updates.
     * Please note that these methods do not prevent addition of duplicate entries.
     */
    void addImageInfoSynchronously(const ImageInfo& info);
    void addImageInfosSynchronously(const QList<ImageInfo>& infos);
    void addImageInfosSynchronously(const QList<ImageInfo>& infos, const QList<QVariant>& extraValues);

    /**
     * Add the given entries. Method returns immediately, the
     * addition may happen later asynchronously.
     * These methods prevent the addition of duplicate entries.
     */
    void ensureHasImageInfo(const ImageInfo& info);
    void ensureHasImageInfos(const QList<ImageInfo>& infos);
    void ensureHasImageInfos(const QList<ImageInfo>& infos, const QList<QVariant>& extraValues);

    /**
     * Ensure that all images grouped on the given leader are contained in the model.
     */
    void ensureHasGroupedImages(const ImageInfo& groupLeader);

    QList<ImageInfo> imageInfos() const;
    QList<qlonglong> imageIds()    const;
    QList<ImageInfo> uniqueImageInfos() const;

    bool hasImage(qlonglong id) const;
    bool hasImage(const ImageInfo& info) const;
    bool hasImage(const ImageInfo& info, const QVariant& extraValue) const;
    bool hasImage(qlonglong id, const QVariant& extraValue) const;

    bool isEmpty() const;

    // Drag and Drop
    DECLARE_MODEL_DRAG_DROP_METHODS

    /**
     * Install an object as a preprocessor for ImageInfos added to this model.
     * For every QList of ImageInfos added to addImageInfo, the signal preprocess()
     * will be emitted. The preprocessor may process the items and shall then readd
     * them by calling reAddImageInfos(). It may take some time to process.
     * It shall discard any held infos when the modelReset() signal is sent.
     * It shall call readdFinished() when no reset occurred and all infos on the way have been readded.
     * This means that only after calling this method, you shall make three connections
     * (preprocess -> your slot, your signal -> reAddImageInfos, your signal -> reAddingFinished)
     * and make or already hold a connection modelReset() -> your slot.
     * There is only one preprocessor at a time, a previously set object will be disconnected.
     */
    void setPreprocessor(QObject* processor);
    void unsetPreprocessor(QObject* processor);

    /**
     * Returns true if this model is currently refreshing.
     * For a preprocessor this means that, although the preprocessor may currently have
     * processed all it got, more batches are to be expected.
     */
    bool isRefreshing() const;

    /**
     * Enable sending of imageInfosAboutToBeRemoved and imageInfosRemoved signals.
     * Default: false
     */
    void setSendRemovalSignals(bool send);

    virtual QVariant      data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    virtual QVariant      headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    virtual int           rowCount(const QModelIndex& parent = QModelIndex()) const;
    virtual Qt::ItemFlags flags(const QModelIndex& index) const;
    virtual QModelIndex   index(int row, int column = 0, const QModelIndex& parent = QModelIndex()) const;

    /** Retrieves the imageInfo object from the data() method of the given index.
     *  The index may be from a QSortFilterProxyModel as long as an ImageModel is at the end. */
    static ImageInfo retrieveImageInfo(const QModelIndex& index);
    static qlonglong retrieveImageId(const QModelIndex& index);

Q_SIGNALS:

    /** Informs that ImageInfos will be added to the model.
     *  This signal is sent before the model data is changed and views are informed.
     */
    void imageInfosAboutToBeAdded(const QList<ImageInfo>& infos);

    /** Informs that ImageInfos have been added to the model.
     *  This signal is sent after the model data is changed and views are informed.
     */
    void imageInfosAdded(const QList<ImageInfo>& infos);

    /** Informs that ImageInfos will be removed from the model.
     *  This signal is sent before the model data is changed and views are informed.
     *  Note: You need to explicitly enable sending of this signal. It is not sent
     *  in clearImageInfos().
     */
    void imageInfosAboutToBeRemoved(const QList<ImageInfo>& infos);

    /** Informs that ImageInfos have been removed from the model.
     *  This signal is sent after the model data is changed and views are informed. *
     *  Note: You need to explicitly enable sending of this signal. It is not sent
     *  in clearImageInfos().
     */
    void imageInfosRemoved(const QList<ImageInfo>& infos);

    /** Connect to this signal only if you are the current preprocessor.
     */
    void preprocess(const QList<ImageInfo>& infos, const QList<QVariant>&);
    void processAdded(const QList<ImageInfo>& infos, const QList<QVariant>&);

    /** If an ImageChangeset affected indexes of this model with changes as set in watchFlags(),
     *  this signal contains the changeset and the affected indexes.
     */
    void imageChange(const ImageChangeset&, const QItemSelection&);

    /** If an ImageTagChangeset affected indexes of this model,
     *  this signal contains the changeset and the affected indexes.
     */
    void imageTagChange(const ImageTagChangeset&, const QItemSelection&);

    /** Signals that the model is right now ready to start an incremental refresh.
     *  This is guaranteed only for the scope of emitting this signal.
     */
    void readyForIncrementalRefresh();

    /** Signals that the model has finished currently with all scheduled
     *  refreshing, full or incremental, and all preprocessing.
     *  The model is in polished, clean situation right now.
     */
    void allRefreshingFinished();

public Q_SLOTS:

    void reAddImageInfos(const QList<ImageInfo>& infos, const QList<QVariant>& extraValues);
    void reAddingFinished();

protected:

    /** Subclasses that add ImageInfos in batches shall call startRefresh()
     *  when they start sending batches and finishRefresh() when they have finished.
     *  No incremental refreshes will be started while listing.
     *  A clearImageInfos() always stops listing, calling finishRefresh() is then not necessary.
     */
    void startRefresh();
    void finishRefresh();

    /** As soon as the model is ready to start an incremental refresh, the signal
     *  readyForIncrementalRefresh() will be emitted. The signal will be emitted inline
     *  if the model is ready right now.
     */
    void requestIncrementalRefresh();
    bool hasIncrementalRefreshPending() const;

    /** Starts an incremental refresh operation. You shall only call this method from a slot
     *  connected to readyForIncrementalRefresh(). To initiate an incremental refresh,
     *  call requestIncrementalRefresh().
     */
    void startIncrementalRefresh();
    void finishIncrementalRefresh();

    void emitDataChangedForAll();
    void emitDataChangedForSelection(const QItemSelection& selection);

    // Called when the internal storage is cleared
    virtual void imageInfosCleared() {};

    // Called before rowsAboutToBeRemoved
    virtual void imageInfosAboutToBeRemoved(int /*begin*/, int /*end*/) {};

protected Q_SLOTS:

    virtual void slotImageChange(const ImageChangeset& changeset);
    virtual void slotImageTagChange(const ImageTagChangeset& changeset);

private:

    void appendInfos(const QList<ImageInfo>& infos, const QList<QVariant>& extraValues);
    void appendInfosChecked(const QList<ImageInfo>& infos, const QList<QVariant>& extraValues);
    void publiciseInfos(const QList<ImageInfo>& infos, const QList<QVariant>& extraValues);
    void cleanSituationChecks();
    void removeRowPairsWithCheck(const QList<QPair<int, int> >& toRemove);
    void removeRowPairs(const QList<QPair<int, int> >& toRemove);

public:

    // Declared public because it's used in ImageModelIncrementalUpdater class
    class Private;

private:

    Private* const d;
};

} // namespace Digikam

Q_DECLARE_METATYPE(Digikam::ImageModel*)

#endif // IMAGEMODEL_H
