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

#ifndef IMAGEHISTORYGRAPH_H
#define IMAGEHISTORYGRAPH_H

// Qt includes

#include <QSharedDataPointer>

// KDE includes

// Local includes

#include "dimagehistory.h"
#include "digikam_export.h"
#include "imageinfo.h"

class QDebug;

namespace Digikam
{

class DImageHistory;
class ImageHistoryGraphData;
class ImageInfo;

class DIGIKAM_DATABASE_EXPORT ImageHistoryGraph
{
public:

    ImageHistoryGraph();
    ImageHistoryGraph(const ImageHistoryGraph& other);
    ~ImageHistoryGraph();

    ImageHistoryGraph &operator=(const ImageHistoryGraph& other);

    bool isNull() const;
    bool isEmpty() const;
    bool isSingleVertex() const;

    ImageHistoryGraphData &data();
    const ImageHistoryGraphData &data() const;

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
    static ImageHistoryGraph fromInfo(const ImageInfo& info, ProcessingMode mode = PrepareForDisplay);

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
     * Sort vertex information priorizing for the given vertex
     */
    void sortForInfo(const ImageInfo& subject);

    /**
     * Combines reduceEdges(), dropOrphans() and sortForInfo()
     */
    void prepareForDisplay(const ImageInfo& subject);

    /**
     * Returns all possible relations between images in this graph,
     * the edges of the transitive closure.
     */
    QList<QPair<qlonglong, qlonglong> > relationCloud() const;

    /**
     * Returns all image infos from all vertices in this graph
     */
    QList<ImageInfo> allImageInfos() const;

private:

    QSharedDataPointer<ImageHistoryGraphData> d;
};

QDebug DIGIKAM_DATABASE_EXPORT operator<<(QDebug dbg, const ImageHistoryGraph& g);

} // namespace

#endif // IMAGEHISTORYGRAPHDATA_H

