/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-03-15
 * Description : a tool to create panorama by fusion of several images.
 *
 * Copyright (C) 2012-2016 by Benjamin Girault <benjamin dot girault at gmail dot com>
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

#include "panotask.h"

// Qt includes

#include <QFileInfo>

namespace Digikam
{

PanoTask::PanoTask(PanoAction action, const QString& workDirPath)
    : action(action),
      isAbortedFlag(false),
      successFlag(false),
      tmpDir(QUrl::fromLocalFile(workDirPath + QLatin1String("/")))
{
}

PanoTask::~PanoTask()
{
}

bool PanoTask::success() const
{
    return successFlag;
}

void PanoTask::requestAbort()
{
    isAbortedFlag = true;
}

}  // namespace Digikam
