/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-10-23
 * Description : Graph data class for image history
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

#include "imagehistorygraph.h"

// Qt includes

// KDE includes

#include <kdebug.h>
#include <kglobal.h>

// Local includes

#include "dimagehistory.h"
#include "imagehistorygraphdata.h"

namespace Digikam
{

// -----------------------------------------------------------------------------------------------

class ImageHistoryGraphDataSharedNull : public QSharedDataPointer<ImageHistoryGraphData>
{
public:

    ImageHistoryGraphDataSharedNull() : QSharedDataPointer<ImageHistoryGraphData>(new ImageHistoryGraphData) {}
};

K_GLOBAL_STATIC(ImageHistoryGraphDataSharedNull, imageHistoryGraphDataSharedNull)

// -----------------------------------------------------------------------------------------------

bool HistoryVertexProperties::operator==(const QString& id) const
{
    return uuid == id;
}

bool HistoryVertexProperties::operator==(const ImageInfo& info) const
{
    return infos.contains(info);
}

bool HistoryVertexProperties::operator==(qlonglong id) const
{
    foreach (const ImageInfo& info, infos)
        if (info.id() == id)
            return true;
    return false;
}

QDebug operator<<(QDebug dbg, const HistoryImageId& id)
{
    dbg.nospace() << " { ";
    dbg.nospace() << id.m_uuid;
    dbg.space() << id.m_type;
    dbg.space() << id.m_fileName;
    dbg.space() << id.m_filePath;
    dbg.space() << id.m_creationDate;
    dbg.space() << id.m_uniqueHash;
    dbg.space() << id.m_fileSize;
    dbg.space() << id.m_originalUUID;
    dbg.nospace() << " } ";
    return dbg;
}

bool HistoryVertexProperties::operator==(const HistoryImageId& other) const
{
    if (!uuid.isNull() && uuid == other.m_uuid)
        return true;

    foreach (const HistoryImageId& id, referredImages)
    {
        if (ImageHistoryGraph::sameReferredImage(id, other))
        {
            kDebug() << id << endl << other;
            return true;
        }
    }

    return false;
}

HistoryVertexProperties &HistoryVertexProperties::operator+=(const QString& id)
{
    if (!uuid.isNull() && id.isNull())
        uuid = id;
    return *this;
}

HistoryVertexProperties &HistoryVertexProperties::operator+=(const ImageInfo& info)
{
    if (!info.isNull() && !infos.contains(info))
    {
        infos << info;
        if (uuid.isNull())
            uuid = info.uuid();
    }
    return *this;
}

HistoryVertexProperties &HistoryVertexProperties::operator+=(const HistoryImageId& id)
{
    if (id.isValid() && !referredImages.contains(id))
    {
        referredImages << id;
        if (uuid.isNull())
            uuid = id.m_uuid;
    }
    return *this;
}

// -----------------------------------------------------------------------------------------------

HistoryEdgeProperties &HistoryEdgeProperties::operator+=(const FilterAction& action)
{
    actions << action;
    return *this;
}

// -----------------------------------------------------------------------------------------------

ImageHistoryGraphData::Vertex
ImageHistoryGraphData::addVertex(const QList<HistoryImageId>& imageIds, ImageHistoryIdResolver *resolver)
{
    if (imageIds.isEmpty())
        return Vertex();

    Vertex v = addVertex(imageIds.first(), resolver);

    if (imageIds.size() > 1)
        applyProperties(v, QList<ImageInfo>(), imageIds);

    return v;
}

ImageHistoryGraphData::Vertex
ImageHistoryGraphData::addVertex(const HistoryImageId& imageId, ImageHistoryIdResolver *resolver)
{
    if (!imageId.isValid())
        return Vertex();

    Vertex v;
    QList<ImageInfo> infos;

    // first: find by HistoryImageId
    v = findVertexByProperties(imageId);

    if (v.isNull())
    {
        // second: Resolve HistoryImageId, find by ImageInfo
        foreach (qlonglong id, resolver->resolveHistoryImageId(imageId))
        {
            ImageInfo info(id);
            infos << info;
            if (v.isNull())
                v = findVertexByProperties(info);
        }
    }

    applyProperties(v, infos, QList<HistoryImageId>() << imageId);
    return v;
}

ImageHistoryGraphData::Vertex ImageHistoryGraphData::addVertex(qlonglong id)
{
    return addVertex(ImageInfo(id));
}

ImageHistoryGraphData::Vertex ImageHistoryGraphData::addVertex(const ImageInfo& info)
{
    Vertex v;
    QString uuid;
    HistoryImageId id;

    v = findVertexByProperties(info);
    //kDebug() << "Find by id" << info.id() << ": found" << v.isNull();
    if (v.isNull())
    {
        uuid = info.uuid();
        if (!uuid.isNull())
            v = findVertexByProperties(uuid);
        //kDebug() << "Find by uuid" << uuid << ": found" << v.isNull();
        if (v.isNull())
        {
            HistoryImageId id = info.historyImageId();
            v = findVertexByProperties(id);
            //kDebug() << "Find by h-i-m" << ": found" << v.isNull();
        }
    }

    applyProperties(v, QList<ImageInfo>() << info, QList<HistoryImageId>() << id);
    //kDebug() << "Returning vertex" << v << properties(v).infos.size();
    return v;
}

void ImageHistoryGraphData::applyProperties(Vertex& v, const QList<ImageInfo>& infos, const QList<HistoryImageId>& ids)
{
    // if needed, add a new vertex; or retrieve properties to add possibly new entries
    HistoryVertexProperties props;
    if (v.isNull())
        v = Graph::addVertex();
    else
        props = properties(v);

    // adjust properties
    foreach (const ImageInfo& info, infos)
        props += info;
    foreach (const HistoryImageId& id, ids)
        props += id;
    setProperties(v, props);
}

// -----------------------------------------------------------------------------------------------

ImageHistoryGraph::ImageHistoryGraph()
    : d(*imageHistoryGraphDataSharedNull)
{
}

ImageHistoryGraph::ImageHistoryGraph(const ImageHistoryGraph& other)
    : d(other.d)
{
}

ImageHistoryGraph::~ImageHistoryGraph()
{
}

ImageHistoryGraph &ImageHistoryGraph::operator=(const ImageHistoryGraph& other)
{
    d = other.d;
    return *this;
}

bool ImageHistoryGraph::isNull() const
{
    return d == *imageHistoryGraphDataSharedNull;
}

bool ImageHistoryGraph::isEmpty() const
{
    return d->vertexCount() == 0;
}

ImageHistoryGraphData &ImageHistoryGraph::data()
{
    return *d;
}

const ImageHistoryGraphData &ImageHistoryGraph::data() const
{
    return *d;
}

void ImageHistoryGraph::addHistory(const ImageInfo& historySubject, ImageHistoryIdResolver *resolver)
{
    addHistory(historySubject, historySubject.imageHistory(), resolver);
}


void ImageHistoryGraph::addHistory(const ImageInfo& historySubject, const DImageHistory& history, ImageHistoryIdResolver *resolver)
{
    if (history.isEmpty())
        return;

    // append the subject to its history
    DImageHistory historyWithCurrent = history;
    historyWithCurrent << historySubject.historyImageId();

    ImageHistoryGraphData::Vertex v, last;
    HistoryEdgeProperties edgeProps;

    foreach (const DImageHistory::Entry& entry, history.entries())
    {
        if (!last.isNull())
        {
            edgeProps += entry.action;
        }

        if (!entry.referredImages.isEmpty())
        {
            v = d->addVertex(entry.referredImages, resolver);
        }

        if (!v.isNull())
        {
            if (!last.isNull())
            {
                ImageHistoryGraphData::Edge e = d->addEdge(v, last);
                d->setProperties(e, edgeProps);
                edgeProps = HistoryEdgeProperties();
            }
            last = v;
        }
    }
}

void ImageHistoryGraph::addRelations(const QList<QPair<qlonglong, qlonglong> >& pairs)
{
    ImageHistoryGraphData::Vertex v1, v2;
    typedef QPair<qlonglong, qlonglong> IdPair;
    foreach (const IdPair& pair, pairs)
    {
        v1 = d->addVertex(pair.first);
        v2 = d->addVertex(pair.second);
        //kDebug() << "Adding" << v1 << "->" << v2;
        d->addEdge(v1, v2);
    }
}

void ImageHistoryGraph::finish()
{
    QList<ImageHistoryGraphData::Edge> removedEgdes;
    ImageHistoryGraphData::Graph reduction = d->transitiveReduction(&removedEgdes);

    foreach (const ImageHistoryGraphData::Edge& e, removedEgdes)
    {
        if (!d->properties(e).actions.isEmpty())
        {
            // TODO: conflict resolution
            kDebug() << "Conflicting history information: Edge removed by transitiveReduction is not empty.";
        }
    }

    *d = reduction;
}

QList<QPair<qlonglong, qlonglong> > ImageHistoryGraph::relationCloud() const
{
    QList<QPair<qlonglong, qlonglong> > pairs;
    ImageHistoryGraphData closure = d->transitiveClosure();
    QList<ImageHistoryGraphData::VertexPair> edges = closure.edges();
    foreach (const ImageHistoryGraphData::VertexPair& edge, edges)
    {
        foreach (const ImageInfo& source, closure.properties(edge.first).infos)
        {
            foreach (const ImageInfo& target, closure.properties(edge.second).infos)
            {
                pairs << QPair<qlonglong, qlonglong>(source.id(), target.id());
            }
        }
    }
    return pairs;
}

bool ImageHistoryGraph::sameReferredImage(const HistoryImageId& id1, const HistoryImageId& id2)
{
    if (!id1.isValid() || !id2.isValid())
        return false;

    if (!id1.m_uuid.isNull() && id1.m_uuid == id2.m_uuid)
        return true;

    if (!id1.m_uniqueHash.isNull() && id1.m_uniqueHash == id2.m_uniqueHash
        && id1.m_fileSize == id2.m_fileSize)
        return true;

    if (!id1.m_fileName.isNull() && id1.m_fileName == id2.m_fileName
        && !id1.m_creationDate.isNull() && id1.m_creationDate == id2.m_creationDate)
        return true;

    if (!id1.m_filePath.isNull() && id1.m_filePath == id2.m_filePath)
        return true;

    return false;
}

QDebug operator<<(QDebug dbg, const ImageHistoryGraph& g)
{
    QList<ImageHistoryGraphData::Vertex> vertices = g.data().topologicalSort();

    dbg << endl;
    foreach (const ImageHistoryGraphData::Vertex& target, vertices)
    {
        QList<qlonglong> sourceIds, targetIds;
        foreach (const ImageInfo& info, g.data().properties(target).infos)
            targetIds << info.id();
        foreach (const ImageHistoryGraphData::Vertex& source,
                 g.data().adjacentVertices(target, ImageHistoryGraphData::InboundEdges))
        {
            foreach (const ImageInfo& info, g.data().properties(source).infos)
                sourceIds << info.id();
        }
        if (!targetIds.isEmpty())
            dbg << targetIds << "->" << sourceIds << endl;
        else if (g.data().outDegree(target) == 0)
            dbg << "Unconnected:" << targetIds << endl;
    }
    return dbg;
}

} // namespace


