/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-11-22
 * Description : a bar widget to display image thumbnails
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2005-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "thumbbar.moc"

// C++ includes

#include <cmath>

// Qt includes

#include <QDateTime>
#include <QDir>
#include <QFrame>
#include <QHash>
#include <QImage>
#include <QPainter>
#include <QPalette>
#include <QPixmap>
#include <QPoint>
#include <QTextDocument>
#include <QTimer>
#include <QToolTip>
#include <QFileInfo>

// KDE includes

#include <kapplication.h>
#include <kcodecs.h>
#include <kfileitem.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmimetype.h>
#include <kdebug.h>
#include <kdeversion.h>

// LibKDcraw includes

#include <libkdcraw/kdcraw.h>
#include <libkdcraw/version.h>

// Local includes

#include "dmetadata.h"
#include "thumbnailloadthread.h"
#include "thumbnailsize.h"

namespace Digikam
{

class ThumbBarView::ThumbBarViewPriv
{
public:

    ThumbBarViewPriv() :
        margin(5), radius(3)
    {
        dragging        = false;
        clearing        = false;
        needPreload     = false;
        toolTipItem     = 0;
        firstItem       = 0;
        lastItem        = 0;
        currItem        = 0;
        highlightedItem = 0;
        count           = 0;
        thumbLoadThread = 0;
        tileSize        = ThumbnailSize::Small;
        maxTileSize     = ThumbnailSize::Huge;
        toolTipTimer    = 0;
        timer           = 0;
        preloadTimer    = 0;
        toolTip         = 0;
        orientation     = Qt::Horizontal;
    }

    bool                        clearing;
    bool                        dragging;
    bool                        needPreload;

    const int                   margin;
    const int                   radius;
    int                         count;
    int                         tileSize;
    int                         orientation;
    int                         maxTileSize;

    QTimer*                     timer;
    QTimer*                     toolTipTimer;
    QTimer*                     preloadTimer;

    QPoint                      dragStartPos;

    ThumbBarItem*               firstItem;
    ThumbBarItem*               lastItem;
    ThumbBarItem*               currItem;
    ThumbBarItem*               highlightedItem;
    ThumbBarItem*               toolTipItem;

    QHash<KUrl, ThumbBarItem*>  itemHash;
    ThumbnailLoadThread*        thumbLoadThread;

    ThumbBarToolTipSettings     toolTipSettings;

    ThumbBarToolTip*            toolTip;
};

// -------------------------------------------------------------------------

class ThumbBarItem::ThumbBarItemPriv
{
public:

    ThumbBarItemPriv() :
        pos(0),
        next(0),
        prev(0),
        view(0)
    {
    }

    int           pos;

    QRect         tooltipRect;

    KUrl          url;

    ThumbBarItem* next;
    ThumbBarItem* prev;

    ThumbBarView* view;
};

// -------------------------------------------------------------------------

ThumbBarView::ThumbBarView(QWidget* parent, int orientation,
                           const ThumbBarToolTipSettings& settings)
    : Q3ScrollView(parent), d(new ThumbBarViewPriv)
{
    d->toolTipSettings = settings;
    d->timer           = new QTimer(this);
    d->toolTipTimer    = new QTimer(this);
    d->preloadTimer    = new QTimer(this);
    d->preloadTimer->setSingleShot(true);
    d->thumbLoadThread = ThumbnailLoadThread::defaultThumbBarThread();
    d->maxTileSize     = d->thumbLoadThread->maximumThumbnailSize();

    connect(d->thumbLoadThread, SIGNAL(signalThumbnailLoaded(LoadingDescription,QPixmap)),
            this, SLOT(slotGotThumbnail(LoadingDescription,QPixmap)));

    connect(d->timer, SIGNAL(timeout()),
            this, SLOT(slotUpdate()));

    connect(d->preloadTimer, SIGNAL(timeout()),
            this, SLOT(slotPreload()));

    connect(this, SIGNAL(contentsMoving(int,int)),
            this, SLOT(slotContentsMoved()));

    connect(d->toolTipTimer, SIGNAL(timeout()),
            this, SLOT(slotToolTip()));

    viewport()->setMouseTracking(true);
    viewport()->setAcceptDrops(true);

    setFrameStyle(QFrame::NoFrame);
    setAcceptDrops(true);
    setOrientation(orientation);
}

ThumbBarView::~ThumbBarView()
{
    // Delete all hash items
    while (!d->itemHash.isEmpty())
    {
        ThumbBarItem* value = *d->itemHash.begin();
        d->itemHash.erase(d->itemHash.begin());
        delete value;
    }

    clear(false);

    delete d->timer;
    delete d->toolTipTimer;
    delete d->toolTip;
    delete d;
}

void ThumbBarView::setOrientation(int orientation)
{
    if (orientation != d->orientation)
    {
        d->orientation = orientation;

        // Reset the minimum and maximum sizes.
        setMinimumSize(QSize(0, 0));
        setMaximumSize(QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX));

        // Adjust minimum and maximum width to thumbnail sizes.
        if (d->orientation == Qt::Vertical)
        {
            setMinimumWidth(ThumbnailSize::Small + 2*d->margin + 2*d->radius + verticalScrollBar()->sizeHint().width());
            setMaximumWidth(d->maxTileSize + 2*d->margin + 2*d->radius + verticalScrollBar()->sizeHint().width());
            setVScrollBarMode(Q3ScrollView::Auto);
            setHScrollBarMode(Q3ScrollView::AlwaysOff);
        }
        else
        {
            setMinimumHeight(ThumbnailSize::Small + 2*d->margin + 2*d->radius + verticalScrollBar()->sizeHint().width());
            setMaximumHeight(d->maxTileSize + 2*d->margin + 2*d->radius + horizontalScrollBar()->sizeHint().height());
            setHScrollBarMode(Q3ScrollView::Auto);
            setVScrollBarMode(Q3ScrollView::AlwaysOff);
        }
    }
}

void ThumbBarView::setToolTip(ThumbBarToolTip* toolTip)
{
    d->toolTip = toolTip;
}

void ThumbBarView::resizeEvent(QResizeEvent* e)
{
    if (!e)
    {
        return;
    }

    Q3ScrollView::resizeEvent(e);

    if (d->orientation == Qt::Vertical)
    {
        d->tileSize = width() - 2*d->margin - 2*d->radius - verticalScrollBar()->sizeHint().width();
        verticalScrollBar()->setSingleStep(d->tileSize);
        verticalScrollBar()->setPageStep(2*d->tileSize);
    }
    else
    {
        d->tileSize = height() - 2*d->margin - 2*d->radius - horizontalScrollBar()->sizeHint().height();
        horizontalScrollBar()->setSingleStep(d->tileSize);
        horizontalScrollBar()->setPageStep(2*d->tileSize);
    }

    rearrangeItems();
    ensureItemVisible(currentItem());
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

int ThumbBarView::getRadius()
{
    return d->radius;
}

void ThumbBarView::setToolTipSettings(const ThumbBarToolTipSettings& settings)
{
    d->toolTipSettings = settings;
}

ThumbBarToolTipSettings& ThumbBarView::getToolTipSettings() const
{
    return d->toolTipSettings;
}

int ThumbBarView::countItems()
{
    return d->count;
}

KUrl::List ThumbBarView::itemsUrls()
{
    KUrl::List urlList;

    if (!countItems())
    {
        return urlList;
    }

    for (ThumbBarItem* item = firstItem(); item; item = item->next())
    {
        urlList.append(item->url());
    }

    return urlList;
}

void ThumbBarView::triggerUpdate()
{
    d->timer->setSingleShot(true);
    d->timer->start(0);
}

ThumbBarItem* ThumbBarView::currentItem() const
{
    return d->currItem;
}

ThumbBarItem* ThumbBarView::highlightedItem() const
{
    return d->highlightedItem;
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

    if (d->orientation == Qt::Vertical)
    {
        itemPos = pos.y();
    }
    else
    {
        itemPos = pos.x();
    }

    for (ThumbBarItem* item = d->firstItem; item; item = item->d->next)
    {
        if (itemPos >= item->d->pos && itemPos <= (item->d->pos+d->tileSize+2*d->margin+2*d->radius))
        {
            return item;
        }
    }

    return 0;
}

ThumbBarItem* ThumbBarView::findItemByUrl(const KUrl& url) const
{
    for (ThumbBarItem* item = d->firstItem; item; item = item->d->next)
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
    {
        return;
    }

    ensureItemVisible(item);
    emit signalUrlSelected(item->url());
    emit signalItemSelected(item);

    if (d->currItem == item)
    {
        return;
    }

    if (d->currItem)
    {
        ThumbBarItem* item = d->currItem;
        d->currItem = 0;
        item->repaint();
    }

    d->currItem = item;

    if (d->currItem)
    {
        item->repaint();
    }
}

void ThumbBarView::ensureItemVisible(ThumbBarItem* item)
{
    if (item)
    {
        if (d->highlightedItem)
        {
            d->highlightedItem = 0;
        }

        d->toolTipItem = 0;
        d->toolTipTimer->stop();
        slotToolTip();

        int pos = item->d->pos + d->margin + d->radius + (int)(d->tileSize*.5);

        // We want the complete thumb visible and the next one.
        // find the middle of the image and give a margin of 1,5 image
        // When changed, watch regression for bug 104031
        if (d->orientation == Qt::Vertical)
        {
            ensureVisible(0, pos, 0, viewport()->height());
        }
        else
        {
            ensureVisible(pos, 0, viewport()->width(), 0);
        }
    }
}

void ThumbBarView::refreshThumbs(const KUrl::List& urls)
{
    for (KUrl::List::const_iterator it = urls.constBegin() ; it != urls.constEnd() ; ++it)
    {
        ThumbBarItem* item = findItemByUrl(*it);

        if (item)
        {
            invalidateThumb(item);
        }
    }
}

void ThumbBarView::invalidateThumb(ThumbBarItem* item)
{
    if (!item)
    {
        return;
    }

    d->thumbLoadThread->deleteThumbnail(item->url().toLocalFile());
    d->thumbLoadThread->find(item->url().toLocalFile(), d->tileSize);
}

void ThumbBarView::reloadThumbs(const KUrl::List& urls)
{
    // This one does not delete the thumbnail file on disk
    for (KUrl::List::const_iterator it = urls.constBegin() ; it != urls.constEnd() ; ++it)
    {
        ThumbBarItem* item = findItemByUrl(*it);

        if (item)
        {
            reloadThumb(item);
        }
    }
}

void ThumbBarView::reloadThumb(ThumbBarItem* item)
{
    if (!item)
    {
        return;
    }

    d->thumbLoadThread->find(item->url().toLocalFile(), d->tileSize);
}

bool ThumbBarView::pixmapForItem(ThumbBarItem* item, QPixmap& pix) const
{
    if (d->tileSize > d->maxTileSize)
    {
        //TODO: Install a widget maximum size to prevent this situation
        bool hasPixmap = d->thumbLoadThread->find(item->url().toLocalFile(), pix, d->maxTileSize);

        if (hasPixmap)
        {
            kWarning() << "Thumbbar: Requested thumbnail size" << d->tileSize
                       << "is larger than the maximum thumbnail size" << d->maxTileSize
                       << ". Returning a scaled-up image.";
            pix = pix.scaled(d->tileSize, d->tileSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return d->thumbLoadThread->find(item->url().toLocalFile(), pix, d->tileSize);
    }
}

void ThumbBarView::preloadPixmapForItem(ThumbBarItem* item) const
{
    d->thumbLoadThread->preload(item->url().toLocalFile(), qMin(d->tileSize, d->maxTileSize));
}

void ThumbBarView::viewportPaintEvent(QPaintEvent* e)
{
    int   ts;
    QRect tile;
    QRect contentsPaintRect(viewportToContents(e->rect().topLeft()), viewportToContents(e->rect().bottomRight()));

    if (d->orientation == Qt::Vertical)
    {
        ts   = d->tileSize + 2*d->margin + 2*d->radius;
        tile = QRect(0, 0, visibleWidth()-1, ts-1);
    }
    else
    {
        ts   = d->tileSize + 2*d->margin + 2*d->radius;
        tile = QRect(0, 0, ts-1, visibleHeight()-1);
    }

    QPainter p(viewport());
    p.fillRect(e->rect(), palette().color(QPalette::Background));

    for (ThumbBarItem* item = d->firstItem; item; item = item->d->next)
    {
        if (d->orientation == Qt::Vertical)
        {
            if (item->rect().intersects(contentsPaintRect))
            {
                int translate = item->d->pos - contentsY();
                p.translate(0, translate);

                p.setPen(Qt::white);

                if (item == d->currItem)
                {
                    p.setBrush(palette().highlight().color());
                }
                else
                {
                    p.setBrush(palette().background().color());
                }

                p.drawRect(tile);

                if (item == d->highlightedItem)
                {
                    QRect r = item->rect();
                    p.setPen(QPen(palette().color(QPalette::Highlight), 3, Qt::SolidLine));
                    p.drawRect(1, 1, r.width()-3, r.height()-3);
                }

                QPixmap pix;

                if (pixmapForItem(item, pix))
                {
                    int x = (tile.width()  - pix.width())/2;
                    int y = (tile.height() - pix.height())/2;
                    p.drawPixmap(x, y, pix);
                    p.drawPixmap(x-d->radius, y-d->radius,
                                 generateFuzzyRect(QSize(pix.width()+2*d->radius,
                                                         pix.height()+2*d->radius),
                                                   QColor(0, 0, 0, 128), d->radius));
                    item->setTooltipRect(QRect(x, y+item->position(), pix.width(), pix.height()));
                }

                p.translate(0, - translate);
            }
        }
        else
        {
            if (item->rect().intersects(contentsPaintRect))
            {
                int translate = item->d->pos - contentsX();
                p.translate(translate, 0);

                p.setPen(Qt::white);

                if (item == d->currItem)
                {
                    p.setBrush(palette().highlight().color());
                }
                else
                {
                    p.setBrush(palette().background().color());
                }

                p.drawRect(tile);

                if (item == d->highlightedItem)
                {
                    QRect r = item->rect();
                    p.setPen(QPen(palette().color(QPalette::Highlight), 3, Qt::SolidLine));
                    p.drawRect(1, 1, r.width()-3, r.height()-3);
                }

                QPixmap pix;

                if (pixmapForItem(item, pix))
                {
                    int x = (tile.width()  - pix.width())/2;
                    int y = (tile.height() - pix.height())/2;
                    p.drawPixmap(x, y, pix);
                    p.drawPixmap(x-3, y-3, generateFuzzyRect(QSize(pix.width()+6,
                                 pix.height()+6),
                                 QColor(0, 0, 0, 128), 3));
                    item->setTooltipRect(QRect(x+item->position(), y, pix.width(), pix.height()));
                }

                p.translate(- translate, 0);
            }
        }
    }

    checkPreload();
}

QPixmap ThumbBarView::generateFuzzyRect(const QSize& size, const QColor& color, int radius)
{
    QPixmap pix(size);
    pix.fill(Qt::transparent);

    QPainter painter(&pix);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // Draw corners ----------------------------------

    QRadialGradient gradient;
    gradient.setColorAt(1, Qt::transparent);
    gradient.setColorAt(0, color);
    gradient.setRadius(radius);
    QPoint center;

    // Top Left
    center = QPoint(radius, radius);
    gradient.setCenter(center);
    gradient.setFocalPoint(center);
    painter.fillRect(0, 0, radius, radius, gradient);

    // Top right
    center = QPoint(size.width() - radius, radius);
    gradient.setCenter(center);
    gradient.setFocalPoint(center);
    painter.fillRect(center.x(), 0, radius, radius, gradient);

    // Bottom left
    center = QPoint(radius, size.height() - radius);
    gradient.setCenter(center);
    gradient.setFocalPoint(center);
    painter.fillRect(0, center.y(), radius, radius, gradient);

    // Bottom right
    center = QPoint(size.width() - radius, size.height() - radius);
    gradient.setCenter(center);
    gradient.setFocalPoint(center);
    painter.fillRect(center.x(), center.y(), radius, radius, gradient);

    // Draw borders ----------------------------------

    QLinearGradient linearGradient;
    linearGradient.setColorAt(1, Qt::transparent);
    linearGradient.setColorAt(0, color);

    // Top
    linearGradient.setStart(0, radius);
    linearGradient.setFinalStop(0, 0);
    painter.fillRect(radius, 0, size.width() - 2*radius, radius, linearGradient);

    // Bottom
    linearGradient.setStart(0, size.height() - radius);
    linearGradient.setFinalStop(0, size.height());
    painter.fillRect(radius, int(linearGradient.start().y()), size.width() - 2*radius, radius, linearGradient);

    // Left
    linearGradient.setStart(radius, 0);
    linearGradient.setFinalStop(0, 0);
    painter.fillRect(0, radius, radius, size.height() - 2*radius, linearGradient);

    // Right
    linearGradient.setStart(size.width() - radius, 0);
    linearGradient.setFinalStop(size.width(), 0);
    painter.fillRect(int(linearGradient.start().x()), radius, radius, size.height() - 2*radius, linearGradient);
    return pix;
}

void ThumbBarView::contentsMousePressEvent(QMouseEvent* e)
{
    // hide tooltip
    d->toolTipItem = 0;
    d->toolTipTimer->stop();
    slotToolTip();

    if (e->button() != Qt::LeftButton)
    {
        return;
    }

    ThumbBarItem* barItem = findItem(e->pos());
    d->dragging           = true;
    d->dragStartPos       = e->pos();

    if (!barItem || barItem == d->currItem)
    {
        return;
    }

    if (d->currItem)
    {
        ThumbBarItem* item = d->currItem;
        d->currItem = 0;
        item->repaint();
    }

    d->currItem = barItem;
    barItem->repaint();
}

void ThumbBarView::contentsMouseMoveEvent(QMouseEvent* e)
{
    if (!e)
    {
        return;
    }

    if (e->buttons() == Qt::NoButton)
    {
        ThumbBarItem* item = findItem(e->pos());

        if (d->toolTipSettings.showToolTips)
        {
            if (!isActiveWindow())
            {
                d->toolTipItem = 0;
                d->toolTipTimer->stop();
                slotToolTip();
                return;
            }

            if (item != d->toolTipItem)
            {
                d->toolTipItem = 0;
                d->toolTipTimer->stop();
                slotToolTip();

                if (acceptToolTip(item, e->pos()))
                {
                    d->toolTipItem = item;
                    d->toolTipTimer->setSingleShot(true);
                    d->toolTipTimer->start(500);
                }
            }

            if (item == d->toolTipItem && !acceptToolTip(item, e->pos()))
            {
                d->toolTipItem = 0;
                d->toolTipTimer->stop();
                slotToolTip();
            }
        }

        // Draw item highlightment when mouse is over.

        if (item != d->highlightedItem)
        {
            d->highlightedItem = item;
            viewport()->update();
        }

        return;
    }

    d->toolTipItem  = 0;
    d->toolTipTimer->stop();
    slotToolTip();

    if (d->dragging && (e->buttons() & Qt::LeftButton))
    {
        if ( findItem(d->dragStartPos) &&
             (d->dragStartPos - e->pos()).manhattanLength() > QApplication::startDragDistance() )
        {
            startDrag();
        }

        return;
    }
}

void ThumbBarView::contentsMouseReleaseEvent(QMouseEvent* e)
{
    d->dragging = false;
    ThumbBarItem* item = findItem(e->pos());

    if (e->button() == Qt::LeftButton && item)
    {
        emit signalUrlSelected(item->url());
        emit signalItemSelected(item);
    }
}

void ThumbBarView::contentsWheelEvent(QWheelEvent* e)
{
    e->accept();

    if (d->highlightedItem)
    {
        d->highlightedItem = 0;
    }

    d->toolTipItem = 0;
    d->toolTipTimer->stop();
    slotToolTip();

    if (e->delta() < 0)
    {
        if (e->modifiers() & Qt::ShiftModifier)
        {
            if (d->orientation == Qt::Vertical)
            {
                scrollBy(0, verticalScrollBar()->pageStep());
            }
            else
            {
                scrollBy(horizontalScrollBar()->pageStep(), 0);
            }
        }
        else
        {
            if (d->orientation == Qt::Vertical)
            {
                scrollBy(0, verticalScrollBar()->singleStep());
            }
            else
            {
                scrollBy(horizontalScrollBar()->singleStep(), 0);
            }
        }
    }

    if (e->delta() > 0)
    {
        if (e->modifiers() & Qt::ShiftModifier)
        {
            if (d->orientation == Qt::Vertical)
            {
                scrollBy(0, (-1)*verticalScrollBar()->pageStep());
            }
            else
            {
                scrollBy((-1)*horizontalScrollBar()->pageStep(), 0);
            }
        }
        else
        {
            if (d->orientation == Qt::Vertical)
            {
                scrollBy(0, (-1)*verticalScrollBar()->singleStep());
            }
            else
            {
                scrollBy((-1)*horizontalScrollBar()->singleStep(), 0);
            }
        }
    }
}

void ThumbBarView::leaveEvent(QEvent* e)
{
    if (d->highlightedItem)
    {
        d->highlightedItem = 0;
    }

    // hide tooltip
    d->toolTipItem = 0;
    d->toolTipTimer->stop();
    slotToolTip();

    Q3ScrollView::leaveEvent(e);
}

void ThumbBarView::focusOutEvent(QFocusEvent* e)
{
    if (d->highlightedItem)
    {
        d->highlightedItem = 0;
    }

    // hide tooltip
    d->toolTipItem = 0;
    d->toolTipTimer->stop();
    slotToolTip();

    Q3ScrollView::focusOutEvent(e);
}

void ThumbBarView::slotToolTip()
{
    if (d->toolTip)
    {
        d->toolTip->setItem(d->toolTipItem);
    }
}

void ThumbBarView::startDrag()
{
}

void ThumbBarView::clear(bool updateView)
{
    d->clearing        = true;
    d->highlightedItem = 0;
    d->toolTipItem     = 0;
    d->toolTipTimer->stop();
    slotToolTip();

    ThumbBarItem* item = d->firstItem;

    while (item)
    {
        ThumbBarItem* tmp = item->d->next;
        delete item;
        item = tmp;
    }

    d->firstItem = 0;
    d->lastItem  = 0;
    d->count     = 0;
    d->currItem  = 0;

    if (updateView)
    {
        slotUpdate();
    }

    d->clearing = false;

    emit signalItemSelected(0);
}

void ThumbBarView::insertItem(ThumbBarItem* item)
{
    if (!item)
    {
        return;
    }

    d->toolTipItem = 0;
    d->toolTipTimer->stop();
    slotToolTip();

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
        emit signalUrlSelected(item->url());
        emit signalItemSelected(item);
    }

    d->itemHash.insert(item->url(), item);

    d->count++;
    triggerUpdate();
    emit signalItemAdded();
}

void ThumbBarView::takeItem(ThumbBarItem* item)
{
    if (!item)
    {
        return;
    }

    if (d->toolTipItem == item)
    {
        d->toolTipItem = 0;
        d->toolTipTimer->stop();
        slotToolTip();
    }

    if (d->highlightedItem == item)
    {
        d->highlightedItem = 0;
    }

    d->count--;

    if (item == d->firstItem)
    {
        d->firstItem = d->currItem = d->firstItem->d->next;

        if (d->firstItem)
        {
            d->firstItem->d->prev = 0;
        }
        else
        {
            d->firstItem = d->lastItem = d->currItem = 0;
        }
    }
    else if (item == d->lastItem)
    {
        d->lastItem = d->currItem = d->lastItem->d->prev;

        if ( d->lastItem )
        {
            d->lastItem->d->next = 0;
        }
        else
        {
            d->firstItem = d->lastItem = d->currItem = 0;
        }
    }
    else
    {
        ThumbBarItem* i = item;

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

    d->itemHash.remove(item->url());

    if (!d->clearing)
    {
        triggerUpdate();
    }

    if (d->count == 0)
    {
        emit signalItemSelected(0);
    }
}

void ThumbBarView::removeItem(ThumbBarItem* item)
{
    if (!item)
    {
        return;
    }

    if (d->toolTipItem == item)
    {
        d->toolTipItem = 0;
        d->toolTipTimer->stop();
        slotToolTip();
    }

    if (d->highlightedItem == item)
    {
        d->highlightedItem = 0;
    }

    delete item;
}

void ThumbBarView::rearrangeItems()
{
    KUrl::List urlList;
    d->highlightedItem = 0;
    d->toolTipItem     = 0;
    d->toolTipTimer->stop();
    slotToolTip();

    int pos            = 0;
    ThumbBarItem* item = d->firstItem;

    while (item)
    {
        item->d->pos = pos;
        pos          += d->tileSize + 2*d->margin + 2*d->radius;
        item         = item->d->next;
    }

    if (d->orientation == Qt::Vertical)
    {
        resizeContents(visibleWidth(), d->count*(d->tileSize+2*d->margin+2*d->radius));
    }
    else
    {
        resizeContents(d->count*(d->tileSize+2*d->margin+2*d->radius), visibleHeight());
    }

    // only trigger preload if we have valid arranged items
    if (d->count)
    {
        d->needPreload = true;
    }
}

void ThumbBarView::repaintItem(ThumbBarItem* item)
{
    if (item)
    {
        if (d->orientation == Qt::Vertical)
        {
            repaintContents(0, item->d->pos, visibleWidth(), d->tileSize+2*d->margin+2*d->radius);
        }
        else
        {
            repaintContents(item->d->pos, 0, d->tileSize+2*d->margin+2*d->radius, visibleHeight());
        }
    }
}

void ThumbBarView::slotUpdate()
{
    rearrangeItems();
    viewport()->update();
}

void ThumbBarView::checkPreload()
{
    if (d->needPreload && !d->preloadTimer->isActive())
    {
        d->preloadTimer->start(50);
    }
}

void ThumbBarView::slotContentsMoved()
{
    d->needPreload = true;
}

void ThumbBarView::slotPreload()
{
    d->needPreload = false;
    // we get items in an area visibleWidth() to the left and right of the visible area
    QRect visibleArea(contentsX(), contentsY(), visibleWidth(), visibleHeight());

    if (getOrientation() == Qt::Vertical)
    {
        int y1 = contentsY() - visibleHeight();
        int y2 = contentsY();
        int y3 = contentsY() + visibleHeight();
        int y4 = contentsY() + 2* visibleHeight();

        for (ThumbBarItem* item = firstItem(); item; item = item->next())
        {
            int pos = item->position();

            if ( (y1 <= pos && pos <= y2) || (y3 <= pos && pos <= y4))
            {
                if (!item->rect().intersects(visibleArea))
                {
                    preloadPixmapForItem(item);
                }
            }

            if (pos > y4)
            {
                break;
            }
        }
    }
    else
    {
        int x1 = contentsX() - visibleWidth();
        int x2 = contentsX();
        int x3 = contentsX() + visibleWidth();
        int x4 = contentsX() + 2* visibleWidth();

        for (ThumbBarItem* item = firstItem(); item; item = item->next())
        {
            int pos = item->position();

            if ( (x1 <= pos && pos <= x2) || (x3 <= pos && pos <= x4))
            {
                if (!item->rect().intersects(visibleArea))
                {
                    preloadPixmapForItem(item);
                }
            }

            if (pos > x4)
            {
                break;
            }
        }
    }
}

void ThumbBarView::slotGotThumbnail(const LoadingDescription& desc, const QPixmap& pix)
{
    if (!pix.isNull())
    {
        QHash<KUrl, ThumbBarItem*>::const_iterator it = d->itemHash.constFind(KUrl(desc.filePath));

        if (it == d->itemHash.constEnd())
        {
            return;
        }

        ThumbBarItem* item = *it;
        item->repaint();
    }
}

bool ThumbBarView::acceptToolTip(ThumbBarItem* item, const QPoint& p)
{
    if (item && item->tooltipRect().contains(p))
    {
        return true;
    }

    return false;
}

// -------------------------------------------------------------------------

ThumbBarItem::ThumbBarItem(ThumbBarView* view, const KUrl& url)
    : d(new ThumbBarItemPriv)
{
    d->url  = url;
    d->view = view;
    d->view->insertItem(this);
}

ThumbBarItem::~ThumbBarItem()
{
    d->view->takeItem(this);
    delete d;
}

KUrl ThumbBarItem::url() const
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
    if (d->view->d->orientation == Qt::Vertical)
    {
        return QRect(0, d->pos,
                     d->view->visibleWidth(),
                     d->view->d->tileSize + 2*d->view->d->margin+2*d->view->d->radius);
    }
    else
    {
        return QRect(d->pos, 0,
                     d->view->d->tileSize + 2*d->view->d->margin+2*d->view->d->radius,
                     d->view->visibleHeight());
    }
}

void ThumbBarItem::setTooltipRect(const QRect& rect)
{
    d->tooltipRect = rect;
}

QRect ThumbBarItem::tooltipRect() const
{
    return d->tooltipRect;
}

int ThumbBarItem::position() const
{
    return d->pos;
}

void ThumbBarItem::repaint()
{
    d->view->repaintItem(this);
}

// -------------------------------------------------------------------------

class ThumbBarToolTip::ThumbBarToolTipPriv
{
public:

    ThumbBarToolTipPriv() :
        view(0),
        item(0)
    {
    }

    ThumbBarView* view;

    ThumbBarItem* item;
};

ThumbBarToolTip::ThumbBarToolTip(ThumbBarView* view)
    : DItemToolTip(), d(new ThumbBarToolTipPriv)
{
    d->view = view;
}

ThumbBarToolTip::~ThumbBarToolTip()
{
    delete d;
}

ThumbBarToolTipSettings& ThumbBarToolTip::toolTipSettings() const
{
    return d->view->getToolTipSettings();
}

void ThumbBarToolTip::setItem(ThumbBarItem* item)
{
    d->item = item;

    if (!d->item)
    {
        hide();
    }
    else
    {
        updateToolTip();
        reposition();

        if (isHidden() && !toolTipIsEmpty())
        {
            show();
        }
    }
}

ThumbBarItem* ThumbBarToolTip::item() const
{
    return d->item;
}

QRect ThumbBarToolTip::repositionRect()
{
    if (!item())
    {
        return QRect();
    }

    QRect rect = item()->rect();
    rect.moveTopLeft(d->view->contentsToViewport(rect.topLeft()));
    rect.moveTopLeft(d->view->viewport()->mapToGlobal(rect.topLeft()));
    return rect;
}

QString ThumbBarToolTip::tipContents()
{
    if (!item())
    {
        return QString();
    }

    QString                 str;
    ThumbBarToolTipSettings settings = toolTipSettings();
    DToolTipStyleSheet      cnt(settings.font);

    QFileInfo fileInfo(item()->url().toLocalFile());
    KFileItem fi(KFileItem::Unknown, KFileItem::Unknown, item()->url());
    DMetadata metaData(item()->url().toLocalFile());
    QString tip = cnt.tipHeader;

    // -- File properties ----------------------------------------------

    if (settings.showFileName  ||
        settings.showFileDate  ||
        settings.showFileSize  ||
        settings.showImageType ||
        settings.showImageDim)
    {
        tip += cnt.headBeg + i18n("File Properties") + cnt.headEnd;

        if (settings.showFileName)
        {
            tip += cnt.cellBeg + i18n("Name:") + cnt.cellMid;
            tip += item()->url().fileName() + cnt.cellEnd;
        }

        if (settings.showFileDate)
        {
            QDateTime modifiedDate = fileInfo.lastModified();
            str = KGlobal::locale()->formatDateTime(modifiedDate, KLocale::ShortDate, true);
            tip += cnt.cellBeg + i18n("Date:") + cnt.cellMid + str + cnt.cellEnd;
        }

        if (settings.showFileSize)
        {
            tip += cnt.cellBeg + i18n("Size:") + cnt.cellMid;
            str = i18n("%1 (%2)", KIO::convertSize(fi.size()),
                       KGlobal::locale()->formatNumber(fi.size(), 0));
            tip += str + cnt.cellEnd;
        }

        QSize   dims;

        QString rawFilesExt(KDcrawIface::KDcraw::rawFiles());
        QString ext = fileInfo.suffix().toUpper();

        if (!ext.isEmpty() && rawFilesExt.toUpper().contains(ext))
        {
            str  = i18n("RAW Image");
            dims = metaData.getImageDimensions();
        }
        else
        {
            str  = fi.mimeComment();
            dims = metaData.getPixelSize();
        }

        if (settings.showImageType)
        {
            tip += cnt.cellBeg + i18n("Type:") + cnt.cellMid + str + cnt.cellEnd;
        }

        if (settings.showImageDim)
        {
            QString mpixels;
            mpixels.setNum(dims.width()*dims.height()/1000000.0, 'f', 2);
            str = (!dims.isValid()) ? i18n("Unknown") : i18n("%1x%2 (%3Mpx)",
                    dims.width(), dims.height(), mpixels);
            tip += cnt.cellBeg + i18n("Dimensions:") + cnt.cellMid + str + cnt.cellEnd;
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
        PhotoInfoContainer photoInfo = metaData.getPhotographInformation();

        if (!photoInfo.isEmpty())
        {
            QString metaStr;
            tip += cnt.headBeg + i18n("Photograph Properties") + cnt.headEnd;

            if (settings.showPhotoMake)
            {
                str = QString("%1 / %2").arg(photoInfo.make.isEmpty() ? cnt.unavailable : photoInfo.make)
                      .arg(photoInfo.model.isEmpty() ? cnt.unavailable : photoInfo.model);

                if (str.length() > cnt.maxStringLength)
                {
                    str = str.left(cnt.maxStringLength-3) + "...";
                }

                metaStr += cnt.cellBeg + i18n("Make/Model:") + cnt.cellMid + Qt::escape(str) + cnt.cellEnd;
            }

            if (settings.showPhotoDate)
            {
                if (photoInfo.dateTime.isValid())
                {
                    str = KGlobal::locale()->formatDateTime(photoInfo.dateTime, KLocale::ShortDate, true);

                    if (str.length() > cnt.maxStringLength)
                    {
                        str = str.left(cnt.maxStringLength-3) + "...";
                    }

                    metaStr += cnt.cellBeg + i18n("Created:") + cnt.cellMid + Qt::escape(str) + cnt.cellEnd;
                }
                else
                {
                    metaStr += cnt.cellBeg + i18n("Created:") + cnt.cellMid + Qt::escape(cnt.unavailable) + cnt.cellEnd;
                }
            }

            if (settings.showPhotoFocal)
            {
                str = photoInfo.aperture.isEmpty() ? cnt.unavailable : photoInfo.aperture;

                if (photoInfo.focalLength35mm.isEmpty())
                {
                    str += QString(" / %1").arg(photoInfo.focalLength.isEmpty() ? cnt.unavailable : photoInfo.focalLength);
                }
                else
                    str += QString(" / %1").arg(i18n("%1 (35mm: %2)",
                                                     photoInfo.focalLength, photoInfo.focalLength35mm));

                if (str.length() > cnt.maxStringLength)
                {
                    str = str.left(cnt.maxStringLength-3) + "...";
                }

                metaStr += cnt.cellBeg + i18n("Aperture/Focal:") + cnt.cellMid + Qt::escape(str) + cnt.cellEnd;
            }

            if (settings.showPhotoExpo)
            {
                str = QString("%1 / %2").arg(photoInfo.exposureTime.isEmpty() ? cnt.unavailable :
                                             photoInfo.exposureTime)
                      .arg(photoInfo.sensitivity.isEmpty() ? cnt.unavailable :
                           i18n("%1 ISO", photoInfo.sensitivity));

                if (str.length() > cnt.maxStringLength)
                {
                    str = str.left(cnt.maxStringLength-3) + "...";
                }

                metaStr += cnt.cellBeg + i18n("Exposure/Sensitivity:") + cnt.cellMid + Qt::escape(str) + cnt.cellEnd;
            }

            if (settings.showPhotoMode)
            {
                if (photoInfo.exposureMode.isEmpty() && photoInfo.exposureProgram.isEmpty())
                {
                    str = cnt.unavailable;
                }
                else if (!photoInfo.exposureMode.isEmpty() && photoInfo.exposureProgram.isEmpty())
                {
                    str = photoInfo.exposureMode;
                }
                else if (photoInfo.exposureMode.isEmpty() && !photoInfo.exposureProgram.isEmpty())
                {
                    str = photoInfo.exposureProgram;
                }
                else
                {
                    str = QString("%1 / %2").arg(photoInfo.exposureMode).arg(photoInfo.exposureProgram);
                }

                if (str.length() > cnt.maxStringLength)
                {
                    str = str.left(cnt.maxStringLength-3) + "...";
                }

                metaStr += cnt.cellBeg + i18n("Mode/Program:") + cnt.cellMid + Qt::escape(str) + cnt.cellEnd;
            }

            if (settings.showPhotoFlash)
            {
                str = photoInfo.flash.isEmpty() ? cnt.unavailable : photoInfo.flash;

                if (str.length() > cnt.maxStringLength)
                {
                    str = str.left(cnt.maxStringLength-3) + "...";
                }

                metaStr += cnt.cellBeg + i18n("Flash:") + cnt.cellMid + Qt::escape(str) + cnt.cellEnd;
            }

            if (settings.showPhotoWB)
            {
                str = photoInfo.whiteBalance.isEmpty() ? cnt.unavailable : photoInfo.whiteBalance;

                if (str.length() > cnt.maxStringLength)
                {
                    str = str.left(cnt.maxStringLength-3) + "...";
                }

                metaStr += cnt.cellBeg + i18n("White Balance:") + cnt.cellMid + Qt::escape(str) + cnt.cellEnd;
            }

            tip += metaStr;
        }
    }

    tip += cnt.tipFooter;

    return tip;
}

}  // namespace Digikam
