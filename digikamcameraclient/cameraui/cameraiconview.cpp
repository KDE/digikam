/* ============================================================
 * File  : cameraiconview.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2003-01-23
 * Description : 
 * 
 * Copyright 2003 by Renchi Raju

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

#include <qimage.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qrect.h>
#include <qpalette.h>
#include <qpainter.h>
#include <qimage.h>
#include <qpoint.h>
#include <qpopupmenu.h>
#include <qvaluevector.h>

#include <klocale.h>
#include <kiconloader.h>
#include <kstddirs.h>
#include <ktrader.h>
#include <kservice.h>

#include "gpfileiteminfo.h"
#include "gpfileiteminfodlg.h"
#include "thumbnailsize.h"
#include "cameraiconitem.h"
#include "cameraiconview.h"
#include "cameradragobject.h"
#include "cameraui.h"

const int MAXICONITEMS = 307;

class CameraIconViewPrivate
{
public:

    ThumbnailSize thumbSize;
    QPixmap imagePix;
    QPixmap audioPix;
    QPixmap videoPix;
    QPixmap unknownPix;

};

CameraIconView::CameraIconView(QWidget *parent)
    : ThumbView(parent)
{
    d = new CameraIconViewPrivate;

    connect(this, SIGNAL(signalDoubleClicked(ThumbItem*)),
            this, SLOT(slotDoubleClicked(ThumbItem*)));
    connect(this, SIGNAL(signalRightButtonClicked(ThumbItem*, const QPoint&)),
            this, SLOT(slotRightButtonClicked(ThumbItem*, const QPoint&)));
}

CameraIconView::~CameraIconView()
{
    delete d;    
}

void CameraIconView::setThumbnailSize(const ThumbnailSize& thumbSize)
{
    d->thumbSize = thumbSize;
    
    int w = thumbSize.size();
    int h = thumbSize.size();

    QString iconfile = locate("appdata", "icons/generic.png");
    QImage image(iconfile);

    double scale = double(w-10)/double(image.width());
    image = image.smoothScale(w-10, h-10, QImage::ScaleMin);

    QPixmap pix(w, h);
    pix.fill(colorGroup().base());
    QPainter p(&pix);
    p.fillRect(0, 0, w, h, QBrush(colorGroup().base()));
    if (!image.isNull())
        p.drawImage((w-image.width())/2, (h-image.height())/2, image);
    p.end();

    d->imagePix = pix;
    createPixmap(d->imagePix, "icons/pictures.png", scale);
    d->audioPix = pix;
    createPixmap(d->audioPix, "icons/sound.png", scale);
    d->videoPix = pix;
    createPixmap(d->videoPix, "icons/multimedia.png", scale);
    d->unknownPix = pix;
    createPixmap(d->unknownPix, "icons/documents.png", scale);
    

}

void CameraIconView::createPixmap(QPixmap& pix, const QString& icon,
                                  double scale)
{
    QString iconfile = locate("appdata", icon);
    QImage mimeImg(iconfile);
    
    mimeImg = mimeImg.smoothScale((int) (mimeImg.width()*scale),
                                  (int) (mimeImg.height()*scale),
                                  QImage::ScaleMin);

    int w = d->thumbSize.size();
    int h = d->thumbSize.size();
    
    QPainter p(&pix);
    if (!mimeImg.isNull())
        p.drawImage((w-mimeImg.width())/2, (h-mimeImg.height())/2, mimeImg);
    p.end();
    
}

CameraIconItem* CameraIconView::addItem(const GPFileItemInfo* fileInfo)
{
 
    QPixmap& pix = d->unknownPix;

    if (fileInfo->mime.contains("image"))
        pix = d->imagePix;
    else if (fileInfo->mime.contains("audio"))
        pix = d->audioPix;
    else if (fileInfo->mime.contains("video"))
        pix = d->videoPix;
    else
        pix = d->unknownPix;


    CameraIconItem *iconItem =
        new CameraIconItem(this, fileInfo, pix);

    return iconItem;
}

void CameraIconView::clear()
{
    ThumbView::clear();
    emit signalCleared();
}


void CameraIconView::setThumbnail(CameraIconItem* iconItem,
                                  const QImage& thumbnail)
{
    if (!iconItem) return;
    iconItem->setPixmap(thumbnail);
}

void CameraIconView::calcBanner()
{
    setBannerRect(QRect(0, 0, 0, 0));    
}

void CameraIconView::paintBanner(QPainter*)
{
    
}

void CameraIconView::markDownloaded(CameraIconItem* iconItem)
{
    if (!iconItem) return;

    GPFileItemInfo *fileInfo = const_cast<GPFileItemInfo*>(iconItem->fileInfo());
    
    fileInfo->downloaded = 1;
    iconItem->repaint();
}

void CameraIconView::slotRightButtonClicked(ThumbItem *item,
                                            const QPoint& pos)
{
    if (!item) return;

    CameraIconItem *camItem =
        static_cast<CameraIconItem *>(item);

    // --------------------------------------------------------

    QValueVector<KService::Ptr> serviceVector;
    KTrader::OfferList offers =
        KTrader::self()->query(camItem->fileInfo()->mime,
                               "Type == 'Application'");
    
    QPopupMenu *openWithMenu = new QPopupMenu();

    KTrader::OfferList::Iterator iter;
    KService::Ptr ptr;
    int index = 100;
    for( iter = offers.begin(); iter != offers.end(); ++iter ) {
        ptr = *iter;
        openWithMenu->insertItem( ptr->pixmap(KIcon::Small),
                                  ptr->name(), index++);
        serviceVector.push_back(ptr);
    }
    
    // --------------------------------------------------------
    
    QPopupMenu popmenu(this);
    popmenu.insertItem(SmallIcon("image"),
                       i18n("View"), 10);
    popmenu.insertItem(i18n("Open With ..."), openWithMenu, 11);
    popmenu.insertSeparator(12);
    popmenu.insertItem(SmallIcon("filesave"),
                       i18n("Download"), 13);
    popmenu.insertItem(SmallIcon("editdelete"),
                       i18n("Delete"), 14);
    popmenu.insertSeparator(15);
    popmenu.insertItem(i18n("Exif Information ..."), 16);
    popmenu.insertItem(i18n("Properties ..."), 17);

    // --------------------------------------------------------

    int id = popmenu.exec(pos);

    switch(id) {

    case (10): {
        emit signalOpenItem(camItem->fileInfo()->folder,
                        camItem->fileInfo()->name);
        break;
    }


    case (13): {
        emit signalDownloadSelectedItems();
        break;
    }

    case (14): {
        emit signalDeleteSelectedItems();
        break;
    }

    case (16): {
        emit signalExifInformation(camItem->fileInfo()->folder,
                                   camItem->fileInfo()->name);
        break;
    }
        

    case (17): {
        slotItemInformation(camItem);
        break;
    }

    default:
        break;
    }

    //---------------------------------------------------------------------------

    KService::Ptr camItemServicePtr = 0;

    if (id >= 100) {
        camItemServicePtr = serviceVector[id-100];
        emit signalOpenItem(camItem->fileInfo()->folder,
                            camItem->fileInfo()->name,
                            (*camItemServicePtr).desktopEntryName());
    }

    //---------------------------------------------------------------------------

    serviceVector.clear();
    delete openWithMenu;
    
    
}

void CameraIconView::slotDoubleClicked(ThumbItem *item)
{
    if (!item) return;

    CameraIconItem *camItem =
        static_cast<CameraIconItem *>(item);
    
    emit signalOpenItem(camItem->fileInfo()->folder,
                        camItem->fileInfo()->name);
}

void CameraIconView::slotItemInformation(CameraIconItem *item)
{
    if (!item) return;    

    GPFileItemInfoDlg *infoDlg =
        new GPFileItemInfoDlg(*(item->fileInfo()),
                              item->pixmap());
    infoDlg->show();
}

void CameraIconView::startDrag()
{
    int count = 0;
    for (ThumbItem *it=firstItem(); it; it=it->nextItem())
        if (it->isSelected())
            count++;

    if (count == 0) return;
    
    CameraDragObject* drag =
        new CameraDragObject(CameraUI::getInstance()->cameraType(),
                             this);
    drag->setPixmap(SmallIcon("image"));
    drag->dragCopy();
}
