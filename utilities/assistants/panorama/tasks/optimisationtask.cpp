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

#include "optimisationtask.h"

// Qt includes

#include <QFile>

namespace Digikam
{

OptimisationTask::OptimisationTask(const QString& workDirPath, const QUrl& input,
                                   QUrl& autoOptimiserPtoUrl, bool levelHorizon, bool gPano,
                                   const QString& autooptimiserPath)
    : CommandTask(PANO_OPTIMIZE, workDirPath, autooptimiserPath),
      autoOptimiserPtoUrl(autoOptimiserPtoUrl),
      ptoUrl(input),
      levelHorizon(levelHorizon),
      buildGPano(gPano)
{
}

OptimisationTask::~OptimisationTask()
{
}

void OptimisationTask::run(ThreadWeaver::JobPointer, ThreadWeaver::Thread*)
{
    autoOptimiserPtoUrl = tmpDir;
    autoOptimiserPtoUrl.setPath(autoOptimiserPtoUrl.path() + QLatin1String("auto_op_pano.pto"));

    QStringList args;
    args << QLatin1String("-am");

    if (levelHorizon)
        args << QLatin1String("-l");

    if (!buildGPano)
       args << QLatin1String("-s");

    args << QLatin1String("-o");
    args << autoOptimiserPtoUrl.toLocalFile();
    args << ptoUrl.toLocalFile();

    runProcess(args);

    // AutoOptimiser does not return an error code when something went wrong...
    QFile ptoOutput(autoOptimiserPtoUrl.toLocalFile());

    if (!ptoOutput.exists())
    {
        successFlag = false;
        errString = getProcessError();
    }

    printDebug(QLatin1String("autooptimiser"));
}

}  // namespace Digikam
