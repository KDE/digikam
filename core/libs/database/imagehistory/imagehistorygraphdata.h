/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-10-23
 * Description : Graph data class for image history
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

#ifndef IMAGE_HISTORY_GRAPH_DATA_H
#define IMAGE_HISTORY_GRAPH_DATA_H

// Qt includes

#include <QSharedData>

// Local includes

#include "filteraction.h"
#include "historyimageid.h"
#include "imagehistorygraph_boost.h"

namespace Digikam
{

class ImageInfo;

class HistoryVertexProperties
{
public:

    /**
     * Every vertex has one associated object of this class
     *
     * All entries in a vertex refer to _identical_ images.
     * There can be multiple referred images in a history entry.
     * Each single HistoryImageId can resolve into none, one,
     * or multiple ImageInfos.
     * So there is no mapping between the two fields here.
     *
     * If an image is created from multiple source images (panorama etc.),
     * there will be one vertex per source image!
     */

    QString               uuid;
    QList<HistoryImageId> referredImages;
    QList<ImageInfo>      infos;

    ImageInfo firstImageInfo() const;

    bool markedAs(HistoryImageId::Type) const;
    bool alwaysMarkedAs(HistoryImageId::Type) const;

    bool operator==(const QString& uuid) const;
    bool operator==(const ImageInfo& info) const;
    bool operator==(qlonglong id) const;
    bool operator==(const HistoryImageId& info) const;

    HistoryVertexProperties& operator+=(const QString& uuid);
    HistoryVertexProperties& operator+=(const ImageInfo& info);
    HistoryVertexProperties& operator+=(const HistoryImageId& info);
};

QDebug operator<<(QDebug dbg, const HistoryVertexProperties& props);
QDebug operator<<(QDebug dbg, const HistoryImageId& id);

// ------------------------------------------------------------------------------

class HistoryEdgeProperties
{
public:

    /**
     * Every edge has one associated object of this class.
     *
     * For two vertices v1, v2 with and edge e, v1 -> v2,
     * describes the actions necessary to create v2 from v2:
     * v1 -> actions[0] -> ... -> actions[n] = v2.
     */

    QList<FilterAction> actions;

    HistoryEdgeProperties& operator+=(const FilterAction& action);
};

typedef Graph<HistoryVertexProperties, HistoryEdgeProperties> HistoryGraph;

// ------------------------------------------------------------------------------

class ImageHistoryGraphData : public HistoryGraph, public QSharedData
{
public:

    ImageHistoryGraphData() : HistoryGraph(ChildToParent)
    {
    }

    ImageHistoryGraphData(const HistoryGraph& g) : HistoryGraph(g)
    {
    }

    ImageHistoryGraphData& operator=(const HistoryGraph& g)
    {
        HistoryGraph::operator=(g);
        return *this;
    }

    Vertex addVertex(const HistoryImageId& id);
    ImageHistoryGraphData::Vertex addVertex(const QList<HistoryImageId>& imageIds);
    Vertex addVertex(qlonglong id);
    Vertex addVertexScanned(qlonglong id);
    Vertex addVertex(const ImageInfo& info);

    void addHistory(const DImageHistory& givenHistory, qlonglong extraCurrent = 0);

    int removeNextUnresolvedVertex(int begin);

    inline QList<ImageInfo> toInfoList(const QList<Vertex>& vertices) const
    {
        QList<ImageInfo> infos;
        foreach(const HistoryGraph::Vertex& v, vertices)
        {
            infos << properties(v).infos;
        }
        return infos;
    }

    QHash<Vertex, HistoryImageId::Types> categorize() const;

protected:

    void applyProperties(Vertex& v, const QList<ImageInfo>& infos, const QList<HistoryImageId>& ids);
};

} // namespace Digikam

#endif // IMAGE_HISTORY_GRAPH_DATA_H
