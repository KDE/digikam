/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2014-09-12
 * Description : Export tool utils methods
 *
 * Copyright (C) 2014-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "exportutils.h"

// Qt includes

#include <QByteArray>
#include <QBuffer>
#include <QApplication>
#include <QTime>

namespace Digikam
{

QDir ExportUtils::makeTemporaryDir(const char* prefix)
{
    QString subDir = QString::fromLatin1("digikam-%1-%2").arg(QString::fromUtf8(prefix)).arg(qApp->applicationPid());
    QString path   = QDir(QDir::tempPath()).filePath(subDir);

    if (!QDir().exists(path))
    {
        QDir().mkpath(path);
    }

    return QDir(path);
}

// ------------------------------------------------------------------------------------

void ExportUtils::removeTemporaryDir(const char* prefix)
{
    QString subDir = QString::fromLatin1("digikam-%1-%2").arg(QString::fromUtf8(prefix)).arg(qApp->applicationPid());
    QString path   = QDir(QDir::tempPath()).filePath(subDir);

    if (QDir().exists(path))
    {
        QDir(path).removeRecursively();
    }
}

// ------------------------------------------------------------------------------------

QString ExportUtils::randomString(const int& length)
{
    const QString possibleCharacters(QString::fromLatin1("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"));

    QString randomString;
    qsrand((uint)QTime::currentTime().msec());

    for (int i = 0 ; i < length ; ++i)
    {
        int index      = qrand() % possibleCharacters.length();
        QChar nextChar = possibleCharacters.at(index);
        randomString.append(nextChar);
    }

    return randomString;
}

} // namespace Digikam
