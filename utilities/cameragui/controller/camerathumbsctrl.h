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

// Local includes

#include "camiteminfo.h"

class KFileItem;
class KJob;

namespace Digikam
{

class CameraController;

class CameraThumbsCtrl : public QObject
{
    Q_OBJECT

public:

    CameraThumbsCtrl(CameraController* ctrl, QObject* parent);
    ~CameraThumbsCtrl();

    void getThumbsInfo(const CamItemInfoList& list);

Q_SIGNALS:

    void signalInfo(const QString& folder, const QString& file, const CamItemInfo&);
    void signalThumb(const QString& folder, const QString& file, const QImage&);
    void signalThumbInfo(const CamItemInfo&, const QImage&);

private Q_SLOTS:

    void slotThumbInfo(const QString&, const QString&, const CamItemInfo&, const QImage&);
    void slotThumbInfoFailed(const QString&, const QString&, const CamItemInfo&);
    void slotGotKDEPreview(const KFileItem&, const QPixmap&);
    void slotFailedKDEPreview(const KFileItem&);
    void slotKdePreviewFinished(KJob*);

private:

    void startKdePreviewJob();

private:

    class CameraThumbsCtrlPriv;
    CameraThumbsCtrlPriv* const d;
};

}  // namespace Digikam

#endif /* CAMERATHUMBSCTRL_H */
