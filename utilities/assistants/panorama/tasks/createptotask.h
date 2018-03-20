/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-03-15
 * Description : a tool to create panorama by fusion of several images.
 *
 * Copyright (C) 2012-2015 by Benjamin Girault <benjamin dot girault at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef PANO_CREATE_PTO_TASK_H
#define PANO_CREATE_PTO_TASK_H

// Qt includes

#include <QPointer>

// Local includes

#include "panotask.h"
#include "metaengine.h"

namespace Digikam
{

class CreatePtoTask : public PanoTask
{
public:

    explicit CreatePtoTask(const QString& workDirPath,
                           PanoramaFileType fileType,
                           QUrl& ptoUrl,
                           const QList<QUrl>& inputFiles,
                           const PanoramaItemUrlsMap& preProcessedMap,
                           bool addGPlusMetadata,
                           const QString& huginVersion);
    ~CreatePtoTask();

protected:

    void run(ThreadWeaver::JobPointer self, ThreadWeaver::Thread* thread) override;

private:

    QUrl&                            ptoUrl;
    const PanoramaItemUrlsMap* const preProcessedMap;
    const PanoramaFileType           fileType;
    const QList<QUrl>&               inputFiles;
    const bool                       addGPlusMetadata;
    const QString&                   huginVersion;
    MetaEngine                       m_meta;
};

}  // namespace Digikam

#endif // PANO_CREATE_PTO_TASK_H
