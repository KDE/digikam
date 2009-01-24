/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-04-11
 * Description : light table thumbs bar
 *
 * Copyright (C) 2007-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <qpixmap.h>
#include <qpainter.h>
#include <qimage.h>
#include <qcursor.h>

// KDE includes.

#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kpopupmenu.h>
#include <kstandarddirs.h>

// Local includes.

#include "ddebug.h"
#include "album.h"
#include "albumdb.h"
#include "albummanager.h"
#include "albumsettings.h"
#include "dragobjects.h"
#include "imageattributeswatch.h"
#include "metadatahub.h"
#include "ratingpopupmenu.h"
#include "themeengine.h"
#include "lighttablebar.h"
#include "lighttablebar.moc"

namespace Digikam
{

class LightTableBarPriv
{

public:

    LightTableBarPriv()
    {
        navigateByPair = false;
        toolTip        = 0;
    }

    bool                  navigateByPair;

    QPixmap               ratingPixmap;

    LightTableBarToolTip *toolTip;
};

class LightTableBarItemPriv
{

public:

    LightTableBarItemPriv()
    {
        onLeftPanel  = false;
        onRightPanel = false;
        info         = 0;
    }

    bool       onLeftPanel;
    bool       onRightPanel;

    ImageInfo *info;
};

LightTableBar::LightTableBar(QWidget* parent, int orientation, bool exifRotate)
             : ThumbBarView(parent, orientation, exifRotate)
{
    d = new LightTableBarPriv;
    setMouseTracking(true);
    readToolTipSettings();
    d->toolTip = new LightTableBarToolTip(this);

    // -- Load rating Pixmap ------------------------------------------

    KGlobal::dirs()->addResourceType("digikam_rating", KGlobal::dirs()->kde_default("data")
                                     + "digikam/data");
    QString ratingPixPath = KGlobal::dirs()->findResourceDir("digikam_rating", "rating.png");
    ratingPixPath += "/rating.png";
    d->ratingPixmap = QPixmap(ratingPixPath);

    QPainter painter(&d->ratingPixmap);
    painter.fillRect(0, 0, d->ratingPixmap.width(), d->ratingPixmap.height(),
                     ThemeEngine::instance()->textSpecialRegColor());
    painter.end();

    if (orientation == Vertical)
        setMinimumWidth(d->ratingPixmap.width()*5 + 6 + 2*getMargin());
    else
        setMinimumHeight(d->ratingPixmap.width()*5 + 6 + 2*getMargin());

    // ----------------------------------------------------------------

    ImageAttributesWatch *watch = ImageAttributesWatch::instance();

    connect(watch, SIGNAL(signalImageRatingChanged(Q_LLONG)),
            this, SLOT(slotImageRatingChanged(Q_LLONG)));

    connect(ThemeEngine::instance(), SIGNAL(signalThemeChanged()),
            this, SLOT(slotThemeChanged()));

    connect(this, SIGNAL(signalItemSelected(ThumbBarItem*)),
            this, SLOT(slotItemSelected(ThumbBarItem*)));
}

LightTableBar::~LightTableBar()
{
    delete d->toolTip;
    delete d;
}

void LightTableBar::setNavigateByPair(bool b)
{
    d->navigateByPair = b;
}

void LightTableBar::slotImageRatingChanged(Q_LLONG imageId)
{
    for (ThumbBarItem *item = firstItem(); item; item = item->next())
    {
        LightTableBarItem *ltItem = dynamic_cast<LightTableBarItem*>(item);
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

    ThumbBarView::contentsMouseReleaseEvent(e);

    QPoint pos = QCursor::pos();
    LightTableBarItem *item = findItemByPos(e->pos());

    RatingPopupMenu *ratingMenu = 0;

    if (e->button() == Qt::RightButton)
    {
        KPopupMenu popmenu(this);

        if (item)
        {
            popmenu.insertItem(SmallIcon("previous"), i18n("Show on left panel"), 10);
            popmenu.insertItem(SmallIcon("next"), i18n("Show on right panel"), 11);
            popmenu.insertItem(SmallIcon("editimage"), i18n("Edit"), 12);

            if (d->navigateByPair)
            {
                popmenu.setItemEnabled(10, false);
                popmenu.setItemEnabled(11, false);
            }

            popmenu.insertSeparator();
            popmenu.insertItem(SmallIcon("fileclose"), i18n("Remove item"), 13);
        }

        int totalItems = itemsURLs().count();
        popmenu.insertItem(SmallIcon("editshred"), i18n("Clear all"), 14);
        popmenu.setItemEnabled(14, totalItems ? true : false);

        if (item)
        {
            popmenu.insertSeparator();

            // Assign Star Rating -------------------------------------------

            ratingMenu = new RatingPopupMenu();

            connect(ratingMenu, SIGNAL(activated(int)),
                    this, SLOT(slotAssignRating(int)));

            popmenu.insertItem(i18n("Assign Rating"), ratingMenu);
        }

        switch(popmenu.exec(pos))
        {
            case 10:    // Left panel
            {
                emit signalSetItemOnLeftPanel(item->info());
                break;
            }
            case 11:    // Right panel
            {
                emit signalSetItemOnRightPanel(item->info());
                break;
            }
            case 12:    // Edit
            {
                emit signalEditItem(item->info());
                break;
            }
            case 13:    // Remove
            {
                emit signalRemoveItem(item->info());
                break;
            }
            case 14:    // Clear All
            {
                emit signalClearAll();
                break;
            }
            default:
                break;
        }
    }

    delete ratingMenu;
}

void LightTableBar::slotAssignRating(int rating)
{
    rating = QMIN(5, QMAX(0, rating));
    ImageInfo *info = currentItemImageInfo();
    if (info)
    {
        MetadataHub hub;
        hub.load(info);
        hub.setRating(rating);
        hub.write(info, MetadataHub::PartialWrite);
        hub.write(info->filePath(), MetadataHub::FullWriteIfChanged);
    }
}

void LightTableBar::slotAssignRatingNoStar()
{
    slotAssignRating(0);
}

void LightTableBar::slotAssignRatingOneStar()
{
    slotAssignRating(1);
}

void LightTableBar::slotAssignRatingTwoStar()
{
    slotAssignRating(2);
}

void LightTableBar::slotAssignRatingThreeStar()
{
    slotAssignRating(3);
}

void LightTableBar::slotAssignRatingFourStar()
{
    slotAssignRating(4);
}

void LightTableBar::slotAssignRatingFiveStar()
{
    slotAssignRating(5);
}

void LightTableBar::setOnLeftPanel(const ImageInfo* info)
{
    for (ThumbBarItem *item = firstItem(); item; item = item->next())
    {
        LightTableBarItem *ltItem = dynamic_cast<LightTableBarItem*>(item);
        if (ltItem)
        {
            if (info)
            {
                if (ltItem->info()->id() == info->id())
                {
                    ltItem->setOnLeftPanel(true);
                    repaintItem(item);
                }
                else if (ltItem->isOnLeftPanel() == true)
                {
                    ltItem->setOnLeftPanel(false);
                    repaintItem(item);
                }
            }
            else if (ltItem->isOnLeftPanel() == true)
            {
                ltItem->setOnLeftPanel(false);
                repaintItem(item);
            }
        }
    }
}

void LightTableBar::setOnRightPanel(const ImageInfo* info)
{
    for (ThumbBarItem *item = firstItem(); item; item = item->next())
    {
        LightTableBarItem *ltItem = dynamic_cast<LightTableBarItem*>(item);
        if (ltItem)
        {
            if (info)
            {
                if (ltItem->info()->id() == info->id())
                {
                    ltItem->setOnRightPanel(true);
                    repaintItem(item);
                }
                else if (ltItem->isOnRightPanel() == true)
                {
                    ltItem->setOnRightPanel(false);
                    repaintItem(item);
                }
            }
            else if (ltItem->isOnRightPanel() == true)
            {
                ltItem->setOnRightPanel(false);
                repaintItem(item);
            }
        }
    }
}

void LightTableBar::slotItemSelected(ThumbBarItem* item)
{
    if (item)
    {
        LightTableBarItem *ltItem = dynamic_cast<LightTableBarItem*>(item);
        if (ltItem)
        {
            emit signalLightTableBarItemSelected(ltItem->info());
            return;
        }
    }

    emit signalLightTableBarItemSelected(0);
}

ImageInfo* LightTableBar::currentItemImageInfo() const
{
    if (currentItem())
    {
        LightTableBarItem *item = dynamic_cast<LightTableBarItem*>(currentItem());
        return item->info();
    }

    return 0;
}

ImageInfoList LightTableBar::itemsImageInfoList()
{
    ImageInfoList list;

    for (ThumbBarItem *item = firstItem(); item; item = item->next())
    {
        LightTableBarItem *ltItem = dynamic_cast<LightTableBarItem*>(item);
        if (ltItem)
        {
            ImageInfo *info = new ImageInfo(*(ltItem->info()));
            list.append(info);
        }
    }

    return list;
}

void LightTableBar::setSelectedItem(LightTableBarItem* ltItem)
{
    ThumbBarItem *item = static_cast<ThumbBarItem*>(ltItem);
    if (item) ThumbBarView::setSelected(item);
}

void LightTableBar::removeItem(const ImageInfo* info)
{
    if (!info) return;

    LightTableBarItem* ltItem = findItemByInfo(info);
    ThumbBarItem *item        = static_cast<ThumbBarItem*>(ltItem);
    if (item) ThumbBarView::removeItem(item);
}

LightTableBarItem* LightTableBar::findItemByInfo(const ImageInfo* info) const
{
    if (info)
    {
        for (ThumbBarItem *item = firstItem(); item; item = item->next())
        {
            LightTableBarItem *ltItem = dynamic_cast<LightTableBarItem*>(item);
            if (ltItem)
            {
                if (ltItem->info()->id() == info->id())
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
        LightTableBarItem *ltItem = dynamic_cast<LightTableBarItem*>(item);
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
    ThemeEngine* te = ThemeEngine::instance();
    QRect er(e->rect());
    QPixmap bgPix;

    if (countItems() > 0)
    {
        int cy, cx, ts, y1, y2, x1, x2;
        QPixmap tile;

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

        bgPix.fill(te->baseColor());

        for (ThumbBarItem *item = firstItem(); item; item = item->next())
        {
            if (getOrientation() == Vertical)
            {
                if (y1 <= item->position() && item->position() <= y2)
                {
                    if (item == currentItem())
                        tile = te->thumbSelPixmap(tile.width(), tile.height());
                    else
                        tile = te->thumbRegPixmap(tile.width(), tile.height());

                    QPainter p(&tile);
                    if (item == currentItem())
                    {
                        p.setPen(QPen(te->textSelColor(), 3));
                        p.drawRect(2, 2, tile.width()-2, tile.height()-2);
                    }
                    else
                    {
                        p.setPen(QPen(te->textRegColor(), 1));
                        p.drawRect(0, 0, tile.width(), tile.height());
                    }
                    p.end();

                    if (item->pixmap())
                    {
                        QPixmap pix;
                        pix.convertFromImage(QImage(item->pixmap()->convertToImage()).
                                             smoothScale(getTileSize(), getTileSize(), QImage::ScaleMin));
                        int x = (tile.width()  - pix.width())/2;
                        int y = (tile.height() - pix.height())/2;
                        bitBlt(&tile, x, y, &pix);

                        LightTableBarItem *ltItem = dynamic_cast<LightTableBarItem*>(item);

                        if (ltItem->isOnLeftPanel())
                        {
                            QPixmap lPix = SmallIcon("previous");
                            bitBlt(&tile, getMargin(), getMargin(), &lPix);
                        }
                        if (ltItem->isOnRightPanel())
                        {
                            QPixmap rPix = SmallIcon("next");
                            bitBlt(&tile, tile.width() - getMargin() - rPix.width(), getMargin(), &rPix);
                        }

                        QRect r(0, tile.height()-getMargin()-d->ratingPixmap.height(),
                                tile.width(), d->ratingPixmap.height());
                        int rating = ltItem->info()->rating();
                        int xr = (r.width() - rating * d->ratingPixmap.width())/2;
                        int wr = rating * d->ratingPixmap.width();
                        QPainter p(&tile);
                        p.drawTiledPixmap(xr, r.y(), wr, r.height(), d->ratingPixmap);
                    }

                    bitBlt(&bgPix, 0, item->position() - cy, &tile);
                }
            }
            else
            {
                if (x1 <= item->position() && item->position() <= x2)
                {
                    if (item == currentItem())
                        tile = te->thumbSelPixmap(tile.width(), tile.height());
                    else
                        tile = te->thumbRegPixmap(tile.width(), tile.height());

                    QPainter p(&tile);
                    if (item == currentItem())
                    {
                        p.setPen(QPen(te->textSelColor(), 2));
                        p.drawRect(1, 1, tile.width()-1, tile.height()-1);
                    }
                    else
                    {
                        p.setPen(QPen(te->textRegColor(), 1));
                        p.drawRect(0, 0, tile.width(), tile.height());
                    }
                    p.end();

                    if (item->pixmap())
                    {
                        QPixmap pix;
                        pix.convertFromImage(QImage(item->pixmap()->convertToImage()).
                                             smoothScale(getTileSize(), getTileSize(), QImage::ScaleMin));
                        int x = (tile.width() - pix.width())/2;
                        int y = (tile.height()- pix.height())/2;
                        bitBlt(&tile, x, y, &pix);

                        LightTableBarItem *ltItem = dynamic_cast<LightTableBarItem*>(item);

                        if (ltItem->isOnLeftPanel())
                        {
                            QPixmap lPix = SmallIcon("previous");
                            bitBlt(&tile, getMargin(), getMargin(), &lPix);
                        }
                        if (ltItem->isOnRightPanel())
                        {
                            QPixmap rPix = SmallIcon("next");
                            bitBlt(&tile, tile.width() - getMargin() - rPix.width(), getMargin(), &rPix);
                        }

                        QRect r(0, tile.height()-getMargin()-d->ratingPixmap.height(),
                                tile.width(), d->ratingPixmap.height());
                        int rating = ltItem->info()->rating();
                        int xr = (r.width() - rating * d->ratingPixmap.width())/2;
                        int wr = rating * d->ratingPixmap.width();
                        QPainter p(&tile);
                        p.drawTiledPixmap(xr, r.y(), wr, r.height(), d->ratingPixmap);
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
    else
    {
        bgPix.resize(contentsRect().width(), contentsRect().height());
        bgPix.fill(te->baseColor());
        QPainter p(&bgPix);
        p.setPen(QPen(te->textRegColor()));
        p.drawText(0, 0, bgPix.width(), bgPix.height(),
                    Qt::AlignCenter|Qt::WordBreak,
                    i18n("Drag and drop images here"));
        p.end();
        bitBlt(viewport(), 0, 0, &bgPix);
    }
}

void LightTableBar::startDrag()
{
    if (!currentItem()) return;

    KURL::List      urls;
    KURL::List      kioURLs;
    QValueList<int> albumIDs;
    QValueList<int> imageIDs;

    LightTableBarItem *item = dynamic_cast<LightTableBarItem*>(currentItem());

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
    int             albumID;
    QValueList<int> albumIDs;
    QValueList<int> imageIDs;
    KURL::List      urls;
    KURL::List      kioURLs;

    if (ItemDrag::decode(e, urls, kioURLs, albumIDs, imageIDs) ||
        AlbumDrag::decode(e, urls, albumID) ||
        TagDrag::canDecode(e))
    {
        e->accept();
        return;
    }

    e->ignore();
}

void LightTableBar::contentsDropEvent(QDropEvent *e)
{
    int             albumID;
    QValueList<int> albumIDs;
    QValueList<int> imageIDs;
    KURL::List      urls;
    KURL::List      kioURLs;

    if (ItemDrag::decode(e, urls, kioURLs, albumIDs, imageIDs))
    {
        ImageInfoList imageInfoList;

        for (QValueList<int>::const_iterator it = imageIDs.begin();
             it != imageIDs.end(); ++it)
        {
            ImageInfo *info = new ImageInfo(*it);
            if (!findItemByInfo(info))
            {
                imageInfoList.append(info);
            }
            else
            {
                delete info;
            }
        }

        emit signalDroppedItems(imageInfoList);
        e->accept();
    }
    else if (AlbumDrag::decode(e, urls, albumID))
    {
        QValueList<Q_LLONG> itemIDs = AlbumManager::instance()->albumDB()->getItemIDsInAlbum(albumID);
        ImageInfoList imageInfoList;

        for (QValueList<Q_LLONG>::const_iterator it = itemIDs.begin();
             it != itemIDs.end(); ++it)
        {
            ImageInfo *info = new ImageInfo(*it);
            if (!findItemByInfo(info))
            {
                imageInfoList.append(info);
            }
            else
            {
                delete info;
            }
        }

        emit signalDroppedItems(imageInfoList);
        e->accept();
    }
    else if(TagDrag::canDecode(e))
    {
        QByteArray ba = e->encodedData("digikam/tag-id");
        QDataStream ds(ba, IO_ReadOnly);
        int tagID;
        ds >> tagID;

        AlbumManager* man           = AlbumManager::instance();
        QValueList<Q_LLONG> itemIDs = man->albumDB()->getItemIDsInTag(tagID, true);
        ImageInfoList imageInfoList;

        for (QValueList<Q_LLONG>::const_iterator it = itemIDs.begin();
             it != itemIDs.end(); ++it)
        {
            ImageInfo *info = new ImageInfo(*it);
            if (!findItemByInfo(info))
            {
                imageInfoList.append(info);
            }
            else
            {
                delete info;
            }
        }

        emit signalDroppedItems(imageInfoList);
        e->accept();
    }
    else
    {
        e->ignore();
    }
}

void LightTableBar::slotThemeChanged()
{
    KGlobal::dirs()->addResourceType("digikam_rating", KGlobal::dirs()->kde_default("data")
                                     + "digikam/data");
    QString ratingPixPath = KGlobal::dirs()->findResourceDir("digikam_rating", "rating.png");
    ratingPixPath += "/rating.png";
    d->ratingPixmap = QPixmap(ratingPixPath);

    QPainter painter(&d->ratingPixmap);
    painter.fillRect(0, 0, d->ratingPixmap.width(), d->ratingPixmap.height(),
                     ThemeEngine::instance()->textSpecialRegColor());
    painter.end();

    slotUpdate();
}

// -------------------------------------------------------------------------

LightTableBarItem::LightTableBarItem(LightTableBar *view, ImageInfo *info)
                 : ThumbBarItem(view, info->kurl())
{
    d = new LightTableBarItemPriv;
    d->info = info;
}

LightTableBarItem::~LightTableBarItem()
{
    delete d;
}

ImageInfo* LightTableBarItem::info() const
{
    return d->info;
}

void LightTableBarItem::setOnLeftPanel(bool on)
{
    d->onLeftPanel = on;
}

void LightTableBarItem::setOnRightPanel(bool on)
{
    d->onRightPanel = on;
}

bool LightTableBarItem::isOnLeftPanel() const
{
    return d->onLeftPanel;
}

bool LightTableBarItem::isOnRightPanel() const
{
    return d->onRightPanel;
}

// -------------------------------------------------------------------------

LightTableBarToolTip::LightTableBarToolTip(ThumbBarView *parent)
                    : ThumbBarToolTip(parent)
{
}

QString LightTableBarToolTip::tipContentExtraData(ThumbBarItem *item)
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
                tip += m_cellSpecBeg + i18n("Caption:") + m_cellSpecMid + breakString(str) + m_cellSpecEnd;
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
