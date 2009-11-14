/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-12-17
 * Description : camera icon view tool tip
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

#ifndef CAMERAICONVIEWTOOLTIP_H
#define CAMERAICONVIEWTOOLTIP_H

// Local includes

#include "gpiteminfo.h"
#include "ditemtooltip.h"

namespace Digikam
{

class CameraIconView;
class CameraIconItem;
class CameraIconViewToolTipPriv;

class CameraIconViewToolTip : public DItemToolTip
{
public:

    CameraIconViewToolTip(CameraIconView* view);
    ~CameraIconViewToolTip();

    void setIconItem(CameraIconItem* iconItem);

    QString fillTipContents(GPItemInfo* info);

private:

    QRect   repositionRect();
    QString tipContents();

private:

    CameraIconViewToolTipPriv* const d;
};

}  // namespace Digikam

#endif /* CAMERAICONVIEWTOOLTIP_H */
