/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-12-10
 * Description : album icon view tool tip
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

#ifndef ALBUMICONVIEWTOOLTIP_H
#define ALBUMICONVIEWTOOLTIP_H

// Local includes.

#include "imageinfo.h"
#include "ditemtooltip.h"

namespace Digikam
{

class AlbumIconView;
class AlbumIconItem;
class AlbumIconViewToolTipPriv;

class AlbumIconViewToolTip : public DItemToolTip
{
public:

    AlbumIconViewToolTip(AlbumIconView* view);
    ~AlbumIconViewToolTip();

    void setIconItem(AlbumIconItem* iconItem);

    static QString fillTipContents(const ImageInfo& info);

private:

    QRect   repositionRect();
    QString tipContents();

private:

    AlbumIconViewToolTipPriv* const d;
};

}  // namespace Digikam

#endif /* ALBUMICONVIEWTOOLTIP_H */
