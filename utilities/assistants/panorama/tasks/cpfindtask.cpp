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

#include "cpfindtask.h"

// Qt includes

#include <QFile>

namespace Digikam
{

CpFindTask::CpFindTask(const QString& workDirPath, const QUrl& input,
                       QUrl& cpFindUrl, bool celeste, const QString& cpFindPath)
    : CommandTask(PANO_CPFIND, workDirPath, cpFindPath),
      cpFindPtoUrl(cpFindUrl),
      celeste(celeste),
      ptoUrl(input)
{
}

CpFindTask::~CpFindTask()
{
}

void CpFindTask::run(ThreadWeaver::JobPointer, ThreadWeaver::Thread*)
{
    // Run CPFind to get control points and order the images
    cpFindPtoUrl = tmpDir;
    cpFindPtoUrl.setPath(cpFindPtoUrl.path() + QLatin1String("cp_pano.pto"));

    QStringList args;
    if (celeste)
        args << QLatin1String("--celeste");
    args << QLatin1String("-o");
    args << cpFindPtoUrl.toLocalFile();
    args << ptoUrl.toLocalFile();

    runProcess(args);

    // CPFind does not return an error code when something went wrong...
    QFile ptoOutput(cpFindPtoUrl.toLocalFile());
    if (!ptoOutput.exists())
    {
        successFlag = false;
        errString = getProcessError();
    }

    printDebug(QLatin1String("cpfind"));
}

}  // namespace Digikam
