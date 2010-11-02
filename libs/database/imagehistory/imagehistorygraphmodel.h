/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-10-27
 * Description : Model to an ImageHistoryGraph
 *
 * Copyright (C) 2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

// KDE includes

// Local includes

#include "imagehistorygraph.h"
#include "digikam_export.h"


namespace Digikam
{

class ImageHistoryGraph;
class ImageInfo;

class DIGIKAM_DATABASE_EXPORT ImageHistoryGraphModel : public QAbstractItemModel
{
    Q_OBJECT

public:

    enum Mode
    {
        PathMode
    };

    ImageHistoryGraphModel(QObject *parent = 0);
    ~ImageHistoryGraphModel();

    /**
     *  Set the history subject and the history graph.
     *  Per default, the subject's history graph is read.
     */
    void setHistory(const ImageInfo& subject, const ImageHistoryGraph& graph = ImageHistoryGraph());

    // QAbstractItemModel implementation
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
    virtual Qt::ItemFlags flags(const QModelIndex& index) const;
    virtual bool hasChildren(const QModelIndex& parent = QModelIndex()) const;
    virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex& index) const;

private:

    class ImageHistoryGraphModelPriv;
    ImageHistoryGraphModelPriv* const d;
};

}

#endif // IMAGEHISTORYGRAPHMODEL_H

