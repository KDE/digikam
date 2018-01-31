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

#include "compilemktask.h"

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

CompileMKTask::CompileMKTask(const QString& workDirPath,
                             const QUrl& mkUrl, const QUrl& /*panoUrl*/,
                             const QString& nonaPath, const QString& enblendPath,
                             const QString& makePath, bool preview)
    : CommandTask(preview ? PANO_STITCHPREVIEW : PANO_STITCH, workDirPath, makePath),
      /*panoUrl(&panoUrl),*/
      mkUrl(mkUrl),
      nonaPath(nonaPath),
      enblendPath(enblendPath)
{
}

CompileMKTask::~CompileMKTask()
{
}

void CompileMKTask::run(ThreadWeaver::JobPointer, ThreadWeaver::Thread*)
{
    QStringList args;
    args << QLatin1String("-f");
    args << mkUrl.toLocalFile();
    args << QString::fromLatin1("ENBLEND='%1'").arg(enblendPath);
    args << QString::fromLatin1("NONA='%1'").arg(nonaPath);

    runProcess(args);

    qCDebug(DIGIKAM_GENERAL_LOG) << "make command line: " << getCommandLine();

    qCDebug(DIGIKAM_GENERAL_LOG) << "make output:" << endl << output;
}

}  // namespace Digikam
