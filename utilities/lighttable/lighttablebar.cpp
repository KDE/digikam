/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-04-11
 * Description : light table thumbs bar
 *
 * Copyright (C) 2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes.

#include <qpainter.h>
#include <qimage.h>
#include <qcursor.h>

// KDE includes.

#include <klocale.h>
#include <kpopupmenu.h>
#include <kiconloader.h>
#include <kglobal.h>
#include <kstandarddirs.h>

// Local includes.

#include "ddebug.h"
#include "album.h"
#include "albumsettings.h"
#include "dragobjects.h"
#include "imageattributeswatch.h"
#include "themeengine.h"
#include "lighttablebar.h"
#include "lighttablebar.moc"

namespace Digikam
{

LightTableBar::LightTableBar(QWidget* parent, int orientation, bool exifRotate)
             : ThumbBarView(parent, orientation, exifRotate)
{
    setMouseTracking(true);
    readToolTipSettings();
    m_toolTip = new LightTableBarToolTip(this);

    connect(ThemeEngine::instance(), SIGNAL(signalThemeChanged()),
            this, SLOT(slotUpdate()));

    connect(this, SIGNAL(signalItemSelected(ThumbBarItem*)),
            this, SLOT(slotItemSelected(ThumbBarItem*)));

    // -- Load rating Pixmap ------------------------------------------

    KGlobal::dirs()->addResourceType("digikam_rating", KGlobal::dirs()->kde_default("data")
                                     + "digikam/data");
    QString ratingPixPath = KGlobal::dirs()->findResourceDir("digikam_rating", "rating.png");
    ratingPixPath += "/rating.png";
    m_ratingPixmap = QPixmap(ratingPixPath);

    QPainter painter(&m_ratingPixmap);
    painter.fillRect(0, 0, m_ratingPixmap.width(), m_ratingPixmap.height(),
                     ThemeEngine::instance()->textSpecialRegColor());
    painter.end();    

    // ----------------------------------------------------------------

    ImageAttributesWatch *watch = ImageAttributesWatch::instance();

    connect(watch, SIGNAL(signalImageRatingChanged(Q_LLONG)),
            this, SLOT(slotImageRatingChanged(Q_LLONG)));
}

LightTableBar::~LightTableBar()
{
    delete m_toolTip;
}

void LightTableBar::slotImageRatingChanged(Q_LLONG imageId)
{
    for (ThumbBarItem *item = firstItem(); item; item = item->next())
    {
        LightTableBarItem *ltItem = static_cast<LightTableBarItem*>(item);
        if (ltItem->info()->id() == imageId)
        {
            triggerUpdate();
            return;
        }
    }
}

void LightTableBar::contentsMouseReleaseEvent(QMouseEvent *e)
{
    if (!e) return;

    QPoint pos = QCursor::pos();
    LightTableBarItem *item = findItemByPos(e->pos());
    if (!item) return;        

    if (e->button() == Qt::RightButton)
    {
        KPopupMenu popmenu(this);
        popmenu.insertTitle(SmallIcon("digikam"), i18n("My Light Table"));
        popmenu.insertItem(SmallIcon("previous"), i18n("Show on left panel"), 10);
        popmenu.insertItem(SmallIcon("next"), i18n("Show on right panel"), 11);
        popmenu.insertSeparator(-1);
        popmenu.insertItem(SmallIcon("fileclose"), i18n("Remove"), 12);

        switch(popmenu.exec(pos))
        {
            case 10:
            {
                emit signalSetItemOnLeftPanel(item->info());
                break;
            }
            case 11:
            {
                emit signalSetItemOnRightPanel(item->info());
                break;
            }
            case 12:
            {
                KURL url = item->info()->kurl();
                emit signalRemoveItem(url);
                removeItem(currentItem());
                break;
            }
            default:
                break;
        }
    }
}

void LightTableBar::setOnLeftPanel(const ImageInfo* info)
{
    if (!info) return;

    for (ThumbBarItem *item = firstItem(); item; item = item->next())
    {
        KURL url1 = item->url();
        KURL url2 = info->kurl();
        LightTableBarItem *ltItem = static_cast<LightTableBarItem*>(item);
        ltItem->setOnLeftPanel(url1 == url2);
    }

    triggerUpdate();
}

void LightTableBar::setOnRightPanel(const ImageInfo* info)
{
    if (!info) return;

    for (ThumbBarItem *item = firstItem(); item; item = item->next())
    {
        KURL url1 = item->url();
        KURL url2 = info->kurl();
        LightTableBarItem *ltItem = static_cast<LightTableBarItem*>(item);
        ltItem->setOnRightPanel(url1 == url2);
    }

    triggerUpdate();
}

void LightTableBar::slotItemSelected(ThumbBarItem* i)
{
    if (i)
    {
        LightTableBarItem *item = static_cast<LightTableBarItem*>(i);
        emit signalLightTableBarItemSelected(item->info());
    }
    else
        emit signalLightTableBarItemSelected(0);
}

ImageInfo* LightTableBar::currentItemImageInfo() const
{
    if (currentItem())
    {
        LightTableBarItem *item = static_cast<LightTableBarItem*>(currentItem());
        return item->info();
    }

    return 0;
}

ImageInfoList LightTableBar::itemsImageInfoList()
{
    ImageInfoList list;

    for (ThumbBarItem *item = firstItem(); item; item = item->next())
    {
        LightTableBarItem *ltItem = static_cast<LightTableBarItem*>(item);
        if (ltItem) 
        {
            ImageInfo *info = new ImageInfo(*(ltItem->info()));
            list.append(info);            
        }
    }

    return list;
}

LightTableBarItem* LightTableBar::findItemByInfo(const ImageInfo* info) const
{
    if (info)
    {
        for (ThumbBarItem *item = firstItem(); item; item = item->next())
        {
            KURL url1 = item->url();
            KURL url2 = info->kurl();
        
            if (url1 == url2)
            {
                LightTableBarItem *ltItem = static_cast<LightTableBarItem*>(item);
                return ltItem;
            }
        }
    }
    return 0;
}

LightTableBarItem* LightTableBar::findItemByPos(const QPoint& pos) const
{
    ThumbBarItem *item = findItem(pos);
    if (item)
    {
        LightTableBarItem *ltItem = static_cast<LightTableBarItem*>(item);
        return ltItem;
    }
    
    return 0;
}

void LightTableBar::readToolTipSettings()
{
    AlbumSettings* albumSettings = AlbumSettings::instance();
    if (!albumSettings) return;

    Digikam::ThumbBarToolTipSettings settings;
    settings.showToolTips   = albumSettings->getShowToolTips();
    settings.showFileName   = albumSettings->getToolTipsShowFileName();
    settings.showFileDate   = albumSettings->getToolTipsShowFileDate();
    settings.showFileSize   = albumSettings->getToolTipsShowFileSize();
    settings.showImageType  = albumSettings->getToolTipsShowImageType();
    settings.showImageDim   = albumSettings->getToolTipsShowImageDim();
    settings.showPhotoMake  = albumSettings->getToolTipsShowPhotoMake();
    settings.showPhotoDate  = albumSettings->getToolTipsShowPhotoDate();
    settings.showPhotoFocal = albumSettings->getToolTipsShowPhotoFocal();
    settings.showPhotoExpo  = albumSettings->getToolTipsShowPhotoExpo();
    settings.showPhotoMode  = albumSettings->getToolTipsShowPhotoMode();
    settings.showPhotoFlash = albumSettings->getToolTipsShowPhotoFlash();
    settings.showPhotoWB    = albumSettings->getToolTipsShowPhotoWB();
    setToolTipSettings(settings);
}

void LightTableBar::viewportPaintEvent(QPaintEvent* e)
{
    int cy, cx, ts, y1, y2, x1, x2;
    QPixmap bgPix, tile;
    QRect er(e->rect());
    
    if (getOrientation() == Vertical)
    {
       cy = viewportToContents(er.topLeft()).y();
        
       bgPix.resize(contentsRect().width(), er.height());
    
       ts = getTileSize() + 2*getMargin();
       tile.resize(visibleWidth(), ts);
    
       y1 = (cy/ts)*ts;
       y2 = ((y1 + er.height())/ts +1)*ts;
    }
    else
    {
       cx = viewportToContents(er.topLeft()).x();
        
       bgPix.resize(er.width(), contentsRect().height());
    
       ts = getTileSize() + 2*getMargin();
       tile.resize(ts, visibleHeight());
    
       x1 = (cx/ts)*ts;
       x2 = ((x1 + er.width())/ts +1)*ts;
    }

    bgPix.fill(ThemeEngine::instance()->baseColor());
    
    for (ThumbBarItem *item = firstItem(); item; item = item->next())
    {
        if (getOrientation() == Vertical)
        {
            if (y1 <= item->position() && item->position() <= y2)
            {
                if (item == currentItem())
                    tile = ThemeEngine::instance()->thumbSelPixmap(tile.width(), tile.height());
                else
                    tile = ThemeEngine::instance()->thumbRegPixmap(tile.width(), tile.height());
    
                QPainter p(&tile);
                p.setPen(Qt::white);
                p.drawRect(0, 0, tile.width(), tile.height());
                p.end();
                
                if (item->pixmap())
                {
                    QPixmap pix; 
                    pix.convertFromImage(QImage(item->pixmap()->convertToImage()).
                                         smoothScale(getTileSize(), getTileSize(), QImage::ScaleMin));
                    int x = (tile.width()  - pix.width())/2;
                    int y = (tile.height() - pix.height())/2;
                    bitBlt(&tile, x, y, &pix);

                    LightTableBarItem *ltItem = static_cast<LightTableBarItem*>(item);
    
                    if (ltItem->getOnLeftPanel())
                    {
                        QPixmap lPix = SmallIcon("previous"); 
                        bitBlt(&tile, getMargin(), getMargin(), &lPix);
                    }
                    if (ltItem->getOnRightPanel())
                    {
                        QPixmap rPix = SmallIcon("next"); 
                        bitBlt(&tile, tile.width() - getMargin() - rPix.width(), getMargin(), &rPix);
                    }

                    QRect r(0, tile.height()-getMargin()-m_ratingPixmap.height(), 
                            tile.width(), m_ratingPixmap.height());
                    int rating = ltItem->info()->rating();
                    int xr = (r.width() - rating * m_ratingPixmap.width())/2;
                    int wr = rating * m_ratingPixmap.width();
                    QPainter p(&tile);
                    p.drawTiledPixmap(xr, r.y(), wr, r.height(), m_ratingPixmap);
                }
                
                bitBlt(&bgPix, 0, item->position() - cy, &tile);
            }
        }
        else
        {
            if (x1 <= item->position() && item->position() <= x2)
            {
                if (item == currentItem())
                    tile = ThemeEngine::instance()->thumbSelPixmap(tile.width(), tile.height());
                else
                    tile = ThemeEngine::instance()->thumbRegPixmap(tile.width(), tile.height());
    
                QPainter p(&tile);
                p.setPen(Qt::white);
                p.drawRect(0, 0, tile.width(), tile.height());
                p.end();
                
                if (item->pixmap())
                {
                    QPixmap pix; 
                    pix.convertFromImage(QImage(item->pixmap()->convertToImage()).
                                         smoothScale(getTileSize(), getTileSize(), QImage::ScaleMin));
                    int x = (tile.width() - pix.width())/2;
                    int y = (tile.height()- pix.height())/2;
                    bitBlt(&tile, x, y, &pix);

                    LightTableBarItem *ltItem = static_cast<LightTableBarItem*>(item);
    
                    if (ltItem->getOnLeftPanel())
                    {
                        QPixmap lPix = SmallIcon("previous"); 
                        bitBlt(&tile, getMargin(), getMargin(), &lPix);
                    }
                    if (ltItem->getOnRightPanel())
                    {
                        QPixmap rPix = SmallIcon("next"); 
                        bitBlt(&tile, tile.width() - getMargin() - rPix.width(), getMargin(), &rPix);
                    }

                    QRect r(0, tile.height()-getMargin()-m_ratingPixmap.height(), 
                            tile.width(), m_ratingPixmap.height());
                    int rating = ltItem->info()->rating();
                    int xr = (r.width() - rating * m_ratingPixmap.width())/2;
                    int wr = rating * m_ratingPixmap.width();
                    QPainter p(&tile);
                    p.drawTiledPixmap(xr, r.y(), wr, r.height(), m_ratingPixmap);
                }
                
                bitBlt(&bgPix, item->position() - cx, 0, &tile);
            }
        }
    }

    if (getOrientation() == Vertical)
       bitBlt(viewport(), 0, er.y(), &bgPix);
    else
       bitBlt(viewport(), er.x(), 0, &bgPix);
}

void LightTableBar::startDrag()
{
    if (!currentItem()) return;

    KURL::List      urls;
    KURL::List      kioURLs;
    QValueList<int> albumIDs;
    QValueList<int> imageIDs;

    LightTableBarItem *item = static_cast<LightTableBarItem*>(currentItem());

    urls.append(item->info()->kurl());
    kioURLs.append(item->info()->kurlForKIO());
    imageIDs.append(item->info()->id());
    albumIDs.append(item->info()->albumID());

    QPixmap icon(DesktopIcon("image", 48));
    int w = icon.width();
    int h = icon.height();

    QPixmap pix(w+4,h+4);
    QPainter p(&pix);
    p.fillRect(0, 0, w+4, h+4, QColor(Qt::white));
    p.setPen(QPen(Qt::black, 1));
    p.drawRect(0, 0, w+4, h+4);
    p.drawPixmap(2, 2, icon);
    p.end();

    QDragObject* drag = 0;

    drag = new ItemDrag(urls, kioURLs, albumIDs, imageIDs, this);
    if (drag)
    {
        drag->setPixmap(pix);
        drag->drag();
    }
}

void LightTableBar::contentsDragMoveEvent(QDragMoveEvent *e)
{
    KURL::List      urls;
    KURL::List      kioURLs;        
    QValueList<int> albumIDs;
    QValueList<int> imageIDs;

    if (!ItemDrag::decode(e, urls, kioURLs, albumIDs, imageIDs))
    {
        e->ignore();
        return;
    }
    e->accept();
}

void LightTableBar::contentsDropEvent(QDropEvent *e)
{
    KURL::List      urls;
    KURL::List      kioURLs;        
    QValueList<int> albumIDs;
    QValueList<int> imageIDs;

    if (ItemDrag::decode(e, urls, kioURLs, albumIDs, imageIDs))
    {
        for (QValueList<int>::const_iterator it = imageIDs.begin();
                it != imageIDs.end(); ++it)
        {
            ImageInfo *info = new ImageInfo(*it);
            if (!findItemByInfo(info))
            {
                new LightTableBarItem(this, info);
            }
            else
            {
                delete info;
            }
        }
    }
    else 
    {
        e->ignore();
    }
}

// -------------------------------------------------------------------------

LightTableBarItem::LightTableBarItem(LightTableBar *view, ImageInfo *info)
                 : ThumbBarItem(view, info->kurl())
{
    m_info         = info;
    m_onLeftPanel  = false;
    m_onRightPanel = false;
}

LightTableBarItem::~LightTableBarItem()
{
}

ImageInfo* LightTableBarItem::info()
{
    return m_info;
}

void LightTableBarItem::setOnLeftPanel(bool on)
{
    m_onLeftPanel = on;
}

void LightTableBarItem::setOnRightPanel(bool on)
{
    m_onRightPanel = on;
}

bool LightTableBarItem::getOnLeftPanel() const
{
    return m_onLeftPanel;
}

bool LightTableBarItem::getOnRightPanel() const
{
    return m_onRightPanel;
}

// -------------------------------------------------------------------------

LightTableBarToolTip::LightTableBarToolTip(ThumbBarView* parent)
                    : ThumbBarToolTip(parent)
{
}

QString LightTableBarToolTip::tipContentExtraData(ThumbBarItem* item)
{
    QString tip, str;
    AlbumSettings* settings = AlbumSettings::instance();
    ImageInfo* info         = static_cast<LightTableBarItem *>(item)->info();

    if (settings)
    {
        if (settings->getToolTipsShowAlbumName() ||
            settings->getToolTipsShowComments()  ||
            settings->getToolTipsShowTags()      ||
            settings->getToolTipsShowRating())
        {
            tip += m_headBeg + i18n("digiKam Properties") + m_headEnd;
    
            if (settings->getToolTipsShowAlbumName())
            {
                PAlbum* album = info->album();
                if (album)
                    tip += m_cellSpecBeg + i18n("Album:") + m_cellSpecMid + 
                           album->url().remove(0, 1) + m_cellSpecEnd;
            }
    
            if (settings->getToolTipsShowComments())
            {
                str = info->caption();
                if (str.isEmpty()) str = QString("---");
                tip += m_cellSpecBeg + i18n("Comments:") + m_cellSpecMid + breakString(str) + m_cellSpecEnd;
            }
    
            if (settings->getToolTipsShowTags())
            {
                QStringList tagPaths = info->tagPaths(false);
    
                str = tagPaths.join(", ");
                if (str.isEmpty()) str = QString("---");
                if (str.length() > m_maxStringLen) str = str.left(m_maxStringLen-3) + "...";
                tip += m_cellSpecBeg + i18n("Tags:") + m_cellSpecMid + str + m_cellSpecEnd;
            }
    
            if (settings->getToolTipsShowRating())
            {
                str.fill( '*', info->rating() );
                if (str.isEmpty()) str = QString("---");
                tip += m_cellSpecBeg + i18n("Rating:") + m_cellSpecMid + str + m_cellSpecEnd;
            }
        }
    }

    return tip;
}

}  // NameSpace Digikam
