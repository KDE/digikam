/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
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

#ifndef DIGIKAM_COPY_FILES_TASK_H
#define DIGIKAM_COPY_FILES_TASK_H

// Qt includes

#include <QPointer>

// Local includes

#include "panotask.h"
#include "metaengine.h"

using namespace Digikam;

namespace DigikamGenericPanoramaPlugin
{

class CopyFilesTask : public PanoTask
{
public:

    explicit CopyFilesTask(const QString& workDirPath,
                           const QUrl& panoUrl,
                           const QUrl& finalPanoUrl,
                           const QUrl& ptoUrl,
                           const PanoramaItemUrlsMap& urls,
                           bool sPTO,
                           bool GPlusMetadata);
    ~CopyFilesTask();

protected:

    void run(ThreadWeaver::JobPointer self, ThreadWeaver::Thread *thread) override;

private:

    const QUrl&                      panoUrl;
    const QUrl                       finalPanoUrl;
    const QUrl&                      ptoUrl;
    const PanoramaItemUrlsMap* const urlList;
    const bool                       savePTO;
    const bool                       addGPlusMetadata;

    MetaEngine                       m_meta;
};

} // namespace DigikamGenericPanoramaPlugin

#endif // DIGIKAM_COPY_FILES_TASK_H
