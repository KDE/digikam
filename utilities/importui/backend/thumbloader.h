/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2014-02-13
 * Description : Thumbnail loader with with QThread support
 *
 * Copyright (C) 2014 by Veaceslav Munteanu <veaceslav dot munteanu90  at gmail dot com>
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

#ifndef THUMBLOADER_H
#define THUMBLOADER_H

#include<QWidget>
#include <QHash>

#include <kurl.h>
#include "camiteminfo.h"

class QMutex;

namespace Digikam
{
class CameraController;
class DKCamera;

class PixWorker : public QObject
{
    Q_OBJECT
public:
    PixWorker(QHash<QString, QImage>* localPix,
              QMutex* localPixLock);

    void setDKCamera(DKCamera* cam);

public slots:
    void doWork(CamItemInfo camInfo, int thSize);

signals:
    void resultReady(const KUrl result);
private:
    class PixWorkerPriv;
    PixWorkerPriv* d;
};

class ThumbLoader : public QObject
{
    Q_OBJECT
public:
    ThumbLoader(QObject* const parent,
                QString title, QString model,
                QString port, QString path);
    ~ThumbLoader();

    QImage getThumbnail(CamItemInfo camInfo, int thSize);
    void addToWork(CamItemInfo camInfo, int thSize);
    void setDKCamera(CameraController* cam);

private slots:
    void handleResult(KUrl result);

signals:
    void signalStartWork(CamItemInfo camInfo, int thSize);
    void signalUpdateModel(KUrl item);
private:
    class ThumbLoaderPriv;
    ThumbLoaderPriv* d;
};

} // namespace Digikam

#endif
