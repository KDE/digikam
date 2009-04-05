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

namespace Digikam
{

class ImageModelPriv;

class DIGIKAM_DATABASE_EXPORT ImageModel : public QAbstractListModel
{
    Q_OBJECT

public:

    ImageModel(QObject *parent = 0);
    ~ImageModel();

    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual QModelIndex index(int row, int column = 0, const QModelIndex &parent = QModelIndex()) const;

    /** Returns the ImageInfo object, reference or image id from the underlying data
     *  pointed to by the index. */
    ImageInfo imageInfo(const QModelIndex &index) const;
    ImageInfo &imageInfoRef(const QModelIndex &index) const;
    qlonglong imageId(const QModelIndex &index) const;
    /** Returns the ImageInfo object, reference or image id from the underlying data
     *  of the given row (parent is the invalid QModelIndex, column is 0).
     *  Note that imageInfoRef will crash if index is invalid. */
    ImageInfo imageInfo(int row) const;
    ImageInfo &imageInfoRef(int row) const;
    qlonglong imageId(int row) const;

    void addImageInfos(const QList<ImageInfo> &infos);
    void clearImageInfos();

    QList<ImageInfo> imageInfos() const;
    QSet<qlonglong> imageIds()    const;

    bool hasImage(qlonglong id) const;
    bool hasImage(const ImageInfo &info) const;

    bool isEmpty() const;

    /**
     * Install an object as a preprocessor for ImageInfos added to this model.
     * For every QList of ImageInfos added to addImageInfo, the signal preprocess()
     * will be emitted. The preprocessor may process the items and shall then readd
     * them by calling reAddImageInfos(). It may take some time to process.
     * It shall discard any held infos when the modelReset() signal is sent.
     * This means that only after calling this method, you shall make the two connections,
     * and have a third connection to modelReset().
     * There is only one preprocessor at a time, a previously set object will be disconnected.
     */
    void setPreprocessor(QObject *processor);
    void unsetPreprocessor(QObject *processor);

    /*
    virtual Qt::DropActions supportedDropActions() const;
    virtual QStringList mimeTypes() const;
    virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);
    virtual QMimeData * mimeData(const QModelIndexList &indexes) const;
    */

Q_SIGNALS:

    /** Informs that ImageInfos will be added to the model.
     *  This signal is sent before the model data is changed and views are informed. */
    void imageInfosAboutToBeAdded(const QList<ImageInfo> &infos);
    /** Informs that ImageInfos have been added to the model.
     *  This signal is sent after the model data is changed and views are informed. */
    void imageInfosAdded(const QList<ImageInfo> &infos);

    /** Connect to this signal only if you are the current preprocessor */
    void preprocess(const QList<ImageInfo> &infos);

public Q_SLOTS:

    void reAddImageInfos(const QList<ImageInfo> &infos);

protected:

protected Q_SLOTS:

    void appendInfos(const QList<ImageInfo> &infos);

private:

    ImageModelPriv *const d;
};

} // namespace Digikam

#endif // IMAGEMODEL_H
