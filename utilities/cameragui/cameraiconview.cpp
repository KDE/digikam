/* ============================================================
 * File  : cameraiconview.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-07-13
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju

 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published bythe Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * ============================================================ */

#include <kio/scheduler.h>
#include <kio/slave.h>

#include <kdirlister.h>
#include <kurl.h>
#include <kio/previewjob.h>
#include <kdebug.h>

#include <qtimer.h>
#include <qpixmap.h>
#include <qpainter.h>

#include "thumbitem.h"
#include "camerathumbjob.h"
#include "cameraiconitem.h"
#include "cameraiconview.h"

class CameraIconViewPriv
{
public:

};
    
CameraIconView::CameraIconView(QWidget* parent)
    : ThumbView(parent)
{
    setWFlags(Qt::WDestructiveClose);

    d = new CameraIconViewPriv;
}

CameraIconView::~CameraIconView()
{
    delete d;
}

CameraIconItem* CameraIconView::firstSelectedItem()
{
    CameraIconItem *iconItem = 0;
    for (ThumbItem *item = firstItem(); item;
         item = item->nextItem())
    {
        if (item->isSelected())
         {
             iconItem = static_cast<CameraIconItem*>(item);
             break;
         }
    }

    return iconItem;
}

void CameraIconView::slotNewItems(const KFileItemList& itemList)
{
    int w = 110;
    QPixmap pix(w, w);
    pix.fill(colorGroup().base());
    QPainter p(&pix);
    p.fillRect(0, 0, w, w, QBrush(colorGroup().base()));
    p.setPen(Qt::black);
    p.drawRect(0, 0, w, w);
    p.end();

    KFileItem* item;

    for (KFileItemListIterator it(itemList); (item = it.current()); ++it)
    {
        if (item->isDir())
            continue;

        /*ThumbItem* iconItem = new ThumbItem(this, item->url().filename(),
          pix); */
        CameraIconItem *iconItem = new CameraIconItem(this, item, pix);
        item->setExtraData(this, iconItem);
    }
}

void CameraIconView::slotDeleteItem(KFileItem *item)
{
    ThumbItem* iconItem = (ThumbItem*) item->extraData(this);
    if (!iconItem)
        return;

    delete iconItem;
    item->removeExtraData(this);
}

void CameraIconView::slotClear()
{
    clear();
}

void CameraIconView::slotGotThumbnail(const KFileItem* item, const QPixmap& pix)
{
    CameraIconItem* iconItem = (CameraIconItem*) item->extraData(this);
    if (!iconItem)
        return;

    iconItem->setPixmap(pix);
}

void CameraIconView::slotFailedThumbnail(const KFileItem* item)
{
    kdDebug() << "Failed thumbnail " << item->url().url() << endl;
}

#include "cameraiconview.moc"
