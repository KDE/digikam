/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-12-09
 * Description : thumbbar tool tip
 *
 * Copyright (C) 2008-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef THUMBBARTOOLTIP_H
#define THUMBBARTOOLTIP_H

// KDE includes

#include <kglobalsettings.h>

// Local includes

#include "digikam_export.h"
#include "ditemtooltip.h"

namespace Digikam
{

class ThumbBarView;
class ThumbBarItem;
class ThumbBarToolTipPriv;

class DIGIKAM_EXPORT ThumbBarToolTipSettings
{
public:

    ThumbBarToolTipSettings()
    {
        showToolTips   = true;
        showFileName   = true;
        showFileDate   = false;
        showFileSize   = false;
        showImageType  = false;
        showImageDim   = true;
        showPhotoMake  = true;
        showPhotoDate  = true;
        showPhotoFocal = true;
        showPhotoExpo  = true;
        showPhotoMode  = true;
        showPhotoFlash = false;
        showPhotoWB    = false;
        font           = KGlobalSettings::generalFont();
    };

    bool  showToolTips;
    bool  showFileName;
    bool  showFileDate;
    bool  showFileSize;
    bool  showImageType;
    bool  showImageDim;
    bool  showPhotoMake;
    bool  showPhotoDate;
    bool  showPhotoFocal;
    bool  showPhotoExpo;
    bool  showPhotoMode;
    bool  showPhotoFlash;
    bool  showPhotoWB;

    QFont font;
};

// --------------------------------------------------------

class DIGIKAM_EXPORT ThumbBarToolTip : public DItemToolTip
{
public:

    ThumbBarToolTip(ThumbBarView* view);
    virtual ~ThumbBarToolTip();

    void setItem(ThumbBarItem* item);
    ThumbBarItem* item() const;

protected:

    ThumbBarToolTipSettings& toolTipSettings() const;

    virtual QRect   repositionRect();
    virtual QString tipContents();

private:

    ThumbBarToolTipPriv* const d;
};

}  // namespace Digikam

#endif /* THUMBBARTOOLTIP_H */
