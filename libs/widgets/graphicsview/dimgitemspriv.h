/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-04-30
 * Description : Graphics View item for DImg
 *
 * Copyright (C) 2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef DIMGITEMSPRIV_H
#define DIMGITEMSPRIV_H

// Qt includes

#include <QString>

// Local includes

#include "digikam_export.h"
#include "dimg.h"
#include "dimgpreviewitem.h"
#include "imagezoomsettings.h"

namespace Digikam
{

class DIGIKAM_EXPORT GraphicsDImgItemPrivate
{
public:

    GraphicsDImgItemPrivate()
    {
    }

    void init(GraphicsDImgItem *q);

    DImg image;
    ImageZoomSettings zoomSettings;
};

class PreviewLoadThread;
class DImgPreviewItem;

class DIGIKAM_EXPORT DImgPreviewItemPrivate : public GraphicsDImgItemPrivate
{
public:

    DImgPreviewItemPrivate();
    void init(DImgPreviewItem *q);

    DImgPreviewItem::State state;
    bool               exifRotate;
    int                previewSize;

    QString            path;
    bool               loadFullImageSize;
    PreviewLoadThread *previewThread;
    PreviewLoadThread *preloadThread;
    QStringList        pathsToPreload;
};

}

#endif



