/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-04-11
 * Description : light table thumbs bar
 *
 * Copyright (C) 2007-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "lighttablebar.moc"

// Qt includes

#include <QList>
#include <QToolTip>
#include <QPixmap>
#include <QPainter>
#include <QImage>
#include <QCursor>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPolygon>
#include <QTextDocument>

// KDE includes

#include <kapplication.h>
#include <kfileitem.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmenu.h>
#include <kmimetype.h>
#include <kstandarddirs.h>

// Local includes

#include "album.h"
#include "albumdb.h"
#include "albummanager.h"
#include "albumsettings.h"
#include "contextmenuhelper.h"
#include "colorlabelwidget.h"
#include "globals.h"
#include "ddragobjects.h"
#include "imageattributeswatch.h"
#include "metadatasettings.h"
#include "metadatahub.h"
#include "ratingwidget.h"
#include "databasewatch.h"
#include "databasechangesets.h"
#include "themeengine.h"
#include "tooltipfiller.h"

namespace Digikam
{

class ImagePreviewBar::ImagePreviewBarPriv
{

public:

    ImagePreviewBarPriv()
    {
        // Pre-computed star polygon for a 15x15 pixmap.
        starPolygon << QPoint(0,  6);
        starPolygon << QPoint(5,  5);
        starPolygon << QPoint(7,  0);
        starPolygon << QPoint(9,  5);
        starPolygon << QPoint(14, 6);
        starPolygon << QPoint(10, 9);
        starPolygon << QPoint(11, 14);
        starPolygon << QPoint(7,  11);
        starPolygon << QPoint(3,  14);
        starPolygon << QPoint(4,  9);

        ratingItem   = 0;
        ratingWidget = 0;
    }

    QPolygon      starPolygon;

    QPixmap       ratingPixmap;

    ThumbBarItem* ratingItem;

    RatingWidget* ratingWidget;
};

ImagePreviewBar::ImagePreviewBar(QWidget* parent, int orientation, bool exifRotate)
    : ThumbBarView(parent, orientation, exifRotate),
      d(new ImagePreviewBarPriv)
{
    setMouseTracking(true);
    readToolTipSettings();
    setToolTip(new ImagePreviewBarToolTip(this));

    // -- Load rating Pixmap ------------------------------------------

    d->ratingPixmap = QPixmap(16, 15);
    d->ratingPixmap.fill(Qt::transparent);

    QPainter painter(&d->ratingPixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setBrush(kapp->palette().color(QPalette::Link));
    painter.setPen(ThemeEngine::instance()->textRegColor());
    painter.drawPolygon(d->starPolygon, Qt::WindingFill);
    painter.end();

    d->ratingWidget = new RatingWidget(viewport());
    d->ratingWidget->setTracking(false);
    d->ratingWidget->setFading(true);
    d->ratingWidget->installEventFilter(this);
    d->ratingWidget->hide();

    if (orientation == Qt::Vertical)
    {
        setMinimumWidth(d->ratingPixmap.width()*5 + 6 + 2*getMargin() + 2*getRadius());
    }
    else
    {
        setMinimumHeight(d->ratingPixmap.width()*5 + 6 + 2*getMargin() + 2*getRadius());
    }

    // ----------------------------------------------------------------

    ImageAttributesWatch* watch = ImageAttributesWatch::instance();

    connect(watch, SIGNAL(signalImageRatingChanged(qlonglong)),
            this, SLOT(slotImageRatingChanged(qlonglong)));

    connect(ThemeEngine::instance(), SIGNAL(signalThemeChanged()),
            this, SLOT(slotThemeChanged()));

    connect(d->ratingWidget, SIGNAL(signalRatingChanged(int)),
            this, SLOT(slotEditRatingFromItem(int)));
}

ImagePreviewBar::~ImagePreviewBar()
{
    delete d;
}

void ImagePreviewBar::clear(bool updateView)
{
    ThumbBarItem* item = d->ratingItem;

    if (item)
    {
        unsetCursor();
        d->ratingItem = 0;
        d->ratingWidget->hide();
        item->repaint();
    }

    ThumbBarView::clear(updateView);
}

void ImagePreviewBar::takeItem(ThumbBarItem* item)
{
    if (!item)
    {
        return;
    }

    if (d->ratingItem == item)
    {
        unsetCursor();
        d->ratingWidget->hide();
        d->ratingItem = 0;
        item->repaint();
    }

    ThumbBarView::takeItem(item);
}

void ImagePreviewBar::removeItem(ThumbBarItem* item)
{
    if (!item)
    {
        return;
    }

    if (d->ratingItem == item)
    {
        unsetCursor();
        d->ratingWidget->hide();
        d->ratingItem = 0;
        item->repaint();
    }

    ThumbBarView::removeItem(item);
}

void ImagePreviewBar::rearrangeItems()
{
    ThumbBarItem* item = d->ratingItem;

    if (item)
    {
        unsetCursor();
        d->ratingItem = 0;
        d->ratingWidget->hide();
        item->repaint();
    }

    ThumbBarView::rearrangeItems();
}

void ImagePreviewBar::ensureItemVisible(ThumbBarItem* item)
{
    if (!item)
    {
        return;
    }

    ThumbBarItem* ritem = d->ratingItem;

    if (ritem)
    {
        unsetCursor();
        d->ratingItem = 0;
        d->ratingWidget->hide();
        ritem->repaint();
    }

    ThumbBarView::ensureItemVisible(item);
}

void ImagePreviewBar::leaveEvent(QEvent* e)
{
    ThumbBarItem* item = d->ratingItem;

    if (item)
    {
        unsetCursor();
        d->ratingItem = 0;
        d->ratingWidget->hide();
        item->repaint();
    }

    ThumbBarView::leaveEvent(e);
}

void ImagePreviewBar::focusOutEvent(QFocusEvent* e)
{
    ThumbBarItem* item = d->ratingItem;

    if (item)
    {
        unsetCursor();
        d->ratingItem = 0;
        d->ratingWidget->hide();
        item->repaint();
    }

    ThumbBarView::focusOutEvent(e);
}

void ImagePreviewBar::contentsWheelEvent(QWheelEvent* e)
{
    ThumbBarItem* item = d->ratingItem;

    if (item)
    {
        unsetCursor();
        d->ratingItem = 0;
        d->ratingWidget->hide();
        item->repaint();
    }

    ThumbBarView::contentsWheelEvent(e);
}

ThumbBarItem* ImagePreviewBar::ratingItem() const
{
    return d->ratingItem;
}

QPixmap ImagePreviewBar::ratingPixmap() const
{
    return d->ratingPixmap;
}

void ImagePreviewBar::setSelectedItem(ImagePreviewBarItem* ltItem)
{
    ThumbBarItem* item = dynamic_cast<ThumbBarItem*>(ltItem);

    if (item)
    {
        ThumbBarView::setSelected(item);
    }
}

void ImagePreviewBar::slotImageRatingChanged(qlonglong imageId)
{
    for (ThumbBarItem* item = firstItem(); item; item = item->next())
    {
        ImagePreviewBarItem* ltItem = dynamic_cast<ImagePreviewBarItem*>(item);

        if (ltItem->info().id() == imageId)
        {
            triggerUpdate();
            return;
        }
    }
}

void ImagePreviewBar::slotEditRatingFromItem(int rating)
{
    if (!d->ratingItem)
    {
        return;
    }

    ImagePreviewBarItem* ltItem = dynamic_cast<ImagePreviewBarItem*>(d->ratingItem);

    rating = qMin(5, qMax(0, rating));
    ImageInfo info = ltItem->info();

    if (!info.isNull())
    {
        MetadataHub hub;
        hub.load(info);
        hub.setRating(rating);
        hub.write(info, MetadataHub::PartialWrite);
        hub.write(info.filePath(), MetadataHub::FullWriteIfChanged);
    }
}

ImageInfo ImagePreviewBar::currentItemImageInfo() const
{
    if (currentItem())
    {
        ImagePreviewBarItem* item = dynamic_cast<ImagePreviewBarItem*>(currentItem());
        return item->info();
    }

    return ImageInfo();
}

ImageInfoList ImagePreviewBar::itemsImageInfoList()
{
    ImageInfoList list;

    for (ThumbBarItem* item = firstItem(); item; item = item->next())
    {
        ImagePreviewBarItem* ltItem = dynamic_cast<ImagePreviewBarItem*>(item);

        if (ltItem)
        {
            list << ltItem->info();
        }
    }

    return list;
}

ImagePreviewBarItem* ImagePreviewBar::findItemByInfo(const ImageInfo& info) const
{
    if (!info.isNull())
    {
        for (ThumbBarItem* item = firstItem(); item; item = item->next())
        {
            ImagePreviewBarItem* ltItem = dynamic_cast<ImagePreviewBarItem*>(item);

            if (ltItem)
            {
                if (ltItem->info() == info)
                {
                    return ltItem;
                }
            }
        }
    }

    return 0;
}

ImagePreviewBarItem* ImagePreviewBar::findItemByPos(const QPoint& pos) const
{
    ThumbBarItem* item = findItem(pos);

    if (item)
    {
        ImagePreviewBarItem* ltItem = dynamic_cast<ImagePreviewBarItem*>(item);
        return ltItem;
    }

    return 0;
}

void ImagePreviewBar::applySettings()
{
    readToolTipSettings();

    MetadataSettings* mSettings = MetadataSettings::instance();

    if (!mSettings)
    {
        return;
    }

    setExifRotate(mSettings->settings().exifRotate);
}

void ImagePreviewBar::readToolTipSettings()
{
    AlbumSettings* albumSettings = AlbumSettings::instance();

    if (!albumSettings)
    {
        return;
    }

    ThumbBarToolTipSettings settings;
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

void ImagePreviewBar::startDrag()
{
    if (!currentItem())
    {
        return;
    }

    KUrl::List urls;
    KUrl::List kioURLs;
    QList<int> albumIDs;
    QList<qlonglong> imageIDs;

    ImagePreviewBarItem* item = dynamic_cast<ImagePreviewBarItem*>(currentItem());

    urls.append(item->info().fileUrl());
    kioURLs.append(item->info().databaseUrl());
    imageIDs.append(item->info().id());
    albumIDs.append(item->info().albumId());

    QPixmap icon;

    if (pixmapForItem(item, icon))
    {
        icon = icon.scaled(48, 48, Qt::KeepAspectRatio);
    }
    else
    {
        icon = DesktopIcon("image-jp2", 48);
    }

    int w = icon.width();
    int h = icon.height();

    QPixmap pix(w+4, h+4);
    QPainter p(&pix);
    p.fillRect(0, 0, pix.width()-1, pix.height()-1, QColor(Qt::white));
    p.setPen(QPen(Qt::black, 1));
    p.drawRect(0, 0, pix.width()-1, pix.height()-1);
    p.drawPixmap(2, 2, icon);
    p.end();

    QDrag* drag = new QDrag(this);
    drag->setMimeData(new DItemDrag(urls, kioURLs, albumIDs, imageIDs));
    drag->setPixmap(pix);
    drag->exec();
}

void ImagePreviewBar::contentsMouseMoveEvent(QMouseEvent* e)
{
    if (!e)
    {
        return;
    }

    if (e->buttons() == Qt::NoButton)
    {
        ImagePreviewBarItem* item = dynamic_cast<ImagePreviewBarItem*>(findItem(e->pos()));

        if (item)
        {
            QRect rect = clickToRateRect(item);

            if (rect.contains(e->pos()))
            {
                setCursor(Qt::CrossCursor);

                d->ratingItem = item;
                item->repaint();

                rect.moveTopLeft(contentsToViewport(rect.topLeft()));
                d->ratingWidget->setFixedSize(rect.size());
                d->ratingWidget->move(rect.topLeft().x()-1, rect.topLeft().y());
                d->ratingWidget->setRating(item->info().rating());
                d->ratingWidget->show();
            }
            else
            {
                unsetCursor();
                d->ratingWidget->hide();
                d->ratingItem = 0;
                item->repaint();
            }
        }
        else
        {
            unsetCursor();
            d->ratingWidget->hide();
            d->ratingItem = 0;
        }
    }

    ThumbBarView::contentsMouseMoveEvent(e);
}

void ImagePreviewBar::drawItem(ThumbBarItem* item, QPainter& p, QPixmap& tile)
{
    Q_UNUSED(tile);

    if (item != d->ratingItem)
    {
        ImagePreviewBarItem* rItem = dynamic_cast<ImagePreviewBarItem*>(item);
        int rating                 = rItem->info().rating();
        QRect r                    = clickToRateRect(rItem);

        if (getOrientation() == Qt::Vertical)
        {
            r.translate(0, -rItem->position());
        }
        else
        {
            r.translate(-rItem->position(), 0);
        }

        r.setX(((r.right() - rating * d->ratingPixmap.width())/2) - 1);
        r.setY(r.y()+1);
        r.setWidth((rating * d->ratingPixmap.width()));
        r.setBottom(r.bottom()+1);
        p.drawTiledPixmap(r, d->ratingPixmap);
    }
}

void ImagePreviewBar::drawEmptyMessage(QPixmap& pixmap)
{
    Q_UNUSED(pixmap)
}

void ImagePreviewBar::viewportPaintEvent(QPaintEvent* e)
{
    ThemeEngine* te = ThemeEngine::instance();
    QRect    er(e->rect());
    QPixmap  bgPix;

    if (countItems() > 0)
    {
        int cy=0, cx=0, ts=0, y1=0, y2=0, x1=0, x2=0;
        QPixmap  tile;

        if (getOrientation() == Qt::Vertical)
        {
            cy    = viewportToContents(er.topLeft()).y();

            bgPix = QPixmap(contentsRect().width(), er.height());

            ts    = getTileSize() + 2*getMargin() + 2*getRadius();
            tile  = QPixmap(visibleWidth()-1, ts-1);

            y1    = (cy/ts)*ts;
            y2    = ((y1 + er.height())/ts +1)*ts;
        }
        else
        {
            cx    = viewportToContents(er.topLeft()).x();

            bgPix = QPixmap(er.width(), contentsRect().height());

            ts    = getTileSize() + 2*getMargin() + 2*getRadius();
            tile  = QPixmap(ts-1, visibleHeight()-1);

            x1    = (cx/ts)*ts;
            x2    = ((x1 + er.width())/ts +1)*ts;
        }

        bgPix.fill(te->baseColor());

        for (ThumbBarItem* item = firstItem(); item; item = item->next())
        {
            if (getOrientation() == Qt::Vertical)
            {
                if (y1 <= item->position() && item->position() <= y2)
                {
                    if (item == currentItem())
                    {
                        tile.fill(kapp->palette().color(QPalette::Highlight));
                        QPainter p(&tile);
                        p.setPen(kapp->palette().color(QPalette::Midlight));
                        p.drawRect(0, 0, tile.width(), tile.height());
                    }
                    else
                    {
                        tile.fill(kapp->palette().color(QPalette::Base));
                        QPainter p(&tile);
                        p.setPen(kapp->palette().color(QPalette::Midlight));
                        p.drawRect(0, 0, tile.width(), tile.height());
                    }

                    QPainter p(&tile);

                    if (item == currentItem())
                    {
                        p.setPen(QPen(te->textSelColor(), 3));
                        p.drawRect(1, 1, tile.width()-2, tile.height()-2);
                    }
                    else
                    {
                        p.setPen(QPen(te->textRegColor(), 1));
                        p.drawRect(0, 0, tile.width()-1, tile.height()-1);
                    }

                    if (item == highlightedItem())
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
                        p.drawPixmap(x-3, y-3, generateFuzzyRect(QSize(pix.width()+6, pix.height()+6),
                                                                 QColor(0, 0, 0, 128), 3));
                        item->setTooltipRect(QRect(x, y+item->position(), pix.width(), pix.height()));

                        drawItem(item, p, tile);
                    }

                    p.end();

                    QPainter p2(&bgPix);
                    p2.drawPixmap(0, item->position() - cy, tile);
                    p2.end();
                }
            }
            else
            {
                if (x1 <= item->position() && item->position() <= x2)
                {
                    if (item == currentItem())
                    {
                        tile.fill(kapp->palette().color(QPalette::Highlight));
                        QPainter p(&tile);
                        p.setPen(kapp->palette().color(QPalette::Midlight));
                        p.drawRect(0, 0, tile.width(), tile.height());
                    }
                    else
                    {
                        tile.fill(kapp->palette().color(QPalette::Base));
                        QPainter p(&tile);
                        p.setPen(kapp->palette().color(QPalette::Midlight));
                        p.drawRect(0, 0, tile.width(), tile.height());
                    }

                    QPainter p(&tile);

                    if (item == currentItem())
                    {
                        p.setPen(QPen(te->textSelColor(), 3));
                        p.drawRect(1, 1, tile.width()-2, tile.height()-2);
                    }
                    else
                    {
                        p.setPen(QPen(te->textRegColor(), 1));
                        p.drawRect(0, 0, tile.width()-1, tile.height()-1);
                    }

                    if (item == highlightedItem())
                    {
                        QRect r = item->rect();
                        p.setPen(QPen(palette().color(QPalette::Highlight), 3, Qt::SolidLine));
                        p.drawRect(1, 1, r.width()-3, r.height()-3);
                    }

                    QPixmap pix;

                    if (pixmapForItem(item, pix))
                    {
                        int x = (tile.width() - pix.width())/2;
                        int y = (tile.height()- pix.height())/2;
                        p.drawPixmap(x, y, pix);
                        p.drawPixmap(x-3, y-3, generateFuzzyRect(QSize(pix.width()+6, pix.height()+6),
                                                                 QColor(0, 0, 0, 128), 3));
                        item->setTooltipRect(QRect(x+item->position(), y, pix.width(), pix.height()));

                        drawItem(item, p, tile);
                    }

                    p.end();

                    QPainter p2(&bgPix);
                    p2.drawPixmap(item->position() - cx, 0, tile);
                    p2.end();
                }
            }
        }

        QPainter p3(viewport());

        if (getOrientation() == Qt::Vertical)
        {
            p3.drawPixmap(0, er.y(), bgPix);
        }
        else
        {
            p3.drawPixmap(er.x(), 0, bgPix);
        }

        p3.end();
    }
    else
    {
        bgPix = QPixmap(contentsRect().width(), contentsRect().height());
        bgPix.fill(te->baseColor());

        drawEmptyMessage(bgPix);

        QPainter p5(viewport());
        p5.drawPixmap(0, 0, bgPix);
        p5.end();
    }

    checkPreload();
}

void ImagePreviewBar::slotThemeChanged()
{
    QPainter painter(&d->ratingPixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setBrush(kapp->palette().color(QPalette::Link));
    painter.setPen(ThemeEngine::instance()->textRegColor());
    painter.drawPolygon(d->starPolygon, Qt::WindingFill);
    painter.end();
    slotUpdate();
}

// NOTE: see B.K.O #181184 : we need to catch mouse leave event from rating
//       box when user move cursor over scrollbar.

bool ImagePreviewBar::eventFilter(QObject* obj, QEvent* ev)
{
    if ( obj == d->ratingWidget )
    {
        if ( ev->type() == QEvent::Leave)
        {
            // Cave: ratingWidget->hide can recurse here again! See bug 184473
            ThumbBarItem* item = d->ratingItem;

            if (item)
            {
                unsetCursor();
                d->ratingItem = 0;
                d->ratingWidget->hide();
                item->repaint();
            }
        }
    }

    // pass the event on to the parent class
    return ThumbBarView::eventFilter(obj, ev);
}

QRect ImagePreviewBar::clickToRateRect(ImagePreviewBarItem* item)
{
    QRect r    = item->rect();
    int top    = r.bottom() - getMargin() - ratingPixmap().height() - 2;
    int left   = r.left() - 1;
    int bottom = r.bottom() - getMargin() - 2;
    int right  = r.right() - 1;
    return QRect(left, top, right-left, bottom-top);
}

// -------------------------------------------------------------------------

ImagePreviewBarItem::ImagePreviewBarItem(ImagePreviewBar* view, const ImageInfo& info)
    : ThumbBarItem(view, info.fileUrl())
{
    m_info = info;
}

ImagePreviewBarItem::~ImagePreviewBarItem()
{
}

ImageInfo ImagePreviewBarItem::info()
{
    return m_info;
}

// -------------------------------------------------------------------------

ImagePreviewBarToolTip::ImagePreviewBarToolTip(ThumbBarView* parent)
    : ThumbBarToolTip(parent)
{
}

ImagePreviewBarToolTip::~ImagePreviewBarToolTip()
{
}

QString ImagePreviewBarToolTip::tipContents()
{
    if (!item())
    {
        return QString();
    }

    ImageInfo info = dynamic_cast<ImagePreviewBarItem*>(item())->info();
    return ToolTipFiller::imageInfoTipContents(info);
}

// -------------------------------------------------------------------------

class LightTableBar::LightTableBarPriv
{

public:

    LightTableBarPriv() :
        navigateByPair(false)
    {
    }

    bool navigateByPair;
};

LightTableBar::LightTableBar(QWidget* parent, int orientation, bool exifRotate)
    : ImagePreviewBar(parent, orientation, exifRotate),
      d(new LightTableBarPriv)
{
    connect(this, SIGNAL(signalItemSelected(ThumbBarItem*)),
            this, SLOT(slotItemSelected(ThumbBarItem*)));

    connect(DatabaseAccess::databaseWatch(), SIGNAL(collectionImageChange(const CollectionImageChangeset&)),
            this, SLOT(slotCollectionImageChange(const CollectionImageChangeset&)),
            Qt::QueuedConnection);
}

LightTableBar::~LightTableBar()
{
    delete d;
}

void LightTableBar::setNavigateByPair(bool b)
{
    d->navigateByPair = b;
}

void LightTableBar::contentsMouseReleaseEvent(QMouseEvent* e)
{
    if (!e)
    {
        return;
    }

    ThumbBarView::contentsMouseReleaseEvent(e);

    QPoint pos = QCursor::pos();
    LightTableBarItem* item = dynamic_cast<LightTableBarItem*>(findItemByPos(e->pos()));

    if (e->button() == Qt::RightButton)
    {
        // temporary actions ----------------------------------

        QAction* leftPanelAction, *rightPanelAction, *editAction, *removeAction, *clearAllAction = 0;

        leftPanelAction  = new QAction(SmallIcon("arrow-left"),         i18n("Show on left panel"), this);
        rightPanelAction = new QAction(SmallIcon("arrow-right"),        i18n("Show on right panel"), this);
        editAction       = new QAction(SmallIcon("editimage"),          i18n("Edit"), this);
        removeAction     = new QAction(SmallIcon("dialog-close"),       i18n("Remove item"), this);
        clearAllAction   = new QAction(SmallIcon("edit-delete-shred"),  i18n("Clear all"), this);

        leftPanelAction->setEnabled(d->navigateByPair  ? false : true);
        rightPanelAction->setEnabled(d->navigateByPair ? false : true);
        clearAllAction->setEnabled(itemsUrls().count() ? true  : false);

        // ----------------------------------------------------

        KMenu popmenu(this);
        ContextMenuHelper cmhelper(&popmenu);

        if (item)
        {
            cmhelper.addAction(leftPanelAction, true);
            cmhelper.addAction(rightPanelAction, true);
            cmhelper.addSeparator();
            // ------------------------------------------------
            cmhelper.addAction(editAction);
            cmhelper.addAction(removeAction);
            cmhelper.addSeparator();
            // ------------------------------------------------
            cmhelper.addLabelsAction();
            cmhelper.addSeparator();
            // ------------------------------------------------
        }

        cmhelper.addAction(clearAllAction, true);

        // special action handling --------------------------------

        connect(&cmhelper, SIGNAL(signalAssignPickLabel(int)),
                this, SLOT(slotAssignPickLabel(int)));

        connect(&cmhelper, SIGNAL(signalAssignColorLabel(int)),
                this, SLOT(slotAssignColorLabel(int)));

        connect(&cmhelper, SIGNAL(signalAssignRating(int)),
                this, SLOT(slotAssignRating(int)));

        QAction* choice = cmhelper.exec(pos);

        if (choice)
        {
            if (choice == leftPanelAction)
            {
                emit signalSetItemOnLeftPanel(item->info());
            }
            else if (choice == rightPanelAction)
            {
                emit signalSetItemOnRightPanel(item->info());
            }
            else if (choice == editAction)
            {
                emit signalEditItem(item->info());
            }
            else if (choice == removeAction)
            {
                emit signalRemoveItem(item->info());
            }
            else if (choice == clearAllAction)
            {
                emit signalClearAll();
            }
        }
    }
}

void LightTableBar::slotAssignPickLabel(int pickId)
{
    assignPickLabel(currentItemImageInfo(), pickId);
}

void LightTableBar::slotAssignColorLabel(int colorId)
{
    assignColorLabel(currentItemImageInfo(), colorId);
}

void LightTableBar::assignPickLabel(const ImageInfo& info, int pickId)
{
    if (!info.isNull())
    {
        MetadataHub hub;
        hub.load(info);
        hub.setPickLabel(pickId);
        hub.write(info, MetadataHub::PartialWrite);
        hub.write(info.filePath(), MetadataHub::FullWriteIfChanged);
    }
}

void LightTableBar::assignColorLabel(const ImageInfo& info, int colorId)
{
    if (!info.isNull())
    {
        MetadataHub hub;
        hub.load(info);
        hub.setColorLabel(colorId);
        hub.write(info, MetadataHub::PartialWrite);
        hub.write(info.filePath(), MetadataHub::FullWriteIfChanged);
    }
}

void LightTableBar::slotPickLabelChanged(const KUrl& url, int pick)
{
    assignPickLabel(ImageInfo(url), pick);
}

void LightTableBar::slotColorLabelChanged(const KUrl& url, int color)
{
    assignColorLabel(ImageInfo(url), color);
}

void LightTableBar::slotRatingChanged(const KUrl& url, int rating)
{
    assignRating(ImageInfo(url), rating);
}

void LightTableBar::slotAssignRating(int rating)
{
    assignRating(currentItemImageInfo(), rating);
}

void LightTableBar::assignRating(const ImageInfo& info, int rating)
{
    rating = qMin(5, qMax(0, rating));

    if (!info.isNull())
    {
        MetadataHub hub;
        hub.load(info);
        hub.setRating(rating);
        hub.write(info, MetadataHub::PartialWrite);
        hub.write(info.filePath(), MetadataHub::FullWriteIfChanged);
    }
}

void LightTableBar::toggleTag(int tagID)
{
    ImageInfo info = currentItemImageInfo();

    if (!info.isNull())
    {
        MetadataHub hub;
        hub.load(info);
        hub.setTag(tagID, !info.tagIds().contains(tagID));
        hub.write(info, MetadataHub::PartialWrite);
        hub.write(info.filePath(), MetadataHub::FullWriteIfChanged);
    }
}

void LightTableBar::setOnLeftPanel(const ImageInfo& info)
{
    for (ThumbBarItem* item = firstItem(); item; item = item->next())
    {
        LightTableBarItem* ltItem = dynamic_cast<LightTableBarItem*>(item);

        if (ltItem)
        {
            if (!info.isNull())
            {
                if (ltItem->info() == info)
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

void LightTableBar::setOnRightPanel(const ImageInfo& info)
{
    for (ThumbBarItem* item = firstItem(); item; item = item->next())
    {
        LightTableBarItem* ltItem = dynamic_cast<LightTableBarItem*>(item);

        if (ltItem)
        {
            if (!info.isNull())
            {
                if (ltItem->info() == info)
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
        LightTableBarItem* ltItem = dynamic_cast<LightTableBarItem*>(item);

        if (ltItem)
        {
            emit signalLightTableBarItemSelected(ltItem->info());
            return;
        }
    }

    emit signalLightTableBarItemSelected(ImageInfo());
}

void LightTableBar::removeItemByInfo(const ImageInfo& info)
{
    if (info.isNull())
    {
        return;
    }

    ImagePreviewBarItem* ltItem = findItemByInfo(info);
    ThumbBarItem* item          = dynamic_cast<ThumbBarItem*>(ltItem);

    if (item)
    {
        removeItem(item);
    }
}

void LightTableBar::removeItemById(qlonglong id)
{
    ImagePreviewBarItem* item = findItemById(id);

    if (item)
    {
        removeItem(item);
    }
}

ImagePreviewBarItem* LightTableBar::findItemById(qlonglong id) const
{
    for (ThumbBarItem* item = firstItem(); item; item = item->next())
    {
        ImagePreviewBarItem* ltItem = dynamic_cast<ImagePreviewBarItem*>(item);

        if (ltItem)
        {
            if (ltItem->info().id() == id)
            {
                return ltItem;
            }
        }
    }

    return 0;
}

void LightTableBar::drawItem(ThumbBarItem* item, QPainter& p, QPixmap& tile)
{
    LightTableBarItem* rItem = dynamic_cast<LightTableBarItem*>(item);
    int pickId               = rItem->info().pickLabel();
    int colorId              = rItem->info().colorLabel();

    if (colorId > NoColorLabel)
    {

        QRect r = item->rect();
        p.setPen(QPen(ColorLabelWidget::labelColor((ColorLabel)colorId), 5, Qt::SolidLine));
        p.drawRect(3, 3, r.width()-7, r.height()-7);
    }

    if (pickId != NoPickLabel)
    {
        QIcon icon;
        int size = KIconLoader::SizeSmallMedium;

        if (pickId == RejectedLabel)
        {
            icon = KIconLoader::global()->loadIcon("flag-red", KIconLoader::NoGroup, size);
        }
        else if (pickId == PendingLabel)
        {
            icon = KIconLoader::global()->loadIcon("flag-yellow", KIconLoader::NoGroup, size);
        }
        else if (pickId == AcceptedLabel)
        {
            icon = KIconLoader::global()->loadIcon("flag-green", KIconLoader::NoGroup, size);
        }

        icon.paint(&p, item->rect().width()/2 - size/2, 10, size, size);
    }

    if (rItem->isOnLeftPanel())
    {
        QPixmap lPix = SmallIcon("arrow-left");
        p.drawPixmap(getMargin(), getMargin(), lPix);
    }

    if (rItem->isOnRightPanel())
    {
        QPixmap rPix = SmallIcon("arrow-right");
        p.drawPixmap(tile.width() - getMargin() - rPix.width(), getMargin(), rPix);
    }

    if (item != ratingItem())
    {
        int rating = rItem->info().rating();
        QRect r    = clickToRateRect(rItem);

        if (getOrientation() == Qt::Vertical)
        {
            r.translate(0, -rItem->position());
        }
        else
        {
            r.translate(-rItem->position(), 0);
        }

        r.setX(((r.right() - rating * ratingPixmap().width())/2) - 1);
        r.setY(r.y()+1);
        r.setWidth((rating * ratingPixmap().width()));
        r.setBottom(r.bottom()+1);
        p.drawTiledPixmap(r, ratingPixmap());
    }
}

void LightTableBar::drawEmptyMessage(QPixmap& bgPix)
{
    ThemeEngine* te = ThemeEngine::instance();

    QPainter p4(&bgPix);
    p4.setPen(QPen(te->textRegColor()));
    p4.drawText(0, 0, bgPix.width(), bgPix.height(),
                Qt::AlignCenter|Qt::TextWordWrap,
                i18n("Drag and drop images here"));
    p4.end();
}

void LightTableBar::startDrag()
{
    if (!currentItem())
    {
        return;
    }

    KUrl::List urls;
    KUrl::List kioURLs;
    QList<int> albumIDs;
    QList<qlonglong> imageIDs;

    LightTableBarItem* item = dynamic_cast<LightTableBarItem*>(currentItem());

    urls.append(item->info().fileUrl());
    kioURLs.append(item->info().databaseUrl());
    imageIDs.append(item->info().id());
    albumIDs.append(item->info().albumId());

    QPixmap icon(DesktopIcon("image-jp2", 48));
    int w = icon.width();
    int h = icon.height();

    QPixmap pix(w+4, h+4);
    QPainter p(&pix);
    p.fillRect(0, 0, pix.width()-1, pix.height()-1, QColor(Qt::white));
    p.setPen(QPen(Qt::black, 1));
    p.drawRect(0, 0, pix.width()-1, pix.height()-1);
    p.drawPixmap(2, 2, icon);
    p.end();

    QDrag* drag = new QDrag(this);
    drag->setMimeData(new DItemDrag(urls, kioURLs, albumIDs, imageIDs));
    drag->setPixmap(pix);
    drag->exec();
}

void LightTableBar::contentsDragEnterEvent(QDragEnterEvent* e)
{
    int        albumID;
    QList<int> albumIDs;
    QList<qlonglong> imageIDs;
    KUrl::List urls;
    KUrl::List kioURLs;

    if (DItemDrag::decode(e->mimeData(), urls, kioURLs, albumIDs, imageIDs) ||
        DAlbumDrag::decode(e->mimeData(), urls, albumID) ||
        DTagDrag::canDecode(e->mimeData()))
    {
        e->accept();
        return;
    }

    e->ignore();
}

void LightTableBar::contentsDropEvent(QDropEvent* e)
{
    int        albumID;
    QList<int> albumIDs;
    QList<qlonglong> imageIDs;
    KUrl::List urls;
    KUrl::List kioURLs;

    if (DItemDrag::decode(e->mimeData(), urls, kioURLs, albumIDs, imageIDs))
    {
        ImageInfoList imageInfoList;

        for (QList<qlonglong>::const_iterator it = imageIDs.constBegin();
             it != imageIDs.constEnd(); ++it)
        {
            ImageInfo info(*it);

            if (!findItemByInfo(info))
            {
                imageInfoList.append(info);
            }
        }

        emit signalDroppedItems(imageInfoList);
        e->accept();
    }
    else if (DAlbumDrag::decode(e->mimeData(), urls, albumID))
    {
        QList<qlonglong> itemIDs = DatabaseAccess().db()->getItemIDsInAlbum(albumID);
        ImageInfoList imageInfoList;

        for (QList<qlonglong>::const_iterator it = itemIDs.constBegin();
             it != itemIDs.constEnd(); ++it)
        {
            ImageInfo info(*it);

            if (!findItemByInfo(info))
            {
                imageInfoList.append(info);
            }
        }

        emit signalDroppedItems(imageInfoList);
        e->accept();
    }
    else if (DTagDrag::canDecode(e->mimeData()))
    {
        int tagID;

        if (!DTagDrag::decode(e->mimeData(), tagID))
        {
            return;
        }

        QList<qlonglong> itemIDs = DatabaseAccess().db()->getItemIDsInTag(tagID, true);
        ImageInfoList imageInfoList;

        for (QList<qlonglong>::const_iterator it = itemIDs.constBegin();
             it != itemIDs.constEnd(); ++it)
        {
            ImageInfo info(*it);

            if (!findItemByInfo(info))
            {
                imageInfoList.append(info);
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

void LightTableBar::slotCollectionImageChange(const CollectionImageChangeset& changeset)
{
    switch (changeset.operation())
    {
        case CollectionImageChangeset::Removed:
        case CollectionImageChangeset::RemovedAll:
        {
            ImageInfo info;
            foreach (const qlonglong& id, changeset.ids())
            {
                ImagePreviewBarItem* item = findItemById(id);

                if (item)
                {
                    info = item->info();
                    removeItem(item);
                    emit signalRemoveItem(info);
                }
            }
            break;
        }
        default:
        {
            break;
        }
    }
}

// -------------------------------------------------------------------------

class LightTableBarItem::LightTableBarItemPriv
{

public:

    LightTableBarItemPriv() :
        onLeftPanel(false),
        onRightPanel(false)
    {
    }

    bool onLeftPanel;
    bool onRightPanel;
};

LightTableBarItem::LightTableBarItem(LightTableBar* view, const ImageInfo& info)
    : ImagePreviewBarItem(view, info.fileUrl()),
      d(new LightTableBarItemPriv)
{
}

LightTableBarItem::~LightTableBarItem()
{
    delete d;
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

}  // namespace Digikam
