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

#ifndef IMAGEHISTORYGRAPH_H
#define IMAGEHISTORYGRAPH_H

// Qt includes

#include <QFlags>
#include <QSharedDataPointer>
#include <QDebug>

// Local includes

#include "imageinfo.h"
#include "historyimageid.h"
#include "digikam_export.h"

namespace Digikam
{

class ImageHistoryGraphData;
class DImageHistory;

class DIGIKAM_DATABASE_EXPORT ImageHistoryGraph
{
public:

    ImageHistoryGraph();
    ImageHistoryGraph(const ImageHistoryGraph& other);
    ~ImageHistoryGraph();

    ImageHistoryGraph& operator=(const ImageHistoryGraph& other);

    bool isNull() const;
    bool isEmpty() const;
    bool isSingleVertex() const;
    /**
     * Returns if the graph contains any edges. Because loops are not allowed,
     * this also means (!isEmpty() && !isSingleVertex()).
     */
    bool hasEdges() const;

    ImageHistoryGraphData& data();
    const ImageHistoryGraphData& data() const;

    enum HistoryLoadingFlag
    {
        /// Load the relation cloud to the graph. Will give all edges, but no further info
        LoadRelationCloud  = 1 << 0,
        /// Will load the DImageHistory of the given subject
        LoadSubjectHistory = 1 << 1,
        /// Will load the DImageHistory of all leave vertices of the graph
        LoadLeavesHistory  = 1 << 2,

        LoadAll            = LoadRelationCloud | LoadSubjectHistory | LoadLeavesHistory
    };
    Q_DECLARE_FLAGS(HistoryLoadingMode, HistoryLoadingFlag)

    enum ProcessingMode
    {
        NoProcessing,
        PrepareForDisplay
    };

    /**
     * Convenience: Reads all available history for the given info from the database
     * and returns the created graph.
     * Depending on mode, the graph will be preparedForDisplay().
     * If no history is recorded and no relations found, a single-vertex graph is returned.
     */
    static ImageHistoryGraph fromInfo(const ImageInfo& info,
                                      HistoryLoadingMode loadingMode = LoadAll,
                                      ProcessingMode processingMode  = PrepareForDisplay);

    /**
     * Add the given history.
     * The optionally given info or id is used as the "current" image of the history.
     * If you read a history from a file's metadata or the database, you shall give the
     * relevant subject.
     */
    void addHistory(const DImageHistory& history, const ImageInfo& historySubject = ImageInfo());
    void addHistory(const DImageHistory& history, const HistoryImageId& historySubject = HistoryImageId());

    /**
     * This is very similar to addHistory. The only difference is that
     * no attempt is made to retrieve an ImageInfo for the historySubjectId.
     * Can be useful in the context of scanning
     */
    void addScannedHistory(const DImageHistory& history, qlonglong historySubjectId);

    /**
     * Add images and their relations from the given pairs.
     * Each pair (a,b) means "a is derived from b".
     */
    void addRelations(const QList<QPair<qlonglong, qlonglong> >& pairs);

    /** Clears this graph. */
    void clear();

    /**
     * Remove edges which provide only duplicate information
     * (performs a transitive reduction).
     * Especially call this when addRelations() was used.
     */
    void reduceEdges();

    /**
     * Returns true if for any entry no ImageInfo could be located.
     */
    bool hasUnresolvedEntries() const;

    /**
     * Remove all vertices from the graph for which no existing ImageInfo
     * could be found in the database
     */
    void dropUnresolvedEntries();

    /**
     * Sort vertex information prioritizing for the given vertex
     */
    void sortForInfo(const ImageInfo& subject);

    /**
     * Combines reduceEdges(), dropOrphans() and sortForInfo()
     */
    void prepareForDisplay(const ImageInfo& subject);

    /**
     * Returns all possible relations between images in this graph,
     * the edges of the transitive closure.
     * The first variant returns (1,2),(3,4),(6,8), the second (1,3,6)(2,4,8).
     */
    QList<QPair<qlonglong, qlonglong> > relationCloud() const;
    QPair<QList<qlonglong>, QList<qlonglong> > relationCloudParallel() const;

    /**
     * Returns image infos / ids from all vertices in this graph
     */
    QList<ImageInfo> allImages() const;
    QList<qlonglong> allImageIds() const;

    /**
     * Returns image infos / ids from all root vertices in this graph,
     * i.e. vertices with no precedent history.
     */
    QList<ImageInfo> rootImages() const;

    /**
     * Returns image infos / ids from all leaf vertices in this graph,
     * i.e. vertices with no subsequent history.
     */
    QList<ImageInfo> leafImages() const;

    /**
     * Attempts at a categorization of all images in the graph
     * into the types defined by HistoryImageId.
     * The type will be invalid if no decision can be made due to conflicting data.
     */
    QHash<ImageInfo, HistoryImageId::Types> categorize() const;

private:

    QSharedDataPointer<ImageHistoryGraphData> d;
};

QDebug DIGIKAM_DATABASE_EXPORT operator<<(QDebug dbg, const ImageHistoryGraph& g);

} // namespace Digikam

Q_DECLARE_OPERATORS_FOR_FLAGS(Digikam::ImageHistoryGraph::HistoryLoadingMode)

#endif // IMAGEHISTORYGRAPH_H
