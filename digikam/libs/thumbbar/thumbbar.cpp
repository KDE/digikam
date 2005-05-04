/* ============================================================
 * File  : thumbbar.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-11-22
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju

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

#include <qpixmap.h>
#include <qtimer.h>
#include <qpainter.h>
#include <qdict.h>
#include <qpoint.h>
#include <qdatetime.h>

#include <kfileitem.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <kio/previewjob.h>
#include <klocale.h>
#include <kmimetype.h>
#include <kfileitem.h>
#include <kglobal.h>

#include <cmath>

#include "thumbbar.h"

namespace Digikam
{

class ThumbBarViewPriv
{
public:

    ThumbBarViewPriv() {
        firstItem = 0;
        lastItem  = 0;
        currItem  = 0;
        count     = 0;
        itemDict.setAutoDelete(false);
    }
    
    ThumbBarItem *firstItem;
    ThumbBarItem *lastItem;
    ThumbBarItem *currItem;
    int           count;

    QDict<ThumbBarItem> itemDict;
    
    bool          clearing;
    int           margin;
    int           tileSize;
    
    QTimer*          timer;
    ThumbBarToolTip* tip;
};

ThumbBarView::ThumbBarView(QWidget* parent)
    : QScrollView(parent)
{
    d = new ThumbBarViewPriv;
    d->margin   = 5;
    d->tileSize = 64;

    d->tip = new ThumbBarToolTip(this);
    
    d->timer = new QTimer(this);
    connect(d->timer, SIGNAL(timeout()),
            SLOT(slotUpdate()));

    viewport()->setBackgroundMode(Qt::NoBackground);
    setHScrollBarMode(QScrollView::AlwaysOff);
    setFrameStyle(QFrame::NoFrame);
    setFixedWidth(d->tileSize + 2*d->margin
                  + verticalScrollBar()->sizeHint().width());

    viewport()->setMouseTracking(true);
}

ThumbBarView::~ThumbBarView()
{
    clear(false);

    delete d->timer;
    delete d->tip;
    delete d;
}

int ThumbBarView::countItems()
{
    return d->count;
}

void ThumbBarView::clear(bool updateView)
{
    d->clearing = true;

    ThumbBarItem *item = d->firstItem;
    while (item)
    {
        ThumbBarItem *tmp = item->m_next;
        delete item;
        item = tmp;
    }

    d->firstItem = 0;
    d->lastItem  = 0;
    d->count     = 0;
    d->currItem  = 0;
    
    if (updateView)
        slotUpdate();

    d->clearing = false;
}

void ThumbBarView::triggerUpdate()
{
    d->timer->start(0, true);    
}

ThumbBarItem* ThumbBarView::currentItem() const
{
    return d->currItem;    
}

ThumbBarItem* ThumbBarView::firstItem() const
{
    return d->firstItem;    
}

ThumbBarItem* ThumbBarView::lastItem() const
{
    return d->lastItem;
}

ThumbBarItem* ThumbBarView::findItem(const QPoint& pos) const
{
    int y = pos.y();
    for (ThumbBarItem *item = d->firstItem; item; item = item->m_next)
    {
        if (y >= item->m_pos &&
            y <= (item->m_pos+d->tileSize+2*d->margin))
        {
            return item;
        }
    }

    return 0;
}

void ThumbBarView::setSelected(ThumbBarItem* item)
{
    if (d->currItem == item)
        return;

    if (d->currItem)
    {
        ThumbBarItem* item = d->currItem;
        d->currItem = 0;
        item->repaint();
    }

    d->currItem = item;
    if (d->currItem)
    {
        ensureVisible(0,item->m_pos);
        item->repaint();
        emit signalURLSelected(item->url());
    }
}

void ThumbBarView::invalidateThumb(ThumbBarItem* item)
{
    if (!item)
        return;

    if (item->m_pixmap)
    {
        delete item->m_pixmap;
        item->m_pixmap = 0;
    }
    KIO::PreviewJob* job = KIO::filePreview(item->url(),
                                            d->tileSize);
    connect(job, SIGNAL(gotPreview(const KFileItem *, const QPixmap &)),
            SLOT(slotGotPreview(const KFileItem *, const QPixmap &)));
    connect(job, SIGNAL(failed(const KFileItem *)),
            SLOT(slotFailedPreview(const KFileItem *)));
}

void ThumbBarView::viewportPaintEvent(QPaintEvent* e)
{
    QRect er(e->rect());
    int cy = viewportToContents(er.topLeft()).y();
    
    QPixmap bgPix(contentsRect().width(), er.height());
    bgPix.fill(colorGroup().background());

    int ts = d->tileSize+2*d->margin;
    QPixmap tile(visibleWidth(), ts);

    int y1 = (cy/ts)*ts;
    int y2 = ((y1 + er.height())/ts +1)*ts;

    for (ThumbBarItem *item = d->firstItem; item; item = item->m_next)
    {
        if (y1 <= item->m_pos && item->m_pos <= y2)
        {
            if (item == d->currItem)
                tile.fill(colorGroup().highlight());
            else
                tile.fill(colorGroup().background());

            QPainter p(&tile);
            p.setPen(Qt::white);
            p.drawRect(0, 0, tile.width(), tile.height());
            p.end();
            
            if (item->m_pixmap)
            {
                int x = (tile.width() -item->m_pixmap->width())/2;
                int y = (tile.height()-item->m_pixmap->height())/2;
                bitBlt(&tile, x, y, item->m_pixmap);
            }
            
            bitBlt(&bgPix, 0, item->m_pos - cy, &tile);
        }
    }

    bitBlt(viewport(), 0, er.y(), &bgPix);
}

void ThumbBarView::contentsMousePressEvent(QMouseEvent* e)
{
    ThumbBarItem* barItem = 0;
    
    int y = e->pos().y();
    for (ThumbBarItem *item = d->firstItem; item; item = item->m_next)
    {
        if (y >= item->m_pos &&
            y <= (item->m_pos+d->tileSize+2*d->margin))
        {
            barItem = item;
            break;
        }
    }

    if (!barItem || barItem == d->currItem)
        return;

    if (d->currItem)
    {
        ThumbBarItem* item = d->currItem;
        d->currItem = 0;
        item->repaint();
    }

    d->currItem = barItem;
    barItem->repaint();
    
    emit signalURLSelected(barItem->url());
}

void ThumbBarView::insertItem(ThumbBarItem* item)
{
    if (!item) return;

    if (!d->firstItem)
    {
        d->firstItem = item;
        d->lastItem  = item;
        item->m_prev = 0;
        item->m_next = 0;
    }
    else
    {
        d->lastItem->m_next = item;
        item->m_prev = d->lastItem;
        item->m_next = 0;
        d->lastItem = item;

    }

    if (!d->currItem)
    {
        d->currItem = item;
        emit signalURLSelected(item->url());
    }
    
    d->itemDict.insert(item->url().url(), item);
    
    d->count++;
    triggerUpdate();
}

void ThumbBarView::removeItem(ThumbBarItem* item)
{
    if (!item) return;

    d->count--;

    if (item == d->firstItem)
    {
        d->firstItem = d->currItem = d->firstItem->m_next;
        if (d->firstItem)
            d->firstItem->m_prev = 0;
        else
            d->firstItem = d->lastItem = d->currItem = 0;
    }
    else if (item == d->lastItem)
    {
        d->lastItem = d->currItem = d->lastItem->m_prev;
        if ( d->lastItem )
           d->lastItem->m_next = 0;
        else
            d->firstItem = d->lastItem = d->currItem = 0;
    }
    else
    {
        ThumbBarItem *i = item;
        if (i) {
            if (i->m_prev ){
                i->m_prev->m_next = d->currItem = i->m_next;
                }
            if ( i->m_next ){
                i->m_next->m_prev = d->currItem = i->m_prev;
                }
            }
    }

    d->itemDict.remove(item->url().url());
    
    if (!d->clearing)
    {
        triggerUpdate();
    }
}

void ThumbBarView::rearrangeItems()
{
    KURL::List urlList;

    int pos = 0;
    ThumbBarItem *item = d->firstItem;
    while (item)
    {
        item->m_pos = pos;
        pos += d->tileSize + 2*d->margin;
        if (!(item->m_pixmap))
            urlList.append(item->m_url);
        item = item->m_next;
    }

    resizeContents(width(), d->count*(d->tileSize+2*d->margin));
    
    if (!urlList.isEmpty())
    {
        KIO::PreviewJob* job = KIO::filePreview(urlList,
                                                d->tileSize);
        connect(job, SIGNAL(gotPreview(const KFileItem *, const QPixmap &)),
                SLOT(slotGotPreview(const KFileItem *, const QPixmap &)));
        connect(job, SIGNAL(failed(const KFileItem *)),
                SLOT(slotFailedPreview(const KFileItem *)));
    }
}

void ThumbBarView::repaintItem(ThumbBarItem* item)
{
    if (item)
    {
        repaintContents(0, item->m_pos, visibleWidth(),
                        d->tileSize+2*d->margin);
    }
}

void ThumbBarView::slotUpdate()
{
    rearrangeItems();
    viewport()->update();
}

void ThumbBarView::slotGotPreview(const KFileItem *fileItem,
                                  const QPixmap& pix)
{
    ThumbBarItem* item = d->itemDict.find(fileItem->url().url());
    if (!item)
        return;

    if (item->m_pixmap)
    {
        delete item->m_pixmap;
        item->m_pixmap = 0;
    }
    
    item->m_pixmap = new QPixmap(pix);
    item->repaint();
}

void ThumbBarView::slotFailedPreview(const KFileItem* fileItem)
{
    ThumbBarItem* item = d->itemDict.find(fileItem->url().url());
    if (!item)
        return;

    KIconLoader* iconLoader = KApplication::kApplication()->iconLoader();
    QPixmap pix = iconLoader->loadIcon("image", KIcon::NoGroup,
                                       d->tileSize);

    if (item->m_pixmap)
    {
        delete item->m_pixmap;
        item->m_pixmap = 0;
    }
    
    item->m_pixmap = new QPixmap(pix);
    item->repaint();
}

ThumbBarItem::ThumbBarItem(ThumbBarView* view,
                           const KURL& url)
    : m_view(view), m_url(url), m_next(0), m_prev(0),
      m_pixmap(0)
{
    m_view->insertItem(this);
}

ThumbBarItem::~ThumbBarItem()
{
    m_view->removeItem(this);
    if (m_pixmap)
        delete m_pixmap;
}

KURL ThumbBarItem::url() const
{
    return m_url;
}

ThumbBarItem* ThumbBarItem::next() const
{
    return m_next;
}

ThumbBarItem* ThumbBarItem::prev() const
{
    return m_prev;
}

QRect ThumbBarItem::rect() const
{
    return QRect(0, m_pos,
                 m_view->visibleWidth(),
                 m_view->d->tileSize + 2*m_view->d->margin);
}

void ThumbBarItem::repaint()
{
    m_view->repaintItem(this);   
}

ThumbBarToolTip::ThumbBarToolTip(ThumbBarView* parent)
    : QToolTip(parent->viewport()), m_view(parent)
{
    
}

void ThumbBarToolTip::maybeTip(const QPoint& pos)
{
    if ( !parentWidget() || !m_view)
        return;

    ThumbBarItem* item = m_view->findItem( m_view->viewportToContents(pos) );
    if (!item)
        return;

    QRect r(item->rect());
    r = QRect( m_view->contentsToViewport(r.topLeft()),
               r.size() );

    QString cellBeg("<tr><td><nobr><font size=-1>");
    QString cellMid("</font></nobr></td>"
                    "<td><nobr><font size=-1>");
    QString cellEnd("</font></nobr></td></tr>");
    
    QString tipText;
    tipText  = "<table cellspacing=0 cellpadding=0>";
    tipText += cellBeg + i18n("Name:") + cellMid;
    tipText += item->url().filename() + cellEnd;

    tipText += cellBeg + i18n("Type:") + cellMid;
    tipText += KMimeType::findByURL(item->url())->comment() + cellEnd;

    KFileItem fileItem(KFileItem::Unknown, KFileItem::Unknown,
                       item->url());

    QDateTime date;
    date.setTime_t(fileItem.time(KIO::UDS_MODIFICATION_TIME));
    tipText += cellBeg + i18n("Modification Date:") + cellMid +
               KGlobal::locale()->formatDateTime(date, true, true)
               + cellEnd;

    tipText += cellBeg + i18n("Size:") + cellMid;
    tipText += i18n("%1 (%2)")
               .arg(KIO::convertSize(fileItem.size()))
               .arg(KGlobal::locale()->formatNumber(fileItem.size(), 0))
               + cellEnd;

    tipText += "</table>";
    
    tip(r, tipText);
}

}  // NameSpace Digikam

#include "thumbbar.moc"
