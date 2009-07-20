/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-03-05
 * Description : Qt item model for database entries
 *
 * Copyright (C) 2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "imageinfo.h"
#include "digikam_export.h"

class QItemSelection;

namespace Digikam
{

class ImageChangeset;
class ImageTagChangeset;
namespace DatabaseFields { class Set; }
class ImageModelDragDropHandler;
class ImageModelPriv;

class DIGIKAM_DATABASE_EXPORT ImageModel : public QAbstractListModel
{
    Q_OBJECT

public:

    enum ImageModelRoles
    {
        /// An ImageModel* pointer to this model
        ImageModelPointerRole = Qt::UserRole,
        ImageModelInternalId  = Qt::UserRole + 1,
        /// Returns a thumbnail pixmap. May be implemented by subclasses.
        /// Returns either a valid pixmap or a null QVariant.
        ThumbnailRole         = Qt::UserRole + 2,
        /// Returns a QDateTime with the creation date
        CreationDateRole      = Qt::UserRole + 3,
        // For use by subclasses
        SubclassRoles         = Qt::UserRole + 100,
        // For use by filter models
        FilterModelRoles      = Qt::UserRole + 500
    };

    ImageModel(QObject *parent = 0);
    ~ImageModel();

    /** If a cache is kept, lookup by file path is fast,
     *  without a cache it is O(n). Default is false. */
    void setKeepsFilePathCache(bool keepCache);
    bool keepsFilePathCache() const;

    /** Set a set of database fields to watch.
     *  If either of these is changed, dataChanged() will be emitted.
     *  Default is no flag (no signal will be emitted) */
    void setWatchFlags(const DatabaseFields::Set& set);

    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
    virtual Qt::ItemFlags flags(const QModelIndex& index) const;
    virtual QModelIndex index(int row, int column = 0, const QModelIndex& parent = QModelIndex()) const;

    /** Returns the ImageInfo object, reference or image id from the underlying data
     *  pointed to by the index.
     *  If the index is not valid, imageInfo will return a null ImageInfo, imageId will
     *  return 0, imageInfoRef must not be called with an invalid index. */
    ImageInfo imageInfo(const QModelIndex& index) const;
    ImageInfo& imageInfoRef(const QModelIndex& index) const;
    qlonglong imageId(const QModelIndex& index) const;
    QList<ImageInfo> imageInfos(const QList<QModelIndex>& indexes) const;
    QList<qlonglong> imageIds(const QList<QModelIndex>& indexes) const;
    /** Returns the ImageInfo object, reference or image id from the underlying data
     *  of the given row (parent is the invalid QModelIndex, column is 0).
     *  Note that imageInfoRef will crash if index is invalid. */
    ImageInfo imageInfo(int row) const;
    ImageInfo& imageInfoRef(int row) const;
    qlonglong imageId(int row) const;
    /** Return the index for the given ImageInfo or id, if contained in this model. */
    QModelIndex indexForImageInfo(const ImageInfo& info) const;
    QModelIndex indexForImageId(qlonglong id) const;
    /** Returns the index or ImageInfo object from the underlying data
     *  for the given file path. This is fast if keepsFilePathCache is enabled.
     *  The file path is as returned by ImageInfo.filePath(). */
    QModelIndex indexForPath(const QString& filePath) const;
    ImageInfo imageInfo(const QString& filePath) const;

    /** Retrieves the imageInfo object from the data() method of the given index.
     *  The index may be from a QSortFilterProxyModel as long as an ImageModel is at the end. */
    static ImageInfo retrieveImageInfo(const QModelIndex& index);
    static qlonglong retrieveImageId(const QModelIndex& index);

    /** Main entry point for subclasses adding image infos to the model. */
    void addImageInfos(const QList<ImageInfo>& infos);
    /** Clears image infos and resets model. */
    void clearImageInfos();

    QList<ImageInfo> imageInfos() const;
    QList<qlonglong> imageIds()    const;

    bool hasImage(qlonglong id) const;
    bool hasImage(const ImageInfo& info) const;

    bool isEmpty() const;

    // Drag and Drop
    virtual Qt::DropActions supportedDropActions() const;
    virtual QStringList mimeTypes() const;
    virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex& parent);
    virtual QMimeData * mimeData(const QModelIndexList& indexes) const;

    /// Set a drag drop handler.
    void setDragDropHandler(ImageModelDragDropHandler *handler);
    /// Returns the drag drop handler, or 0 if none is installed
    ImageModelDragDropHandler *dragDropHandler() const;

    /** Switch on drag and drop globally for all items. Default is true.
     *  For per-item cases reimplement flags(). */
    void setEnableDrag(bool enable);
    void setEnableDrop(bool enable);

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
    void setPreprocessor(QObject *processor);
    void unsetPreprocessor(QObject *processor);

Q_SIGNALS:

    /** Informs that ImageInfos will be added to the model.
     *  This signal is sent before the model data is changed and views are informed. */
    void imageInfosAboutToBeAdded(const QList<ImageInfo>& infos);
    /** Informs that ImageInfos have been added to the model.
     *  This signal is sent after the model data is changed and views are informed. */
    void imageInfosAdded(const QList<ImageInfo>& infos);

    /** Connect to this signal only if you are the current preprocessor */
    void preprocess(const QList<ImageInfo>& infos);

    /** If an ImageChangeset affected indexes of this model with changes as set in watchFlags(),
     *  this signal contains the changeset and the affected indexes. */
    void imageChange(const ImageChangeset &, const QItemSelection &);
    /** If an ImageTagChangeset affected indexes of this model,
     *  this signal contains the changeset and the affected indexes. */
    void imageTagChange(const ImageTagChangeset &, const QItemSelection &);

    /** Signals that the model is right now ready to start an incremental refresh.
     *  This is guaranteed only for the scope of emitting this signal. */
    void readyForIncrementalRefresh();

public Q_SLOTS:

    void reAddImageInfos(const QList<ImageInfo>& infos);
    void reAddingFinished();

protected:

    /** As soon as the model is ready to start an incremental refresh, the signal
     *  readyForIncrementalRefresh() will be emitted. The signal will be emitted inline
     *  if the model is ready right now. */
    void requestIncrementalRefresh();
    bool hasIncrementalRefreshPending() const;
    /** Starts an incremental refresh operation. You shall only call this method from a slot
     *  connected to readyForIncrementalRefresh(). To initiate an incremental refresh,
     *  call requestIncrementalRefresh() */
    void startIncrementalRefresh();
    void finishIncrementalRefresh();

    // Called when the internal storage is cleared
    virtual void imageInfosCleared() {};

    void emitDataChangedForAll();
    void emitDataChangedForSelection(const QItemSelection& selection);

protected Q_SLOTS:

    virtual void slotImageChange(const ImageChangeset& changeset);
    virtual void slotImageTagChange(const ImageTagChangeset& changeset);

private:

    void appendInfos(const QList<ImageInfo>& infos);
    void publiciseInfos(const QList<ImageInfo>& infos);

    ImageModelPriv *const d;
};

} // namespace Digikam

Q_DECLARE_METATYPE(Digikam::ImageModel*)

#endif // IMAGEMODEL_H
