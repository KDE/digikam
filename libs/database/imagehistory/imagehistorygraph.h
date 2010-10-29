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

    ImageHistoryGraphData &data();
    const ImageHistoryGraphData &data() const;

    /**
     * Add the history of the given ImageInfo.
     * Per default, the history is read from the ImageInfo object.
     */
    void addHistory(const ImageInfo& historySubject);
    void addHistory(const ImageInfo& historySubject, const DImageHistory& history);

    /**
     * Add images and their relations from the given pairs.
     * Each pair (a,b) means "a is derived from b".
     */
    void addRelations(const QList<QPair<qlonglong, qlonglong> >& pairs);

    /**
     * Remove edges which provide only duplicate information
     * (performs a transitive reduction).
     * Especially call this when addRelations() was used.
     */
    void reduceEdges();

    /**
     * Remove all vertices from the graph for which no existing ImageInfo
     * could be found in the database
     */
    void dropOrphans();

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
     * Determines if the two ids refer to the same image.
     * Does not check if such a referred image does exist.
     */
    static bool sameReferredImage(const HistoryImageId& id1, const HistoryImageId& id2);

private:

    QSharedDataPointer<ImageHistoryGraphData> d;
};

QDebug DIGIKAM_DATABASE_EXPORT operator<<(QDebug dbg, const ImageHistoryGraph& g);

} // namespace

#endif // IMAGEHISTORYGRAPHDATA_H

