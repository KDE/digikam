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

#ifndef PRE_PROCESS_TASK_H
#define PRE_PROCESS_TASK_H

// Qt includes

#include <QPointer>

// Local includes

#include "panotask.h"
#include "dmetadata.h"

namespace Digikam
{

class PreProcessTask : public PanoTask
{
public:

    const int id;

public:

    explicit PreProcessTask(const QString& workDirPath,
                            int id,
                            PanoramaPreprocessedUrls& targetUrls,
                            const QUrl& sourceUrl);
    ~PreProcessTask();

    void requestAbort() override;

protected:

    void run(ThreadWeaver::JobPointer self, ThreadWeaver::Thread* thread) override;

private:

    bool computePreview(const QUrl& inUrl);
    bool convertRaw();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // PRE_PROCESS_TASK_H
