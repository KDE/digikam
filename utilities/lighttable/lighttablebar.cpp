/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at gmail dot com>
 * Date   : 2007-04-11
 * Description : light table thumbs bar
 *
 * Copyright 2007 by Gilles Caulier
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

// Local includes.

#include "imageinfo.h"
#include "lighttablebar.h"
#include "lighttablebar.moc"

namespace Digikam
{

LightTableBar::LightTableBar(QWidget* parent, int orientation, bool exifRotate)
             : ThumbBarView(parent, orientation, exifRotate)
{
    connect(this, SIGNAL(signalItemSelected(ThumbBarItem*)),
            this, SLOT(slotItemSelected(ThumbBarItem*)));    
}

LightTableBar::~LightTableBar()
{
}

void LightTableBar::slotItemSelected(ThumbBarItem* i)
{
    LightTableBarItem *item = static_cast<LightTableBarItem*>(i);
    emit signalLightTableBarItemSelected(item->info());
}

ImageInfo* LightTableBar::currentItemImageInfo() const
{
    LightTableBarItem *item = static_cast<LightTableBarItem*>(currentItem());
    return item->info();
}

// -------------------------------------------------------------------------

LightTableBarItem::LightTableBarItem(LightTableBar *view, ImageInfo *info)
              : ThumbBarItem(view, info->kurl())
{
    m_info = info;
}

LightTableBarItem::~LightTableBarItem()
{
}

ImageInfo* LightTableBarItem::info()
{
    return m_info;
}

}  // NameSpace Digikam

