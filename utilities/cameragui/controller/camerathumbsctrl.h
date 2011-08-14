/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-08-03
 * Description : digital camera thumbnails controller
 *
 * Copyright (C) 2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef CAMERATHUMBSCTRL_H
#define CAMERATHUMBSCTRL_H

// Qt includes

#include <QObject>
#include <QString>
#include <QImage>
#include <QPixmap>

// Local includes

#include "camiteminfo.h"

class KFileItem;
class KJob;
class KUrl;

namespace Digikam
{

class CameraController;

typedef QPair<CamItemInfo, QPixmap> CachedItem;

class CameraThumbsCtrl : public QObject
{
    Q_OBJECT

public:

    CameraThumbsCtrl(CameraController* ctrl, QObject* parent);
    ~CameraThumbsCtrl();

    CachedItem getThumbInfo(const CamItemInfo& info) const;

    void setCacheSize(int numberOfItems);
    void clearCache();

Q_SIGNALS:

    void signalThumbInfoReady(const CamItemInfo&);

private Q_SLOTS:

    void slotThumbInfo(const QString&, const QString&, const CamItemInfo&, const QImage&);
    void slotThumbInfoFailed(const QString&, const QString&, const CamItemInfo&);
    void slotGotKDEPreview(const KFileItem&, const QPixmap&);
    void slotFailedKDEPreview(const KFileItem&);

private:

    void startKdePreviewJob(const KUrl&);
    void procressKDEPreview(const KFileItem& item, const QPixmap& pix=QPixmap());

    /// Cache management methods.
    void removeItemFromCache(const KUrl& url);
    void putItemToCache(const KUrl& url, const CamItemInfo&, const QPixmap& thumb);
    bool hasItemFromCache(const KUrl& url) const;
    const CachedItem* retrieveItemFromCache(const KUrl& url) const;

private:

    class CameraThumbsCtrlPriv;
    CameraThumbsCtrlPriv* const d;
};

}  // namespace Digikam

#endif /* CAMERATHUMBSCTRL_H */
