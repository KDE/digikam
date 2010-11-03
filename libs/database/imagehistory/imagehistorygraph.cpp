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
#include "imagescanner.h"

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

ImageInfo HistoryVertexProperties::firstImageInfo() const
{
    if (infos.isEmpty())
        return ImageInfo();
    return infos.first();
}

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

bool HistoryVertexProperties::operator==(const HistoryImageId& other) const
{
    if (!uuid.isNull() && uuid == other.m_uuid)
        return true;

    foreach (const HistoryImageId& id, referredImages)
    {
        if (ImageScanner::sameReferredImage(id, other))
        {
            kDebug() << id << "is the same as" << other;
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

QDebug operator<<(QDebug dbg, const HistoryVertexProperties& props)
{
    foreach (const ImageInfo& info, props.infos)
        dbg.space() << info.id();
    return dbg;
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

// -----------------------------------------------------------------------------------------------

HistoryEdgeProperties &HistoryEdgeProperties::operator+=(const FilterAction& action)
{
    actions << action;
    return *this;
}

// -----------------------------------------------------------------------------------------------

HistoryGraph::Vertex
ImageHistoryGraphData::addVertex(const QList<HistoryImageId>& imageIds)
{
    if (imageIds.isEmpty())
        return Vertex();

    Vertex v = addVertex(imageIds.first());

    if (imageIds.size() > 1)
        applyProperties(v, QList<ImageInfo>(), imageIds);

    return v;
}

HistoryGraph::Vertex
ImageHistoryGraphData::addVertex(const HistoryImageId& imageId)
{
    if (!imageId.isValid())
        return Vertex();

    Vertex v;
    QList<ImageInfo> infos;

    // first: find by HistoryImageId
    v = findVertexByProperties(imageId);
    //kDebug() << "Find by h-i-m: found" << !v.isNull();

    if (v.isNull())
    {
        // second: Resolve HistoryImageId, find by ImageInfo
        foreach (qlonglong id, ImageScanner::resolveHistoryImageId(imageId))
        {
            ImageInfo info(id);
            infos << info;
            if (v.isNull())
                v = findVertexByProperties(info);
        }
        //kDebug() << "Find by image id:" << !v.isNull();
    }

    applyProperties(v, infos, QList<HistoryImageId>() << imageId);
    //kDebug() << "Returning vertex" << v << properties(v).referredImages.size();
    return v;
}

HistoryGraph::Vertex ImageHistoryGraphData::addVertex(qlonglong id)
{
    return addVertex(ImageInfo(id));
}

HistoryGraph::Vertex ImageHistoryGraphData::addVertex(const ImageInfo& info)
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

HistoryGraph::Vertex ImageHistoryGraphData::addVertexScanned(qlonglong id)
{
    // short version where we dont read information about id from an ImageInfo
    Vertex v = findVertexByProperties(id);

    applyProperties(v, QList<ImageInfo>() << ImageInfo(id), QList<HistoryImageId>());
    return v;
}

void ImageHistoryGraphData::applyProperties(Vertex& v, const QList<ImageInfo>& infos, const QList<HistoryImageId>& ids)
{
    // if needed, add a new vertex; or retrieve properties to add possibly new entries
    if (v.isNull())
        v = HistoryGraph::addVertex();

    HistoryVertexProperties& props = properties(v);

    // adjust properties
    foreach (const ImageInfo& info, infos)
        props += info;
    foreach (const HistoryImageId& id, ids)
        props += id;
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

bool ImageHistoryGraph::isSingleVertex() const
{
    return d->vertexCount() == 1;
}

ImageHistoryGraphData &ImageHistoryGraph::data()
{
    return *d;
}

const ImageHistoryGraphData &ImageHistoryGraph::data() const
{
    return *d;
}

void ImageHistoryGraph::clear()
{
    *d = HistoryGraph();
}

ImageHistoryGraph ImageHistoryGraph::fromInfo(const ImageInfo& info, ProcessingMode mode)
{
    ImageHistoryGraph graph;
    graph.addHistory(info.imageHistory(), info);
    graph.addRelations(info.relationCloud());

    if (mode == PrepareForDisplay)
        graph.prepareForDisplay(info);

    return graph;
}

void ImageHistoryGraph::addHistory(const DImageHistory& givenHistory, const ImageInfo& historySubject)
{
    addHistory(givenHistory, historySubject.historyImageId());
}

void ImageHistoryGraph::addHistory(const DImageHistory& givenHistory, const HistoryImageId& subjectId)
{
    // append the subject to its history
    DImageHistory history = givenHistory;
    if (subjectId.isValid())
        history << subjectId;
    d->addHistory(history);
}

void ImageHistoryGraph::addScannedHistory(const DImageHistory& history, qlonglong historySubjectId)
{
    d->addHistory(history, historySubjectId);
}

void ImageHistoryGraphData::addHistory(const DImageHistory& history, qlonglong extraCurrent/*=0*/)
{
    if (history.isEmpty())
        return;

    HistoryGraph::Vertex last;
    HistoryEdgeProperties edgeProps;

    foreach (const DImageHistory::Entry& entry, history.entries())
    {
        if (!last.isNull())
        {
            edgeProps += entry.action;
        }

        HistoryGraph::Vertex v;
        if (!entry.referredImages.isEmpty())
        {
            v = addVertex(entry.referredImages);
        }

        if (!v.isNull())
        {
            if (!last.isNull())
            {
                if (v != last)
                {
                    HistoryGraph::Edge e = addEdge(v, last);
                    setProperties(e, edgeProps);
                    edgeProps = HistoryEdgeProperties();
                }
                else
                {
                    kWarning() << "Broken history: Same file referred by different entries. Refusing to add a loop.";
                }
            }
            last = v;
        }
    }

    if (extraCurrent && !last.isNull())
    {
        HistoryGraph::Vertex v = addVertexScanned(extraCurrent);
        if (!v.isNull() && v != last)
        {
            HistoryGraph::Edge e = addEdge(v, last);
            setProperties(e, edgeProps);
        }
    }
}

void ImageHistoryGraph::addRelations(const QList<QPair<qlonglong, qlonglong> >& pairs)
{
    HistoryGraph::Vertex v1, v2;
    typedef QPair<qlonglong, qlonglong> IdPair;
    foreach (const IdPair& pair, pairs)
    {
        if (pair.first < 1 || pair.second < 1)
            continue;
        if (pair.first == pair.second)
        {
            kWarning() << "Broken relations cloud: Refusing to add a loop.";
        }
        v1 = d->addVertex(pair.first);
        v2 = d->addVertex(pair.second);
        //kDebug() << "Adding" << v1 << "->" << v2;
        d->addEdge(v1, v2);
    }
}

class lessThanByProximityToSubject
{
public:
    lessThanByProximityToSubject(const ImageInfo& subject) : subject(subject) {}
    bool operator()(const ImageInfo&a, const ImageInfo& b)
    {
        if (a == b)
            return false;
        // same collection
        if (a.albumId() != b.albumId())
        {
            // same album
            if (a.albumId() == subject.albumId())
                return true;
            if (b.albumId() == subject.albumId())
                return false;

            if (a.albumRootId() != b.albumRootId())
            {
                // different collection
                if (a.albumRootId() == subject.albumRootId())
                    return true;
                if (b.albumRootId() == subject.albumRootId())
                    return false;
            }
        }

        if (a.modDateTime() != b.modDateTime())
            return a.modDateTime() < b.modDateTime();
        if (a.name() != b.name())
            return qAbs(a.name().compare(subject.name())) < qAbs(b.name().compare(subject.name()));
        // last resort
        return a.id() < b.id();
    }
    ImageInfo subject;
};

void ImageHistoryGraph::reduceEdges()
{
    if (d->vertexCount() <= 1)
        return;

    QList<HistoryGraph::Edge> removedEgdes;
    HistoryGraph reduction = d->transitiveReduction(&removedEgdes);

    if (reduction.isEmpty())
        return; // reduction failed, not a DAG

    foreach (const HistoryGraph::Edge& e, removedEgdes)
    {
        if (!d->properties(e).actions.isEmpty())
        {
            // TODO: conflict resolution
            kDebug() << "Conflicting history information: Edge removed by transitiveReduction is not empty.";
        }
    }

    *d = reduction;
}

bool ImageHistoryGraph::hasUnresolvedEntries() const
{
    foreach (const HistoryGraph::Vertex& v, d->vertices())
        if (d->properties(v).infos.isEmpty())
            return true;
    return false;
}

void ImageHistoryGraph::dropUnresolvedEntries()
{
    // Remove nodes which could not be resolved into image infos
    QList<HistoryGraph::Vertex> toRemove;
    foreach (const HistoryGraph::Vertex& v, d->vertices())
    {
        const HistoryVertexProperties &props = d->properties(v);
        if (props.infos.isEmpty())
        {
            foreach (const HistoryGraph::Edge& upperEdge, d->edges(v, HistoryGraph::EdgesToRoot))
            {
                foreach (const HistoryGraph::Edge& lowerEdge, d->edges(v, HistoryGraph::EdgesToLeave))
                {
                    HistoryEdgeProperties combinedProps;
                    combinedProps.actions += d->properties(upperEdge).actions;
                    combinedProps.actions += d->properties(lowerEdge).actions;
                    HistoryGraph::Edge connection = d->addEdge(d->source(lowerEdge), d->target(upperEdge));
                    d->setProperties(connection, combinedProps);
                }
            }
            toRemove << v;
        }
    }
    d->remove(toRemove);
}

void ImageHistoryGraph::sortForInfo(const ImageInfo& subject)
{
    // Remove nodes which could not be resolved into image infos
    QList<HistoryGraph::Vertex> toRemove;
    foreach (const HistoryGraph::Vertex& v, d->vertices())
    {
        HistoryVertexProperties& props = d->properties(v);
        if (props.infos.isEmpty())
            std::sort(props.infos.begin(), props.infos.end(), lessThanByProximityToSubject(subject));
    }
}

void ImageHistoryGraph::prepareForDisplay(const ImageInfo& subject)
{
    reduceEdges();
    dropUnresolvedEntries();
    sortForInfo(subject);
}

QList<QPair<qlonglong, qlonglong> > ImageHistoryGraph::relationCloud() const
{
    QList<QPair<qlonglong, qlonglong> > pairs;
    ImageHistoryGraphData closure = d->transitiveClosure();
    QList<HistoryGraph::VertexPair> edges = closure.edgePairs();
    foreach (const HistoryGraph::VertexPair& edge, edges)
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

QList<ImageInfo> ImageHistoryGraph::allImageInfos() const
{
    QList<ImageInfo> infos;
    foreach (const HistoryGraph::Vertex& v, d->vertices())
        infos << d->properties(v).infos;
    return infos;
}

static QString toString(const HistoryVertexProperties& props)
{
    QString s;
    s = "Ids: ";
    QStringList ids;
    foreach (const ImageInfo& info, props.infos)
        ids << QString::number(info.id());
    if (props.uuid.isEmpty())
    {
        if (ids.size() == 1)
            return QString("Id: ") + ids.first();
        else
            return QString("Ids: (") + ids.join(",") + ")";
    }
    else
    {
        if (ids.size() == 1)
            return QString("Id: ") + ids.first() + " UUID: " + props.uuid.left(6) + "...";
        else
            return QString("Ids: (") + ids.join(",") + ") UUID: " + props.uuid.left(6) + "...";
    }
}

QDebug operator<<(QDebug dbg, const ImageHistoryGraph& g)
{
    if (g.data().isEmpty())
    {
        dbg << "(Empty graph)";
        return dbg;
    }

    QList<HistoryGraph::Vertex> vertices = g.data().topologicalSort();

    if (vertices.isEmpty())
    {
        vertices = g.data().vertices();
        dbg << "Not-a-DAG-Graph with" << vertices.size() << "vertices:" << endl;
    }
    else
    {
        dbg << "Graph with" << vertices.size() << "vertices:" << endl;
    }

    foreach (const HistoryGraph::Vertex& target, vertices)
    {
        QString targetString = toString(g.data().properties(target));

        QStringList sourceVertexTexts;
        foreach (const HistoryGraph::Vertex& source, g.data().adjacentVertices(target, HistoryGraph::InboundEdges))
            sourceVertexTexts << toString(g.data().properties(source));

        if (!sourceVertexTexts.isEmpty())
            dbg.nospace() << QString("{ ") + targetString + " } "
                             "-> { " + sourceVertexTexts.join(" }, { ") + " }" << endl;
        else if (g.data().outDegree(target) == 0)
            dbg.nospace() << QString("Unconnected: { ") + targetString + " }" << endl;
    }
    return dbg;
}

} // namespace


