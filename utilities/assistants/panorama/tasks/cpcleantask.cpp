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

#include "cpcleantask.h"

// Qt includes

#include <QFile>

namespace Digikam
{

CpCleanTask::CpCleanTask(const QString& workDirPath, const QUrl& input,
                         QUrl& cpCleanPtoUrl, const QString& cpCleanPath)
    : CommandTask(PANO_CPCLEAN, workDirPath, cpCleanPath),
      cpCleanPtoUrl(cpCleanPtoUrl),
      cpFindPtoUrl(input)
{
}

CpCleanTask::~CpCleanTask()
{
}

void CpCleanTask::run(ThreadWeaver::JobPointer, ThreadWeaver::Thread*)
{
    cpCleanPtoUrl = tmpDir;
    cpCleanPtoUrl.setPath(cpCleanPtoUrl.path()+ QLatin1String("cp_pano_clean.pto"));

    QStringList args;
    args << QLatin1String("-o");
    args << cpCleanPtoUrl.toLocalFile();
    args << cpFindPtoUrl.toLocalFile();

    runProcess(args);

    // CPClean does not return an error code when something went wrong...
    QFile ptoOutput(cpCleanPtoUrl.toLocalFile());
    if (!ptoOutput.exists())
    {
        successFlag = false;
        errString = getProcessError();
    }

    printDebug(QLatin1String("cpclean"));
}

}  // namespace Digikam
