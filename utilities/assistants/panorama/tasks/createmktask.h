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

#ifndef PANO_CREATE_MK_TASK_H
#define PANO_CREATE_MK_TASK_H

// Local includes

#include "commandtask.h"

namespace Digikam
{

class CreateMKTask : public CommandTask
{
public:

    explicit CreateMKTask(const QString& workDirPath,
                          const QUrl& input,
                          QUrl& mkUrl,
                          QUrl& panoUrl,
                          PanoramaFileType fileType,
                          const QString& pto2mkPath,
                          bool preview);
    ~CreateMKTask();

protected:

    void run(ThreadWeaver::JobPointer self, ThreadWeaver::Thread *thread) override;

private:

    const QUrl&            ptoUrl;
    QUrl&                  mkUrl;
    QUrl&                  panoUrl;
    const PanoramaFileType fileType;
};

}  // namespace Digikam

#endif // PANO_CREATE_MK_TASK_H
