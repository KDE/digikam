/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-10-27
 * Description : Model to an ImageHistoryGraph
 *
 * Copyright (C) 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef IMAGEHISTORYGRAPHMODEL_H
#define IMAGEHISTORYGRAPHMODEL_H

// Qt includes

#include <QAbstractItemModel>

// Local includes

#include "dragdropimplementations.h"
#include "imagehistorygraph.h"
#include "digikam_export.h"

namespace Digikam
{

class ImageHistoryGraph;
class ImageInfo;
class ImageListModel;
class FilterAction;

class DIGIKAM_DATABASE_EXPORT ImageHistoryGraphModel : public QAbstractItemModel, public DragDropModelImplementation
{
    Q_OBJECT

public:

    enum Mode
    {
        ImagesListMode,
        ImagesTreeMode,
        CombinedTreeMode
    };

    enum ExtraRoles
    {
        IsImageItemRole        = Qt::UserRole + 1000,
        IsFilterActionItemRole = Qt::UserRole + 1001,
        IsHeaderItemRole       = Qt::UserRole + 1002,
        IsCategoryItemRole     = Qt::UserRole + 1003,
        IsSeparatorItemRole    = Qt::UserRole + 1004,

        IsSubjectImageRole     = Qt::UserRole + 1010,

        FilterActionRole       = Qt::UserRole + 1020
    };

public:

    explicit ImageHistoryGraphModel(QObject* const parent = 0);
    ~ImageHistoryGraphModel();

    void setMode(Mode mode);
    Mode mode() const;

    /**
     *  Set the history subject and the history graph.
     *  Per default, the subject's history graph is read.
     */
    void setHistory(const ImageInfo& subject, const ImageHistoryGraph& graph = ImageHistoryGraph());

    ImageInfo subject() const;

    bool isImage(const QModelIndex& index) const;
    bool hasImage(const ImageInfo& info);
    ImageInfo imageInfo(const QModelIndex& index) const;
    /// Note: There may be multiple indexes for an info. The index found first is returned.
    QModelIndex indexForInfo(const ImageInfo& info) const;

    bool isFilterAction(const QModelIndex& index) const;
    FilterAction filterAction(const QModelIndex& index) const;

    // QAbstractItemModel implementation
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    virtual int rowCount(const QModelIndex& parent = QModelIndex())                                   const;
    virtual int columnCount(const QModelIndex& parent = QModelIndex())                                const;
    virtual Qt::ItemFlags flags(const QModelIndex& index)                                             const;
    virtual bool hasChildren(const QModelIndex& parent = QModelIndex())                               const;
    virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex())         const;
    virtual QModelIndex parent(const QModelIndex& index)                                              const;
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole)                       const;
    virtual bool setData(const QModelIndex& index, const QVariant& value, int role);

    DECLARE_MODEL_DRAG_DROP_METHODS

    /**
     * Returns an internal image model used for entries representing images.
     * Note: Set a thumbnail thread on this model if you need thumbnails.
     */
    ImageListModel* imageModel() const;
    /**
     * If the given index is represented by the internal image model,
     * return the image model's index.
     * Otherwise an invalid index is returned.
     */
    QModelIndex imageModelIndex(const QModelIndex& index) const;

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // IMAGEHISTORYGRAPHMODEL_H
