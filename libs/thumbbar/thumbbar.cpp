/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-11-22
 * Description : a bar widget to display image thumbnails
 * 
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2005-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include <qimage.h>
#include <qtimer.h>
#include <qpainter.h>
#include <qdict.h>
#include <qpoint.h>
#include <qstylesheet.h>
#include <qdatetime.h>
#include <qguardedptr.h>

// KDE includes.

#include <kmdcodec.h>
#include <kfileitem.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmimetype.h>
#include <kfileitem.h>
#include <kglobal.h>

// LibKDcraw includes.

#include <libkdcraw/version.h>
#include <libkdcraw/kdcraw.h>

#if KDCRAW_VERSION < 0x000106
#include <libkdcraw/dcrawbinary.h>
#endif

// Local includes.

#include "dmetadata.h"
#include "thumbnailjob.h"
#include "thumbnailsize.h"
#include "thumbbar.h"
#include "thumbbar.moc"

namespace Digikam
{

class ThumbBarViewPriv
{
public:

    ThumbBarViewPriv() :
        margin(5)
    {
        dragging   = false;
        exifRotate = false;
        clearing   = false;
        toolTip    = 0;
        firstItem  = 0;
        lastItem   = 0;
        currItem   = 0;
        count      = 0;
        thumbJob   = 0;
        tileSize   = ThumbnailSize::Small;

        itemDict.setAutoDelete(false);
    }

    bool                       clearing;
    bool                       exifRotate;
    bool                       dragging;

    const int                  margin;
    int                        count;
    int                        tileSize;
    int                        orientation;

    QTimer                    *timer;

    QPoint                     dragStartPos;

    ThumbBarItem              *firstItem;
    ThumbBarItem              *lastItem;
    ThumbBarItem              *currItem;

    QDict<ThumbBarItem>        itemDict;
    QGuardedPtr<ThumbnailJob>  thumbJob;

    ThumbBarToolTipSettings    toolTipSettings;

    ThumbBarToolTip           *toolTip;
};

// -------------------------------------------------------------------------

class ThumbBarItemPriv
{
public:

    ThumbBarItemPriv()
    {
        pos    = 0;
        pixmap = 0;
        next   = 0;
        prev   = 0;
        view   = 0;
    }

    int           pos;

    QPixmap      *pixmap;

    KURL          url;

    ThumbBarItem *next;
    ThumbBarItem *prev;

    ThumbBarView *view;
};

// -------------------------------------------------------------------------

ThumbBarView::ThumbBarView(QWidget* parent, int orientation, bool exifRotate,
                           ThumbBarToolTipSettings settings)
            : QScrollView(parent)
{
    d = new ThumbBarViewPriv;
    d->orientation     = orientation;
    d->exifRotate      = exifRotate;
    d->toolTipSettings = settings;
    d->toolTip         = new ThumbBarToolTip(this);
    d->timer           = new QTimer(this);

    connect(d->timer, SIGNAL(timeout()),
            this, SLOT(slotUpdate()));

    viewport()->setBackgroundMode(Qt::NoBackground);
    viewport()->setMouseTracking(true);
    viewport()->setAcceptDrops(true);

    setFrameStyle(QFrame::NoFrame);
    setAcceptDrops(true); 

    if (d->orientation == Vertical)
    {
        setHScrollBarMode(QScrollView::AlwaysOff);
    }
    else
    {
        setVScrollBarMode(QScrollView::AlwaysOff);
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
    delete d->toolTip;
    delete d;
}

void ThumbBarView::resizeEvent(QResizeEvent* e)
{
    if (!e) return;

    QScrollView::resizeEvent(e);

    if (d->orientation == Vertical)
    {
        d->tileSize = width() - 2*d->margin - verticalScrollBar()->sizeHint().width();
        verticalScrollBar()->setLineStep(d->tileSize);
        verticalScrollBar()->setPageStep(2*d->tileSize);
    }
    else
    {
        d->tileSize = height() - 2*d->margin - horizontalScrollBar()->sizeHint().height();
        horizontalScrollBar()->setLineStep(d->tileSize);
        horizontalScrollBar()->setPageStep(2*d->tileSize);
    }

    rearrangeItems();
    ensureItemVisible(currentItem());
}

void ThumbBarView::setExifRotate(bool exifRotate)
{
    if (d->exifRotate == exifRotate)
	return;
	
    d->exifRotate = exifRotate;
    QString thumbCacheDir = QDir::homeDirPath() + "/.thumbnails/";

    for (ThumbBarItem *item = d->firstItem; item; item = item->d->next)
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

bool ThumbBarView::getExifRotate()
{
    return d->exifRotate;
}

int ThumbBarView::getOrientation()
{
    return d->orientation;
}

int ThumbBarView::getTileSize()
{
    return d->tileSize;
}

int ThumbBarView::getMargin()
{
    return d->margin;
}

void ThumbBarView::setToolTipSettings(const ThumbBarToolTipSettings &settings)
{
    d->toolTipSettings = settings;
}

ThumbBarToolTipSettings& ThumbBarView::getToolTipSettings()
{
    return d->toolTipSettings;
}

int ThumbBarView::countItems()
{
    return d->count;
}

KURL::List ThumbBarView::itemsURLs()
{
    KURL::List urlList;
    if (!countItems())
        return urlList;

    for (ThumbBarItem *item = firstItem(); item; item = item->next())
        urlList.append(item->url());

    return urlList;
}

void ThumbBarView::clear(bool updateView)
{
    d->clearing = true;

    ThumbBarItem *item = d->firstItem;
    while (item)
    {
        ThumbBarItem *tmp = item->d->next;
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

    emit signalItemSelected(0);
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

    for (ThumbBarItem *item = d->firstItem; item; item = item->d->next)
    {
        if (itemPos >= item->d->pos && itemPos <= (item->d->pos+d->tileSize+2*d->margin))
        {
            return item;
        }
    }

    return 0;
}

ThumbBarItem* ThumbBarView::findItemByURL(const KURL& url) const
{
    for (ThumbBarItem *item = d->firstItem; item; item = item->d->next)
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
    if (!item) return;

    ensureItemVisible(item);
    emit signalURLSelected(item->url());
    emit signalItemSelected(item);

    if (d->currItem == item) return;

    if (d->currItem)
    {
        ThumbBarItem* item = d->currItem;
        d->currItem = 0;
        item->repaint();
    }

    d->currItem = item;
    if (d->currItem)
        item->repaint();
}

void ThumbBarView::ensureItemVisible(ThumbBarItem* item)
{
    if (item)
    {
        // We want the complete thumb visible and the next one.
        // find the middle of the image and give a margin of 1,5 image
        // When changed, watch regression for bug 104031
        if (d->orientation == Vertical)
            ensureVisible(0, (int)(item->d->pos + d->margin + d->tileSize*.5),
                          0, (int)(d->tileSize*1.5 + 3*d->margin));
        else
            ensureVisible((int)(item->d->pos + d->margin + d->tileSize*.5), 0,
                          (int)(d->tileSize*1.5 + 3*d->margin), 0);
    }
}

void ThumbBarView::refreshThumbs(const KURL::List& urls)
{
    for (KURL::List::const_iterator it = urls.begin() ; it != urls.end() ; ++it)
    {
        ThumbBarItem *item = findItemByURL(*it);
        if (item)
        {
            invalidateThumb(item);
        }
    }
}

void ThumbBarView::invalidateThumb(ThumbBarItem* item)
{
    if (!item) return;

    if (item->d->pixmap)
    {
        delete item->d->pixmap;
        item->d->pixmap = 0;
    }

    if (!d->thumbJob.isNull())
    {
       d->thumbJob->kill();
       d->thumbJob = 0;
    }

    d->thumbJob = new ThumbnailJob(item->url(), ThumbnailSize::Huge, true, d->exifRotate);

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

    for (ThumbBarItem *item = d->firstItem; item; item = item->d->next)
    {
        if (d->orientation == Vertical)
        {
            if (y1 <= item->d->pos && item->d->pos <= y2)
            {
                if (item == d->currItem)
                    tile.fill(colorGroup().highlight());
                else
                    tile.fill(colorGroup().background());

                QPainter p(&tile);
                p.setPen(Qt::white);
                p.drawRect(0, 0, tile.width(), tile.height());
                p.end();

                if (item->d->pixmap)
                {
                    QPixmap pix; 
                    pix.convertFromImage(QImage(item->d->pixmap->convertToImage()).
                                         smoothScale(d->tileSize, d->tileSize, QImage::ScaleMin));
                    int x = (tile.width()  - pix.width())/2;
                    int y = (tile.height() - pix.height())/2;
                    bitBlt(&tile, x, y, &pix);
                }

                bitBlt(&bgPix, 0, item->d->pos - cy, &tile);
            }
        }
        else
        {
            if (x1 <= item->d->pos && item->d->pos <= x2)
            {
                if (item == d->currItem)
                    tile.fill(colorGroup().highlight());
                else
                    tile.fill(colorGroup().background());

                QPainter p(&tile);
                p.setPen(Qt::white);
                p.drawRect(0, 0, tile.width(), tile.height());
                p.end();

                if (item->d->pixmap)
                {
                    QPixmap pix; 
                    pix.convertFromImage(QImage(item->d->pixmap->convertToImage()).
                                         smoothScale(d->tileSize, d->tileSize, QImage::ScaleMin));
                    int x = (tile.width() - pix.width())/2;
                    int y = (tile.height()- pix.height())/2;
                    bitBlt(&tile, x, y, &pix);
                }

                bitBlt(&bgPix, item->d->pos - cx, 0, &tile);
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
    ThumbBarItem* barItem = findItem(e->pos());
    d->dragging           = true;
    d->dragStartPos       = e->pos();

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
}

void ThumbBarView::contentsMouseMoveEvent(QMouseEvent *e)
{
    if (!e) return;

    if (d->dragging && (e->state() & Qt::LeftButton))
    {
        if ( findItem(d->dragStartPos) &&
             (d->dragStartPos - e->pos()).manhattanLength() > QApplication::startDragDistance() )
        {
            startDrag();
        }
        return;
    }
}

void ThumbBarView::contentsMouseReleaseEvent(QMouseEvent *e)
{
    d->dragging = false;
    ThumbBarItem *item = findItem(e->pos());
    if (item) 
    {
        emit signalURLSelected(item->url());
        emit signalItemSelected(item);
    }
}

void ThumbBarView::contentsWheelEvent(QWheelEvent *e)
{
    e->accept();

    if (e->delta() < 0)
    {
        if (e->state() & Qt::ShiftButton)
        {
            if (d->orientation == Vertical)
                scrollBy(0, verticalScrollBar()->pageStep());
            else
                scrollBy(horizontalScrollBar()->pageStep(), 0);
        }
        else
        {
            if (d->orientation == Vertical)
                scrollBy(0, verticalScrollBar()->lineStep());
            else
                scrollBy(horizontalScrollBar()->lineStep(), 0);
        }
    }

    if (e->delta() > 0)
    {
        if (e->state() & Qt::ShiftButton)
        {
            if (d->orientation == Vertical)
                scrollBy(0, (-1)*verticalScrollBar()->pageStep());
            else
                scrollBy((-1)*horizontalScrollBar()->pageStep(), 0);
        }
        else
        {
            if (d->orientation == Vertical)
                scrollBy(0, (-1)*verticalScrollBar()->lineStep());
            else
                scrollBy((-1)*horizontalScrollBar()->lineStep(), 0);
        }
    }
}

void ThumbBarView::startDrag()
{
}

void ThumbBarView::insertItem(ThumbBarItem* item)
{
    if (!item) return;

    if (!d->firstItem)
    {
        d->firstItem = item;
        d->lastItem  = item;
        item->d->prev = 0;
        item->d->next = 0;
    }
    else
    {
        d->lastItem->d->next = item;
        item->d->prev = d->lastItem;
        item->d->next = 0;
        d->lastItem = item;

    }

    if (!d->currItem)
    {
        d->currItem = item;
        emit signalURLSelected(item->url());
        emit signalItemSelected(item);
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
        d->firstItem = d->currItem = d->firstItem->d->next;
        if (d->firstItem)
            d->firstItem->d->prev = 0;
        else
            d->firstItem = d->lastItem = d->currItem = 0;
    }
    else if (item == d->lastItem)
    {
        d->lastItem = d->currItem = d->lastItem->d->prev;
        if ( d->lastItem )
           d->lastItem->d->next = 0;
        else
            d->firstItem = d->lastItem = d->currItem = 0;
    }
    else
    {
        ThumbBarItem *i = item;
        if (i)
        {
            if (i->d->prev )
            {
                i->d->prev->d->next = d->currItem = i->d->next;
            }
            if ( i->d->next )
            {
                i->d->next->d->prev = d->currItem = i->d->prev;
            }
        }
    }

    d->itemDict.remove(item->url().url());

    if (!d->clearing)
    {
        triggerUpdate();
    }

    if (d->count == 0)
        emit signalItemSelected(0);
}

void ThumbBarView::rearrangeItems()
{
    KURL::List urlList;

    int pos = 0;
    ThumbBarItem *item = d->firstItem;

    while (item)
    {
        item->d->pos = pos;
        pos += d->tileSize + 2*d->margin;
        if (!(item->d->pixmap))
            urlList.append(item->d->url);
        item = item->d->next;
    }

    if (d->orientation == Vertical)
       resizeContents(visibleWidth(), d->count*(d->tileSize+2*d->margin));
    else    
       resizeContents(d->count*(d->tileSize+2*d->margin), visibleHeight());

    if (!urlList.isEmpty())
    {
        if (!d->thumbJob.isNull())
        {
           d->thumbJob->kill();
           d->thumbJob = 0;
        }

        d->thumbJob = new ThumbnailJob(urlList, ThumbnailSize::Huge, true, d->exifRotate);

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
           repaintContents(0, item->d->pos, visibleWidth(), d->tileSize+2*d->margin);
       else
           repaintContents(item->d->pos, 0, d->tileSize+2*d->margin, visibleHeight());
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

        if (item->d->pixmap)
        {
            delete item->d->pixmap;
            item->d->pixmap = 0;
        }

        item->d->pixmap = new QPixmap(pix);
        item->repaint();
    }
}

void ThumbBarView::slotFailedThumbnail(const KURL& url)
{
    ThumbBarItem* item = d->itemDict.find(url.url());
    if (!item)
        return;

    KIconLoader* iconLoader = KApplication::kApplication()->iconLoader();
    QPixmap pix = iconLoader->loadIcon("image", KIcon::NoGroup, ThumbnailSize::Huge);

    if (item->d->pixmap)
    {
        delete item->d->pixmap;
        item->d->pixmap = 0;
    }

    item->d->pixmap = new QPixmap(pix);
    item->repaint();
}

// -------------------------------------------------------------------------

ThumbBarItem::ThumbBarItem(ThumbBarView* view, const KURL& url)
{
    d = new ThumbBarItemPriv;
    d->url  = url;
    d->view = view;
    d->view->insertItem(this);
}

ThumbBarItem::~ThumbBarItem()
{
    d->view->removeItem(this);

    if (d->pixmap)
        delete d->pixmap;

    delete d;
}

KURL ThumbBarItem::url() const
{
    return d->url;
}

ThumbBarItem* ThumbBarItem::next() const
{
    return d->next;
}

ThumbBarItem* ThumbBarItem::prev() const
{
    return d->prev;
}

QRect ThumbBarItem::rect() const
{
    if (d->view->d->orientation == ThumbBarView::Vertical)
    {
        return QRect(0, d->pos,
                     d->view->visibleWidth(),
                     d->view->d->tileSize + 2*d->view->d->margin);
    }
    else
    {
        return QRect(d->pos, 0,
                     d->view->d->tileSize + 2*d->view->d->margin,
                     d->view->visibleHeight());
    }
}

int ThumbBarItem::position() const
{
    return d->pos;
}

QPixmap* ThumbBarItem::pixmap() const
{
    return d->pixmap;
}

void ThumbBarItem::repaint()
{
    d->view->repaintItem(this);   
}

// -------------------------------------------------------------------------

ThumbBarToolTip::ThumbBarToolTip(ThumbBarView* parent)
               : QToolTip(parent->viewport()), m_maxStringLen(30), m_view(parent)
{
    m_headBeg = QString("<tr bgcolor=\"orange\"><td colspan=\"2\">"
                        "<nobr><font size=\"-1\" color=\"black\"><b>");
    m_headEnd = QString("</b></font></nobr></td></tr>");

    m_cellBeg = QString("<tr><td><nobr><font size=\"-1\" color=\"black\">");
    m_cellMid = QString("</font></nobr></td>"
                        "<td><nobr><font size=\"-1\" color=\"black\">");
    m_cellEnd = QString("</font></nobr></td></tr>");

    m_cellSpecBeg = QString("<tr><td><nobr><font size=\"-1\" color=\"black\">");
    m_cellSpecMid = QString("</font></nobr></td>"
                            "<td><nobr><font size=\"-1\" color=\"steelblue\"><i>");
    m_cellSpecEnd = QString("</i></font></nobr></td></tr>");
}

void ThumbBarToolTip::maybeTip(const QPoint& pos)
{
    if ( !parentWidget() || !m_view) return;

    ThumbBarItem* item = m_view->findItem( m_view->viewportToContents(pos) );
    if (!item) return;

    if (!m_view->getToolTipSettings().showToolTips) return;

    QString tipText = tipContent(item);
    tipText.append(tipContentExtraData(item));
    tipText.append("</table>");

    QRect r(item->rect());
    r = QRect( m_view->contentsToViewport(r.topLeft()), r.size() );

    tip(r, tipText);
}

QString ThumbBarToolTip::tipContent(ThumbBarItem* item)
{
    ThumbBarToolTipSettings settings = m_view->getToolTipSettings();

    QString tipText, str;
    QString unavailable(i18n("unavailable"));

    tipText = "<table cellspacing=\"0\" cellpadding=\"0\" width=\"250\" border=\"0\">";

    QFileInfo fileInfo(item->url().path());

    KFileItem fi(KFileItem::Unknown, KFileItem::Unknown, item->url());
    DMetadata metaData(item->url().path());

    // -- File properties ----------------------------------------------

    if (settings.showFileName  ||
        settings.showFileDate  ||
        settings.showFileSize  ||
        settings.showImageType ||
        settings.showImageDim)
    {
        tipText += m_headBeg + i18n("File Properties") + m_headEnd;

        if (settings.showFileName)
        {
            tipText += m_cellBeg + i18n("Name:") + m_cellMid;
            tipText += item->url().fileName() + m_cellEnd;
        }

        if (settings.showFileDate)
        {
            QDateTime modifiedDate = fileInfo.lastModified();
            str = KGlobal::locale()->formatDateTime(modifiedDate, true, true);
            tipText += m_cellBeg + i18n("Modified:") + m_cellMid + str + m_cellEnd;
        }

        if (settings.showFileSize)
        {
            tipText += m_cellBeg + i18n("Size:") + m_cellMid;
            str = i18n("%1 (%2)").arg(KIO::convertSize(fi.size()))
                                .arg(KGlobal::locale()->formatNumber(fi.size(), 0));
            tipText += str + m_cellEnd;
        }

        QSize   dims;
#if KDCRAW_VERSION < 0x000106
        QString rawFilesExt(KDcrawIface::DcrawBinary::instance()->rawFiles());
#else
        QString rawFilesExt(KDcrawIface::KDcraw::rawFiles());
#endif
        QString ext = fileInfo.extension(false).upper();

        if (!ext.isEmpty() && rawFilesExt.upper().contains(ext))
        {
            str = i18n("RAW Image");
            dims = metaData.getImageDimensions();
        }
        else
        {
            str = fi.mimeComment();

            KFileMetaInfo meta = fi.metaInfo();
            if (meta.isValid())
            {
                if (meta.containsGroup("Jpeg EXIF Data"))
                    dims = meta.group("Jpeg EXIF Data").item("Dimensions").value().toSize();
                else if (meta.containsGroup("General"))
                    dims = meta.group("General").item("Dimensions").value().toSize();
                else if (meta.containsGroup("Technical"))
                    dims = meta.group("Technical").item("Dimensions").value().toSize();
            }
        }

        if (settings.showImageType)
        {
            tipText += m_cellBeg + i18n("Type:") + m_cellMid + str + m_cellEnd;
        }

        if (settings.showImageDim)
        {
            QString mpixels;
            mpixels.setNum(dims.width()*dims.height()/1000000.0, 'f', 2);
            str = (!dims.isValid()) ? i18n("Unknown") : i18n("%1x%2 (%3Mpx)")
                .arg(dims.width()).arg(dims.height()).arg(mpixels);
            tipText += m_cellBeg + i18n("Dimensions:") + m_cellMid + str + m_cellEnd;
        }
    }

    // -- Photograph Info ----------------------------------------------------
    
    if (settings.showPhotoMake  ||
        settings.showPhotoDate  ||
        settings.showPhotoFocal ||
        settings.showPhotoExpo  ||
        settings.showPhotoMode  ||
        settings.showPhotoFlash ||
        settings.showPhotoWB)
    {
        PhotoInfoContainer photoInfo = metaData.getPhotographInformations();

        if (!photoInfo.isEmpty())
        {
            QString metaStr;
            tipText += m_headBeg + i18n("Photograph Properties") + m_headEnd;

            if (settings.showPhotoMake)
            {
                str = QString("%1 / %2").arg(photoInfo.make.isEmpty() ? unavailable : photoInfo.make)
                                        .arg(photoInfo.model.isEmpty() ? unavailable : photoInfo.model);
                if (str.length() > m_maxStringLen) str = str.left(m_maxStringLen-3) + "...";
                metaStr += m_cellBeg + i18n("Make/Model:") + m_cellMid + QStyleSheet::escape( str ) + m_cellEnd;
            }

            if (settings.showPhotoDate)
            {
                if (photoInfo.dateTime.isValid())
                {
                    str = KGlobal::locale()->formatDateTime(photoInfo.dateTime, true, true);
                    if (str.length() > m_maxStringLen) str = str.left(m_maxStringLen-3) + "...";
                    metaStr += m_cellBeg + i18n("Created:") + m_cellMid + QStyleSheet::escape( str ) + m_cellEnd;
                }
                else
                    metaStr += m_cellBeg + i18n("Created:") + m_cellMid + QStyleSheet::escape( unavailable ) + m_cellEnd;
            }

            if (settings.showPhotoFocal)
            {
                str = photoInfo.aperture.isEmpty() ? unavailable : photoInfo.aperture;

                if (photoInfo.focalLength35mm.isEmpty())
                    str += QString(" / %1").arg(photoInfo.focalLength.isEmpty() ? unavailable : photoInfo.focalLength);
                else 
                    str += QString(" / %1").arg(i18n("%1 (35mm: %2)").arg(photoInfo.focalLength).arg(photoInfo.focalLength35mm));

                if (str.length() > m_maxStringLen) str = str.left(m_maxStringLen-3) + "...";
                metaStr += m_cellBeg + i18n("Aperture/Focal:") + m_cellMid + QStyleSheet::escape( str ) + m_cellEnd;
            }

            if (settings.showPhotoExpo)
            {
                str = QString("%1 / %2").arg(photoInfo.exposureTime.isEmpty() ? unavailable : photoInfo.exposureTime)
                                        .arg(photoInfo.sensitivity.isEmpty() ? unavailable : i18n("%1 ISO").arg(photoInfo.sensitivity));
                if (str.length() > m_maxStringLen) str = str.left(m_maxStringLen-3) + "...";
                metaStr += m_cellBeg + i18n("Exposure/Sensitivity:") + m_cellMid + QStyleSheet::escape( str ) + m_cellEnd;
            }

            if (settings.showPhotoMode)
            {

                if (photoInfo.exposureMode.isEmpty() && photoInfo.exposureProgram.isEmpty())
                    str = unavailable;
                else if (!photoInfo.exposureMode.isEmpty() && photoInfo.exposureProgram.isEmpty())
                    str = photoInfo.exposureMode;
                else if (photoInfo.exposureMode.isEmpty() && !photoInfo.exposureProgram.isEmpty())
                    str = photoInfo.exposureProgram;
                else 
                    str = QString("%1 / %2").arg(photoInfo.exposureMode).arg(photoInfo.exposureProgram);
                if (str.length() > m_maxStringLen) str = str.left(m_maxStringLen-3) + "...";
                metaStr += m_cellBeg + i18n("Mode/Program:") + m_cellMid + QStyleSheet::escape( str ) + m_cellEnd;
            }

            if (settings.showPhotoFlash)
            {
                str = photoInfo.flash.isEmpty() ? unavailable : photoInfo.flash;
                if (str.length() > m_maxStringLen) str = str.left(m_maxStringLen-3) + "...";
                metaStr += m_cellBeg + i18n("Flash:") + m_cellMid + QStyleSheet::escape( str ) + m_cellEnd;
            }

            if (settings.showPhotoWB)
            {
                str = photoInfo.whiteBalance.isEmpty() ? unavailable : photoInfo.whiteBalance;
                if (str.length() > m_maxStringLen) str = str.left(m_maxStringLen-3) + "...";
                metaStr += m_cellBeg + i18n("White Balance:") + m_cellMid + QStyleSheet::escape( str ) + m_cellEnd;
            }

            tipText += metaStr;
        }
    }

    return tipText;
}

QString ThumbBarToolTip::breakString(const QString& input)
{
    QString str = input.simplifyWhiteSpace();
    str = QStyleSheet::escape(str);
    const uint maxLen = m_maxStringLen;

    if (str.length() <= maxLen)
        return str;

    QString br;

    uint i     = 0;
    uint count = 0;

    while (i < str.length())
    {
        if (count >= maxLen && str[i].isSpace())
        {
            count = 0;
            br.append("<br>");
        }
        else
        {
            br.append(str[i]);
        }

        i++;
        count++;
    }

    return br;
}

}  // NameSpace Digikam
