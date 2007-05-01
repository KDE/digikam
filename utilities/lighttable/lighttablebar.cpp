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

// Qt includes.

#include <qpainter.h>
#include <qimage.h>

// KDE includes.

#include <klocale.h>

// Local includes.

#include "album.h"
#include "albumsettings.h"
#include "themeengine.h"
#include "imageinfo.h"
#include "lighttablebar.h"
#include "lighttablebar.moc"

namespace Digikam
{

LightTableBar::LightTableBar(QWidget* parent, int orientation, bool exifRotate)
             : ThumbBarView(parent, orientation, exifRotate)
{
    readToolTipSettings();
    m_toolTip = new LightTableBarToolTip(this);

    connect(ThemeEngine::instance(), SIGNAL(signalThemeChanged()),
            this, SLOT(slotUpdate()));

    connect(this, SIGNAL(signalItemSelected(ThumbBarItem*)),
            this, SLOT(slotItemSelected(ThumbBarItem*)));    
}

LightTableBar::~LightTableBar()
{
    delete m_toolTip;
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
