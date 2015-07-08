/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-06-15
 * Description : IO Jobs thread for file system jobs
 *
 * Copyright (C) 2015 by Mohamed Anwer <m dot anwer at gmx dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef IOJOBSTHREAD_H
#define IOJOBSTHREAD_H

// Libkdcraw includes

#include <KDCRAW/RActionThreadBase>

// Local includes

#include "album.h"
#include "iojob.h"
#include "imageinfo.h"

using namespace KDcrawIface;

namespace Digikam
{

class IOJobsThread : public RActionThreadBase
{
    Q_OBJECT

public:

    IOJobsThread(QObject* const parent);
    ~IOJobsThread();

    void copy(const QList<QUrl>& srcFiles, const QUrl destAlbum);
    void move(const QList<QUrl>& srcFiles, const QUrl destAlbum);
    void del(const QList<QUrl>& srcsToDelete, bool useTrash);
    void renameFile(const QUrl& srcToRename, const QUrl& newName);

    bool isRenameThread();
    QUrl oldUrlToRename();

    void cancel();
    bool isCanceled();

    bool hasErrors();
    void setKeepErrors(bool keepErrors);
    bool isKeepingErrors();
    QList<QString>& errorsList();

public Q_SLOTS:

    void oneJobFinished();
    void error(const QString& errString);

Q_SIGNALS:

    void finished();
    void renamed(const QUrl& oldUrl, const QUrl& newURl);

private:

    class Private;
    Private *const d;
};

} // namespace Digikam

#endif // IOJOBSTHREAD_H
