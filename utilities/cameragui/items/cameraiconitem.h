/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-09-21
 * Description : camera icon view item
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef CAMERAICONITEM_H
#define CAMERAICONITEM_H

// Qt includes

#include <QString>
#include <QImage>

// Local includes

#include "iconitem.h"

namespace Digikam
{

class GPItemInfo;

class CameraIconItem : public IconItem
{
    Q_OBJECT

public:

    CameraIconItem(IconGroupItem* parent, const GPItemInfo& itemInfo, const QImage& thumbnail);
    ~CameraIconItem();

    void       setThumbnail(const QImage& thumbnail);
    /** thumbnail is valid when preview image is loaded from camera */
    bool       hasValidThumbnail() const;

    void       setDownloadName(const QString& downloadName);
    void       setDownloaded(int status);
    bool       isDownloaded() const;

    void       setItemInfo(const GPItemInfo& itemInfo);
    GPItemInfo itemInfo() const;

    /** Lock on/off item (to prevent deletetion by error) */
    void       toggleLock();

    // reimplemented from IconItem
    virtual QRect clickToOpenRect();

protected:

    virtual void paintItem(QPainter* p);

private:

    void calcRect(const QString& itemName, const QString& newName);

private Q_SLOTS:

    void slotProgressTimerDone();

private:

    class CameraIconItemPriv;
    CameraIconItemPriv* const d;
};

}  // namespace Digikam

#endif /* CAMERAICONITEM_H */
