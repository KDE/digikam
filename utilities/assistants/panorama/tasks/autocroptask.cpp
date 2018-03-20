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

#include "autocroptask.h"

// Qt includes

#include <QFile>

namespace Digikam
{

AutoCropTask::AutoCropTask(const QString& workDirPath,
                           const QUrl& autoOptimiserPtoUrl, QUrl& viewCropPtoUrl,
                           bool /*buildGPano*/, const QString& panoModifyPath)
    : CommandTask(PANO_AUTOCROP, workDirPath, panoModifyPath),
      autoOptimiserPtoUrl(autoOptimiserPtoUrl),
      viewCropPtoUrl(viewCropPtoUrl)/*,
      buildGPano(buildGPano),*/
{
}

AutoCropTask::~AutoCropTask()
{
}

void AutoCropTask::run(ThreadWeaver::JobPointer, ThreadWeaver::Thread*)
{
    viewCropPtoUrl = tmpDir;
    viewCropPtoUrl.setPath(viewCropPtoUrl.path() + QLatin1String("view_crop_pano.pto"));

    QStringList args;
    args << QLatin1String("-c");               // Center the panorama
    args << QLatin1String("-s");               // Straighten the panorama
    args << QLatin1String("--canvas=AUTO");    // Automatic size
    args << QLatin1String("--crop=AUTO");      // Automatic crop
    args << QLatin1String("-o");
    args << viewCropPtoUrl.toLocalFile();
    args << autoOptimiserPtoUrl.toLocalFile();

    runProcess(args);

    // PanoModify does not return an error code when something went wrong...
    QFile ptoOutput(viewCropPtoUrl.toLocalFile());
    if (!ptoOutput.exists())
    {
        successFlag = false;
        errString = getProcessError();
    }

    printDebug(QLatin1String("pano_modify"));
}

}  // namespace Digikam
