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

#include "compilemksteptask.h"

// Qt includes

#include <QFileInfo>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

CompileMKStepTask::CompileMKStepTask(const QString& workDirPath, int id, const QUrl& mkUrl,
                                     const QString& nonaPath, const QString& enblendPath,
                                     const QString& makePath, bool preview)
    : CommandTask(preview ? PANO_NONAFILEPREVIEW : PANO_NONAFILE, workDirPath, makePath),
      id(id),
      mkUrl(mkUrl),
      nonaPath(nonaPath),
      enblendPath(enblendPath)
{
}

CompileMKStepTask::~CompileMKStepTask()
{
}

void CompileMKStepTask::run(ThreadWeaver::JobPointer, ThreadWeaver::Thread*)
{
    QFileInfo fi(mkUrl.toLocalFile());

    QString mkFile = fi.completeBaseName() + QString::number(id).rightJustified(4, QChar::fromLatin1('0')) + QLatin1String(".tif");
    QStringList args;
    args << QLatin1String("-f");
    args << mkUrl.toLocalFile();
    args << QString::fromLatin1("ENBLEND='%1'").arg(enblendPath);
    args << QString::fromLatin1("NONA='%1'").arg(nonaPath);
    args << mkFile;

    runProcess(args);

    qCDebug(DIGIKAM_GENERAL_LOG) << "make job command line: " << getCommandLine();

    qCDebug(DIGIKAM_GENERAL_LOG) << "make job output (" << mkFile << "):" << endl << output;
}

}  // namespace Digikam
