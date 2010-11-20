/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-05-05
 * Description : Metadata operations on images
 *
 * Copyright (C) 2009 by Marcel Wiesweg <marcel.wiesweg@gmx.de>
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

#ifndef METADATAMANAGER_H
#define METADATAMANAGER_H

// Local includes

#include "imageinfo.h"

namespace Digikam
{

class MetadataHub;
class MetadataHubOnTheRoad;

class MetadataManager : public QObject
{
    Q_OBJECT

public:

    static MetadataManager *instance();

    bool requestShutDown();
    void shutDown();

public Q_SLOTS:

    void assignTag(const ImageInfo& info, int tagID);
    void assignTag(const QList<ImageInfo>& infos, int tagID);
    void assignTags(const ImageInfo& info, const QList<int>& tagIDs);
    void assignTags(const QList<ImageInfo>& infos, const QList<int>& tagIDs);
    void assignTags(const QList<int>& imageIds, const QList<int>& tagIDs);

    void removeTag(const ImageInfo& info, int tagID);
    void removeTag(const QList<ImageInfo>& infos, int tagID);
    void removeTags(const ImageInfo& info, const QList<int>& tagIDs);
    void removeTags(const QList<ImageInfo>& infos, const QList<int>& tagIDs);

    void assignRating(const ImageInfo& infos, int rating);
    void assignRating(const QList<ImageInfo>& infos, int rating);

    void setExifOrientation(const QList<ImageInfo>& infos, int orientation);
    void applyMetadata(const QList<ImageInfo>& infos, const MetadataHub &hub);
    void applyMetadata(const QList<ImageInfo>& infos, const MetadataHubOnTheRoad &hub);

Q_SIGNALS:

    void progressMessageChanged(const QString& descriptionOfAction);
    void progressValueChanged(float percent);
    void progressFinished();

    void orientationChangeFailed(const QStringList& failedFileNames);

public:

    // Declared public due to use by MetadataManagerWorker, MetadataManagerDatabaseWorker, and MetadataManagerFileWorker
    class MetadataManagerPriv;

private:

    friend class MetadataManagerCreator;
    MetadataManager();
    ~MetadataManager();

    MetadataManagerPriv* const d;
};

} // namespace Digikam

#endif /* METADATAMANAGER_H */

