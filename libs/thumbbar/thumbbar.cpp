/* ============================================================
 * File  : thumbbar.cpp
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2004-11-22
 * Description : a bar widget to display image thumbnails
 * 
 * Copyright 2004-2005 by Renchi Raju and Gilles Caulier
 * Copyrigth 2005-2006 by Tom Albers <tomalbers@kde.nl>
 * Copyright 2006 by Gilles Caulier
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

// C Ansi includes.

extern "C"
{
#include <unistd.h>
}

// C++ includes.

#include <cmath>

// Qt includes. 

#include <qdir.h>
#include <qpixmap.h>
#include <qtimer.h>
#include <qpainter.h>
#include <qdict.h>
#include <qpoint.h>
#include <qdatetime.h>
#include <qguardedptr.h>

// KDE includes.

#include <kmdcodec.h>
#include <kfileitem.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <kio/previewjob.h>
#include <klocale.h>
#include <kmimetype.h>
#include <kfileitem.h>
#include <kglobal.h>

// Local includes.

#include "thumbbar.h"
#include "thumbnailjob.h"

namespace Digikam
{

class ThumbBarViewPriv
{
public:

    ThumbBarViewPriv()
    {
        exifRotate = false;
        firstItem  = 0;
        lastItem   = 0;
        currItem   = 0;
        count      = 0;
        thumbJob   = 0;
        itemDict.setAutoDelete(false);
    }
    
    bool                      clearing;
    bool                      exifRotate;

    int                       count;
    int                       margin;
    int                       tileSize;
    int                       orientation;
    
    QTimer                   *timer;

    ThumbBarItem             *firstItem;
    ThumbBarItem             *lastItem;
    ThumbBarItem             *currItem;

    ThumbBarToolTip          *tip;
    
    QDict<ThumbBarItem>       itemDict;
    QGuardedPtr<ThumbnailJob> thumbJob;
};

ThumbBarView::ThumbBarView(QWidget* parent, int orientation, bool exifRotate)
            : QScrollView(parent)
{
    d = new ThumbBarViewPriv;
    d->margin      = 5;
    d->tileSize    = 64;
    d->orientation = orientation;
    d->exifRotate  = exifRotate;

    d->tip   = new ThumbBarToolTip(this);
    d->timer = new QTimer(this);
    
    connect(d->timer, SIGNAL(timeout()),
            this, SLOT(slotUpdate()));

    viewport()->setBackgroundMode(Qt::NoBackground);
    viewport()->setMouseTracking(true);
    setFrameStyle(QFrame::NoFrame);
    
    if (d->orientation == Vertical)
    {
       setHScrollBarMode(QScrollView::AlwaysOff);
       setFixedWidth(d->tileSize + 2*d->margin
                     + verticalScrollBar()->sizeHint().width());
    }
    else
    {
       setVScrollBarMode(QScrollView::AlwaysOff);
       setFixedHeight(d->tileSize + 2*d->margin
                      + horizontalScrollBar()->sizeHint().height());
    }
}

ThumbBarView::~ThumbBarView()
{
    if (!d->thumbJob.isNull())
    {
        d->thumbJob->kill();
        d->thumbJob = 0;
    }
    
    clear(false);
        
    delete d->timer;
    delete d->tip;
    delete d;
}

void ThumbBarView::setExifRotate(bool exifRotate)
{
    d->exifRotate = exifRotate;
    QString thumbCacheDir = QDir::homeDirPath() + "/.thumbnails/";

    for (ThumbBarItem *item = d->firstItem; item; item = item->m_next)
    {
        // Remove all current album item thumbs from disk cache.

        QString uri = "file://" + QDir::cleanDirPath(item->url().path(-1));
        KMD5 md5(QFile::encodeName(uri));
        uri = md5.hexDigest();
    
        QString smallThumbPath = thumbCacheDir + "normal/" + uri + ".png";
        QString bigThumbPath   = thumbCacheDir + "large/"  + uri + ".png";

        ::unlink(QFile::encodeName(smallThumbPath));
        ::unlink(QFile::encodeName(bigThumbPath));

        invalidateThumb(item);
    }
    
    triggerUpdate();
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
    int itemPos;
    
    if (d->orientation == Vertical)
        itemPos = pos.y();
    else
        itemPos = pos.x();
    
    for (ThumbBarItem *item = d->firstItem; item; item = item->m_next)
    {
        if (itemPos >= item->m_pos && itemPos <= (item->m_pos+d->tileSize+2*d->margin))
        {
            return item;
        }
    }
    
    return 0;
}

ThumbBarItem* ThumbBarView::findItemByURL(const KURL& url) const
{
    for (ThumbBarItem *item = d->firstItem; item; item = item->m_next)
    {
        if (item->url().equals(url))
        {
            return item;
        }
    }

    return 0;
}

void ThumbBarView::setSelected(ThumbBarItem* item)
{
    if (!item)
        return;
        
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
        // We want the complete thumb visible and the next one.
        // find the middle of the image and give a margin of 1,5 image
        // When changed, watch regression for bug 104031
        if (d->orientation == Vertical)
            ensureVisible(0, (int)(item->m_pos+d->margin+d->tileSize*.5),
                          0, (int)(d->tileSize*1.5+3*d->margin));
        else
            ensureVisible((int)(item->m_pos+d->margin+d->tileSize*.5), 0,
                          (int)(d->tileSize*1.5+3*d->margin), 0);
          
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
    
    if (!d->thumbJob.isNull())
    {
       d->thumbJob->kill();
       d->thumbJob = 0;
    }
       
    d->thumbJob = new ThumbnailJob(item->url(), d->tileSize, true, d->exifRotate);
    
    connect(d->thumbJob, SIGNAL(signalThumbnail(const KURL&, const QPixmap&)),
            this, SLOT(slotGotThumbnail(const KURL&, const QPixmap&)));
   
    connect(d->thumbJob, SIGNAL(signalFailed(const KURL&)),
            this, SLOT(slotFailedThumbnail(const KURL&)));     
}

void ThumbBarView::viewportPaintEvent(QPaintEvent* e)
{
    int cy, cx, ts, y1, y2, x1, x2;
    QPixmap bgPix, tile;
    QRect er(e->rect());
    
    if (d->orientation == Vertical)
    {
       cy = viewportToContents(er.topLeft()).y();
        
       bgPix.resize(contentsRect().width(), er.height());
    
       ts = d->tileSize + 2*d->margin;
       tile.resize(visibleWidth(), ts);
    
       y1 = (cy/ts)*ts;
       y2 = ((y1 + er.height())/ts +1)*ts;
    }
    else
    {
       cx = viewportToContents(er.topLeft()).x();
        
       bgPix.resize(er.width(), contentsRect().height());
    
       ts = d->tileSize + 2*d->margin;
       tile.resize(ts, visibleHeight());
    
       x1 = (cx/ts)*ts;
       x2 = ((x1 + er.width())/ts +1)*ts;
    }
    
    bgPix.fill(colorGroup().background());
    
    for (ThumbBarItem *item = d->firstItem; item; item = item->m_next)
    {
        if (d->orientation == Vertical)
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
        else
        {
            if (x1 <= item->m_pos && item->m_pos <= x2)
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
                
                bitBlt(&bgPix, item->m_pos - cx, 0, &tile);
            }
        }
    }

    if (d->orientation == Vertical)
       bitBlt(viewport(), 0, er.y(), &bgPix);
    else
       bitBlt(viewport(), er.x(), 0, &bgPix);
}

void ThumbBarView::contentsMousePressEvent(QMouseEvent* e)
{
    ThumbBarItem* barItem = 0;
    
    if (d->orientation == Vertical)
    {
       int y = e->pos().y();
       
       for (ThumbBarItem *item = d->firstItem; item; item = item->m_next)
       {
           if (y >= item->m_pos &&
               y <= (item->m_pos + d->tileSize + 2*d->margin))
           {
                barItem = item;
                break;
           }
       }
    }
    else
    {
       int x = e->pos().x();
       
       for (ThumbBarItem *item = d->firstItem; item; item = item->m_next)
       {
           if (x >= item->m_pos &&
               x <= (item->m_pos + d->tileSize + 2*d->margin))
           {
                barItem = item;
                break;
           }
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
    emit signalItemAdded();
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
        if (i)
        {
            if (i->m_prev )
            {
                i->m_prev->m_next = d->currItem = i->m_next;
            }
            if ( i->m_next )
            {
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

    if (d->orientation == Vertical)
       resizeContents(width(), d->count*(d->tileSize+2*d->margin));
    else    
       resizeContents(d->count*(d->tileSize+2*d->margin), height());
       
    if (!urlList.isEmpty())
    {
        if (!d->thumbJob.isNull())
        {
           d->thumbJob->kill();
           d->thumbJob = 0;
        }

        d->thumbJob = new ThumbnailJob(urlList, d->tileSize, true, d->exifRotate);
        
        connect(d->thumbJob, SIGNAL(signalThumbnail(const KURL&, const QPixmap&)),
                this, SLOT(slotGotThumbnail(const KURL&, const QPixmap&)));
    
        connect(d->thumbJob, SIGNAL(signalFailed(const KURL&)),
                this, SLOT(slotFailedThumbnail(const KURL&)));     
    }
}

void ThumbBarView::repaintItem(ThumbBarItem* item)
{
    if (item)
    {
       if (d->orientation == Vertical)
           repaintContents(0, item->m_pos, visibleWidth(), d->tileSize+2*d->margin);
       else
           repaintContents(item->m_pos, 0, d->tileSize+2*d->margin, visibleHeight());
    }
}

void ThumbBarView::slotUpdate()
{
    rearrangeItems();
    viewport()->update();
}

void ThumbBarView::slotGotThumbnail(const KURL& url, const QPixmap& pix)
{
    if (!pix.isNull())
    {
        ThumbBarItem* item = d->itemDict.find(url.url());
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
}

void ThumbBarView::slotFailedThumbnail(const KURL& url)
{
    KIO::PreviewJob* job = KIO::filePreview(url, d->tileSize, 0, 0, 70, true, false);
    
    connect(job, SIGNAL(gotPreview(const KFileItem *, const QPixmap &)),
            this, SLOT(slotGotPreview(const KFileItem *, const QPixmap &)));

    connect(job, SIGNAL(failed(const KFileItem *)),
            this, SLOT(slotFailedPreview(const KFileItem *)));
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

// -------------------------------------------------------------------------

ThumbBarItem::ThumbBarItem(ThumbBarView* view, const KURL& url)
            : m_pos(0), m_pixmap(0), m_url(url), m_next(0), m_prev(0), m_view(view)
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
    if (m_view->d->orientation == ThumbBarView::Vertical)
    {
        return QRect(0, m_pos,
                     m_view->visibleWidth(),
                     m_view->d->tileSize + 2*m_view->d->margin);
    }
    else
    {
        return QRect(m_pos, 0,
                     m_view->d->tileSize + 2*m_view->d->margin,
                     m_view->visibleHeight());
    }
}

void ThumbBarItem::repaint()
{
    m_view->repaintItem(this);   
}

// -------------------------------------------------------------------------

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
    r = QRect( m_view->contentsToViewport(r.topLeft()), r.size() );

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

    KFileItem fileItem(KFileItem::Unknown, KFileItem::Unknown, item->url());

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
