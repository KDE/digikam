/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-09-21
 * Description : camera icon view item 
 * 
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2008 by Gilles Caulier <caulier dot gilles at gmail dot com> 
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

// Qt includes.

#include <qstring.h>
#include <qimage.h>

// Local includes.

#include "iconitem.h"

namespace Digikam
{

class GPItemInfo;
class CameraIconViewItemPriv;

class CameraIconViewItem : public IconItem
{

public:

    CameraIconViewItem(IconGroupItem* parent, const GPItemInfo& itemInfo,
                       const QImage& thumbnail, const QString& downloadName=QString());
    ~CameraIconViewItem();

    void    setThumbnail(const QImage& thumbnail);
    
    void    setDownloadName(const QString& downloadName);
    QString getDownloadName() const;
    void    setDownloaded(int status);

    void    toggleLock();

    GPItemInfo* itemInfo() const;

    // reimplemented from IconItem
    virtual QRect clickToOpenRect();
    
protected:

    virtual void paintItem();
    
private:

    void calcRect(const QString& itemName, const QString& downloadName);

private:

    CameraIconViewItemPriv* d;    
};

}  // namespace Digikam

#endif /* CAMERAICONITEM_H */
