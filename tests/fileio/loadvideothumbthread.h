/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date  : 2016-04-21
 * @brief : a class to manage video thumbnails extraction using threads
 *
 * @author Copyright (C) 2016 by Gilles Caulier
 *         <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a>
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

#ifndef LOADVIDEOTHUMBTHREAD_H
#define LOADVIDEOTHUMBTHREAD_H

// Qt includes

#include <QUrl>

// Local includes

#include "actionthreadbase.h"

using namespace Digikam;

class LoadVideoThumbThread : public ActionThreadBase
{
    Q_OBJECT

public:

    LoadVideoThumbThread(QObject* const parent);
    ~LoadVideoThumbThread();

    void extractVideoThumb(const QList<QUrl>& list);

Q_SIGNALS:

    void starting(const QUrl& url);
    void finished(const QUrl& url);
    void failed(const QUrl& url, const QString& err);
    void progress(const QUrl& url, int percent);

private Q_SLOTS:

    void slotJobDone();
    void slotJobProgress(int);
    void slotJobStarted();
};

#endif /* LOADVIDEOTHUMBTHREAD_H */
