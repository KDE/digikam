/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-09-18
 * Description : camera icon view
 * 
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <qfile.h>
#include <qfileinfo.h>
#include <qtimer.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qcursor.h>
#include <qfontmetrics.h>
#include <qfont.h>
#include <qdragobject.h>
#include <qclipboard.h>

// KDE includes.

#include <kurldrag.h>
#include <kmimetype.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kaction.h>
#include <kapplication.h>

// Local includes.

#include "ddebug.h"
#include "themeengine.h"
#include "thumbnailsize.h"
#include "gpiteminfo.h"
#include "renamecustomizer.h"
#include "icongroupitem.h"
#include "dpopupmenu.h"
#include "dragobjects.h"
#include "cameraui.h"
#include "cameraiconitem.h"
#include "cameraiconview.h"
#include "cameraiconview.moc"

namespace Digikam
{

class CameraIconViewPriv
{
public:

    CameraIconViewPriv()
    {
        renamer             = 0;
        groupItem           = 0;
        cameraUI            = 0;
        thumbSize           = ThumbnailSize::Large;
        pixmapNewPicture    = QPixmap(newPicture_xpm);
        pixmapUnknowPicture = QPixmap(unknowPicture_xpm);
    }

    static const char         *newPicture_xpm[];
    static const char         *unknowPicture_xpm[];

    QDict<CameraIconViewItem>  itemDict;

    QRect                      itemRect;

    QPixmap                    itemRegPixmap;
    QPixmap                    itemSelPixmap;
    QPixmap                    pixmapNewPicture;
    QPixmap                    pixmapUnknowPicture;

    RenameCustomizer          *renamer;

    IconGroupItem             *groupItem;

    ThumbnailSize              thumbSize;

    CameraUI                  *cameraUI;
};

const char *CameraIconViewPriv::newPicture_xpm[] =
{
    "13 13 8 1",
    "       c None",
    ".      c #232300",
    "+      c #F6F611",
    "@      c #000000",
    "#      c #DBDA4D",
    "$      c #FFFF00",
    "%      c #AAA538",
    "&      c #E8E540",
    "      .      ",
    "  .  .+.  .  ",
    " @#@ .$. .#. ",
    "  @$@#$#@$.  ",
    "   @$%&%$@   ",
    " ..#%&&&%#.. ",
    ".+$$&&&&&$$+@",
    " ..#%&&&%#@@ ",
    "   @$%&%$@   ",
    "  .$@#$#@$.  ",
    " @#. @$@ @#. ",
    "  .  @+@  .  ",
    "      @      "
};

const char *CameraIconViewPriv::unknowPicture_xpm[] = 
{
    "16 16 78 1",
    "   g None",
    ".  g #777777",
    "+  g #7A7A7A",
    "@  g #8C8C8C",
    "#  g #787878",
    "$  g #707070",
    "%  g #878787",
    "&  g #C3C3C3",
    "*  g #EAEAEA",
    "=  g #E4E4E4",
    "-  g #E2E2E2",
    ";  g #E6E6E6",
    ">  g #CECECE",
    ",  g #888888",
    "'  g #6B6B6B",
    ")  g #969696",
    "!  g #DEDEDE",
    "~  g #D8D8D8",
    "{  g #FFFFFF",
    "]  g #F2F2F2",
    "^  g #DFDFDF",
    "/  g #9D9D9D",
    "(  g #686868",
    "_  g #848484",
    ":  g #D0D0D0",
    "<  g #F1F1F1",
    "[  g #F0F0F0",
    "}  g #EBEBEB",
    "|  g #FDFDFD",
    "1  g #DDDDDD",
    "2  g #D4D4D4",
    "3  g #838383",
    "4  g #ABABAB",
    "5  g #C8C8C8",
    "6  g #CCCCCC",
    "7  g #F4F4F4",
    "8  g #D6D6D6",
    "9  g #E8E8E8",
    "0  g #C4C4C4",
    "a  g #A4A4A4",
    "b  g #656565",
    "c  g #B4B4B4",
    "d  g #B9B9B9",
    "e  g #BDBDBD",
    "f  g #B7B7B7",
    "g  g #898989",
    "h  g #6D6D6D",
    "i  g #808080",
    "j  g #AAAAAA",
    "k  g #A9A9A9",
    "l  g #737373",
    "m  g #7F7F7F",
    "n  g #9A9A9A",
    "o  g #D3D3D3",
    "p  g #909090",
    "q  g #727272",
    "r  g #8F8F8F",
    "s  g #8E8E8E",
    "t  g #8D8D8D",
    "u  g #EEEEEE",
    "v  g #FAFAFA",
    "w  g #929292",
    "x  g #C5C5C5",
    "y  g #5F5F5F",
    "z  g #989898",
    "A  g #CFCFCF",
    "B  g #9C9C9C",
    "C  g #A0A0A0",
    "D  g #FEFEFE",
    "E  g #ACACAC",
    "F  g #5E5E5E",
    "G  g #868686",
    "H  g #AFAFAF",
    "I  g #C1C1C1",
    "J  g #818181",
    "K  g #7E7E7E",
    "L  g #7B7B7B",
    "M  g #636363",
    "                ",
    "     .+@@#$     ",
    "   .%&*=-;>,'   ",
    "  .)!~={{]^-/(  ",
    "  _::<{[}|{123  ",
    " .456{7558{90ab ",
    " +cde96df={&g,h ",
    " ijjjjjk;{=@,,l ",
    " mnnnnno{-pgggq ",
    " #rprstuvwtttt' ",
    " $tpppp6xpppp@y ",
    "  mnnnzA~Bnnn.  ",
    "  'taaCD{Eaa,F  ",
    "   (GjHI0HjJF   ",
    "     (K,,LM     ",
    "                "
};

CameraIconView::CameraIconView(CameraUI* ui, QWidget* parent)
              : IconView(parent)
{
    d = new CameraIconViewPriv;
    d->cameraUI  = ui;
    d->groupItem = new IconGroupItem(this);

    setHScrollBarMode(QScrollView::AlwaysOff);
    setMinimumSize(400, 300);

    setAcceptDrops(true);
    viewport()->setAcceptDrops(true);

    // ----------------------------------------------------------------

    connect(this, SIGNAL(signalSelectionChanged()),
            this, SLOT(slotSelectionChanged()));

    connect(this, SIGNAL(signalNewSelection(bool)),
            this, SLOT(slotUpdateDownloadNames(bool)));

    connect(this, SIGNAL(signalRightButtonClicked(IconItem*, const QPoint&)),
            this, SLOT(slotContextMenu(IconItem*, const QPoint&)));

    connect(this, SIGNAL(signalRightButtonClicked(const QPoint &)),
            this, SLOT(slotRightButtonClicked(const QPoint &)));

    connect(this, SIGNAL(signalDoubleClicked(IconItem*)),
            this, SLOT(slotDoubleClicked(IconItem*)));

    connect(ThemeEngine::instance(), SIGNAL(signalThemeChanged()),
            this, SLOT(slotThemeChanged()));

    // ----------------------------------------------------------------

    updateItemRectsPixmap();
    slotThemeChanged();
}

CameraIconView::~CameraIconView()
{
    clear();
    delete d;
}

QPixmap* CameraIconView::itemBaseRegPixmap() const
{
    return &d->itemRegPixmap;
}

QPixmap* CameraIconView::itemBaseSelPixmap() const
{
    return &d->itemSelPixmap;
}

QPixmap CameraIconView::newPicturePixmap() const
{
    return d->pixmapNewPicture;
}

QPixmap CameraIconView::unknowPicturePixmap() const
{
    return d->pixmapUnknowPicture;
}

void CameraIconView::setRenameCustomizer(RenameCustomizer* renamer)
{
    d->renamer = renamer;
    
    connect(d->renamer, SIGNAL(signalChanged()),
            this, SLOT(slotDownloadNameChanged()));
}

void CameraIconView::addItem(const GPItemInfo& info)
{
    QImage thumb;
    // Just to have a generic image thumb from desktop with KDE < 3.5.0
    KMimeType::Ptr mime = KMimeType::mimeType(info.mime == QString("image/x-raw") ? 
                                              QString("image/tiff") : info.mime);

    if (mime)
    {
        thumb = QImage(mime->pixmap(KIcon::Desktop, ThumbnailSize::Huge, KIcon::DefaultState)
                       .convertToImage());
    }
    else
    {
        KIconLoader *iconLoader = KApplication::kApplication()->iconLoader();
        thumb = iconLoader->loadIcon("empty", KIcon::Desktop,
                                     ThumbnailSize::Huge, KIcon::DefaultState, 0, true)
                                     .convertToImage();
    }
    
    QString downloadName;

    if (d->renamer)
    {
        if (!d->renamer->useDefault())
        {
            downloadName = getTemplatedName( &info, d->itemDict.count() );
        }
        else
        {
            downloadName = getCasedName( d->renamer->changeCase(), &info);
        }
    }

    CameraIconViewItem* item = new CameraIconViewItem(d->groupItem, info, thumb, downloadName);
    d->itemDict.insert(info.folder+info.name, item);
}

void CameraIconView::removeItem(const QString& folder, const QString& file)
{
    CameraIconViewItem* item = d->itemDict.find(folder+file);
    if (!item)
        return;
    d->itemDict.remove(folder+file);

    setDelayedRearrangement(true);
    delete item;
    setDelayedRearrangement(false);
}

CameraIconViewItem* CameraIconView::findItem(const QString& folder, const QString& file)
{
    return d->itemDict.find(folder+file);
}

int CameraIconView::countItemsByFolder(QString folder)
{
    int count = 0;
    if (folder.endsWith("/")) folder.truncate(folder.length()-1);

    for (IconItem* item = firstItem(); item; item = item->nextItem())
    {
        CameraIconViewItem* iconItem = static_cast<CameraIconViewItem*>(item);
        QString itemFolder = iconItem->itemInfo()->folder;
        if (itemFolder.endsWith("/")) itemFolder.truncate(itemFolder.length()-1);

        if (folder == itemFolder)
            count++;
    }
    
    return count;
}

void CameraIconView::setThumbnail(const QString& folder, const QString& filename, const QImage& image)
{
    CameraIconViewItem* item = d->itemDict.find(folder+filename);
    if (!item)
        return;

    item->setThumbnail(image);
    item->repaint();
}

void CameraIconView::ensureItemVisible(CameraIconViewItem *item)
{
    IconView::ensureItemVisible(item);
}

void CameraIconView::ensureItemVisible(const GPItemInfo& itemInfo)
{
    ensureItemVisible(itemInfo.folder, itemInfo.name);
}

void CameraIconView::ensureItemVisible(const QString& folder, const QString& file)
{
    CameraIconViewItem* item = d->itemDict.find(folder+file);
    if (!item)
        return;

    ensureItemVisible(item);
}

void CameraIconView::slotDownloadNameChanged()
{
    bool hasSelection = false;
    for (IconItem* item = firstItem(); item; item = item->nextItem())
    {
        if (item->isSelected())
        {
            hasSelection = true;
            break;
        }
    }

    // connected to slotUpdateDownloadNames, and used externally
    emit signalNewSelection(hasSelection);
}

void CameraIconView::slotUpdateDownloadNames(bool hasSelection)
{
    bool useDefault = true;
    int  startIndex = 0;

    if (d->renamer)
    {
        useDefault = d->renamer->useDefault();
        startIndex = d->renamer->startIndex() -1;
    }
    
    bool convertLossLessJpeg = d->cameraUI->convertLosslessJpegFiles();
    QString losslessFormat   = d->cameraUI->losslessFormat();

    viewport()->setUpdatesEnabled(false);

    if (hasSelection)
    {
        // Camera items selection.
    
        for (IconItem* item = firstItem(); item; item = item->nextItem())
        {
            QString downloadName;
            CameraIconViewItem* viewItem = static_cast<CameraIconViewItem*>(item);

            if (item->isSelected())
            {
                if (!useDefault)
                    downloadName = getTemplatedName( viewItem->itemInfo(), startIndex );
                else
                    downloadName = getCasedName( d->renamer->changeCase(), viewItem->itemInfo() );

                startIndex++;
            }
    
            if (convertLossLessJpeg && !downloadName.isEmpty())
            {
                QFileInfo fi(downloadName);
                QString ext = fi.extension(false).upper();
                if (ext == QString("JPEG") || ext == QString("JPG") || ext == QString("JPE"))
                {
                    downloadName.truncate(downloadName.length() - ext.length());
                    downloadName.append(losslessFormat.lower());
                }
            }

            viewItem->setDownloadName( downloadName );
        }
    }
    else
    {
        // No camera item selection.
    
        for (IconItem* item = firstItem(); item; item = item->nextItem())
        {
            QString downloadName;
            CameraIconViewItem* viewItem = static_cast<CameraIconViewItem*>(item);
    
            if (!useDefault)
                downloadName = getTemplatedName( viewItem->itemInfo(), startIndex );
            else
                downloadName = getCasedName( d->renamer->changeCase(), viewItem->itemInfo() );

            if (convertLossLessJpeg)
            {
                QFileInfo fi(downloadName);
                QString ext = fi.extension(false).upper();
                if (ext == QString("JPEG") || ext == QString("JPG") || ext == QString("JPE"))
                {
                    downloadName.truncate(downloadName.length() - ext.length());
                    downloadName.append(losslessFormat.lower());
                }
            }
    
            viewItem->setDownloadName( downloadName );
            startIndex++;
        }
    }

    viewport()->setUpdatesEnabled(true);
    viewport()->update();
}

QString CameraIconView::defaultDownloadName(CameraIconViewItem *viewItem)
{
    RenameCustomizer::Case renamecase = RenameCustomizer::NONE;
    if (d->renamer)
        renamecase = d->renamer->changeCase();

    return getCasedName( renamecase, viewItem->itemInfo() );
}

QString CameraIconView::getTemplatedName(const GPItemInfo* itemInfo, int position)
{
    QString ext = itemInfo->name;
    int pos = ext.findRev('.');
    if (pos < 0)
        ext = "";
    else
        ext = ext.right( ext.length() - pos );

    QDateTime mtime;
    mtime.setTime_t(itemInfo->mtime);

    return d->renamer->newName(mtime, position+1, ext);
}

QString CameraIconView::getCasedName(const RenameCustomizer::Case ccase,
                                     const GPItemInfo* itemInfo)
{
    QString dname;

    switch (ccase)
    {
        case(RenameCustomizer::UPPER):
        {
            dname = itemInfo->name.upper();
            break;
        }
        case(RenameCustomizer::LOWER):
        {
            dname = itemInfo->name.lower();
            break;
        }
        default:
        {
            dname = itemInfo->name;
            break;
        }
    };

    return dname;
}

void CameraIconView::slotSelectionChanged()
{
    bool selected               = false;
    CameraIconViewItem* camItem = 0;

    for (IconItem* item = firstItem(); item; item = item->nextItem())
    {
        if (item->isSelected())
        {
            camItem  = static_cast<CameraIconViewItem*>(item);
            selected = true;
            break;
        }
    }

    emit signalNewSelection(selected);
    emit signalSelected(camItem, selected);

    viewport()->update();
}

void CameraIconView::slotContextMenu(IconItem * item, const QPoint&)
{
    if (!item)
        return;

    // don't popup context menu if the camera is busy
    if (d->cameraUI->isBusy())
        return;

    CameraIconViewItem* camItem = static_cast<CameraIconViewItem*>(item);
    
    DPopupMenu menu(this);
    menu.insertItem(SmallIcon("editimage"), i18n("&View"), 0);
    menu.insertSeparator(-1);
    menu.insertItem(SmallIcon("down"),i18n("Download"), 1);
    menu.insertItem(SmallIcon("down"),i18n("Download && Delete"), 4);
    menu.insertItem(SmallIcon("encrypted"), i18n("Toggle lock"), 3);
    menu.insertSeparator(-1);
    menu.insertItem(SmallIcon("editdelete"), i18n("Delete"), 2);

    int result = menu.exec(QCursor::pos());

    switch (result)
    {
        case(0):
        {
            emit signalFileView(camItem);
            break;
        }
        case(1):
        {
            emit signalDownload();
            break;
        }
        case(2):
        {
            emit signalDelete();
            break;
        }
        case(3):
        {
            emit signalToggleLock();
            break;
        }
        case(4):
        {
            emit signalDownloadAndDelete();
            break;
        }
        default:
            break;
    }
}

void CameraIconView::slotDoubleClicked(IconItem* item)
{
    if (!item)
        return;
    
    if (d->cameraUI->isBusy())
        return;

    emit signalFileView(static_cast<CameraIconViewItem*>(item));
}

void CameraIconView::slotSelectAll()
{
    selectAll();    
}

void CameraIconView::slotSelectNone()
{
    clearSelection();
}

void CameraIconView::slotSelectInvert()
{
    invertSelection();
}

void CameraIconView::slotSelectNew()
{
    blockSignals(true);
    clearSelection();

    for (IconItem* item = firstItem(); item;
         item = item->nextItem())
    {
        CameraIconViewItem* viewItem = static_cast<CameraIconViewItem*>(item);
        if (viewItem->itemInfo()->downloaded == GPItemInfo::NewPicture)
        {
            viewItem->setSelected(true, false);
        }
    }

    blockSignals(false);
    emit signalSelectionChanged();
}

void CameraIconView::startDrag()
{
    QStringList lst;

    for (IconItem* item = firstItem(); item; item = item->nextItem())
    {
        if (!item->isSelected())
            continue;

        CameraIconViewItem* iconItem = static_cast<CameraIconViewItem*>(item);
        QString itemPath = iconItem->itemInfo()->folder + iconItem->itemInfo()->name;
        lst.append(itemPath);
    }

    QDragObject * drag = new CameraItemListDrag(lst, d->cameraUI);
    if (drag)
    {
        QPixmap icon(DesktopIcon("image", 48));
        int w = icon.width();
        int h = icon.height();
    
        QPixmap pix(w+4,h+4);
        QString text(QString::number(lst.count()));
    
        QPainter p(&pix);
        p.fillRect(0, 0, w+4, h+4, QColor(Qt::white));
        p.setPen(QPen(Qt::black, 1));
        p.drawRect(0, 0, w+4, h+4);
        p.drawPixmap(2, 2, icon);
        QRect r = p.boundingRect(2,2,w,h,Qt::AlignLeft|Qt::AlignTop,text);
        r.setWidth(QMAX(r.width(),r.height()));
        r.setHeight(QMAX(r.width(),r.height()));
        p.fillRect(r, QColor(0,80,0));
        p.setPen(Qt::white);
        QFont f(font());
        f.setBold(true);
        p.setFont(f);
        p.drawText(r, Qt::AlignCenter, text);
        p.end();
        
        drag->setPixmap(pix);
        drag->drag();
    }
}

void CameraIconView::contentsDropEvent(QDropEvent *event)
{
    // don't popup context menu if the camera is busy
    if (d->cameraUI->isBusy())
        return;

    if ( (!QUriDrag::canDecode(event) && !CameraDragObject::canDecode(event) )
         || event->source() == this)
    {
        event->ignore();
        return;
    }

    KURL::List srcURLs;
    KURLDrag::decode(event, srcURLs);
    uploadItemPopupMenu(srcURLs);
}

void CameraIconView::slotRightButtonClicked(const QPoint&)
{
    // don't popup context menu if the camera is busy
    if (d->cameraUI->isBusy())
        return;

    QMimeSource *data = kapp->clipboard()->data(QClipboard::Clipboard);
    if(!data || !QUriDrag::canDecode(data))
        return;

    KURL::List srcURLs;
    KURLDrag::decode(data, srcURLs);
    uploadItemPopupMenu(srcURLs);
}

void CameraIconView::uploadItemPopupMenu(const KURL::List& srcURLs)
{
    KPopupMenu popMenu(this);
    popMenu.insertTitle(SmallIcon("digikam"), d->cameraUI->cameraTitle());
    popMenu.insertItem( SmallIcon("goto"), i18n("&Upload to camera"), 10 );
    popMenu.insertSeparator(-1);
    popMenu.insertItem( SmallIcon("cancel"), i18n("C&ancel") );

    popMenu.setMouseTracking(true);
    int id = popMenu.exec(QCursor::pos());
    switch(id) 
    {
        case 10: 
        {
            emit signalUpload(srcURLs);
            break;
        }
        default:
            break;
    }
}

QRect CameraIconView::itemRect() const
{
    return d->itemRect;
}

void CameraIconView::setThumbnailSize(const ThumbnailSize& thumbSize)
{
    if ( d->thumbSize != thumbSize)
    {
        d->thumbSize = thumbSize;
        updateItemRectsPixmap();
        triggerRearrangement();
    }
}

ThumbnailSize CameraIconView::thumbnailSize() const
{
    return d->thumbSize;
}

void CameraIconView::updateItemRectsPixmap()
{
    int thumbSize = d->thumbSize.size();

    QRect pixRect;
    QRect textRect;
    QRect extraRect;

    pixRect.setWidth(thumbSize);
    pixRect.setHeight(thumbSize);

    QFontMetrics fm(font());
    QRect r = QRect(fm.boundingRect(0, 0, thumbSize, 0xFFFFFFFF,
                                    Qt::AlignHCenter | Qt::AlignTop,
                                    "XXXXXXXXX"));
    textRect.setWidth(r.width());
    textRect.setHeight(r.height());

    QFont fn(font());
    if (fn.pointSize() > 0)
    {
        fn.setPointSize(QMAX(fn.pointSize()-2, 6));
    }

    fm = QFontMetrics(fn);
    r = QRect(fm.boundingRect(0, 0, thumbSize, 0xFFFFFFFF,
                              Qt::AlignHCenter | Qt::AlignTop,
                              "XXXXXXXXX"));
    extraRect.setWidth(r.width());
    extraRect.setHeight(r.height());

    r = QRect();
    r.setWidth(QMAX(QMAX(pixRect.width(), textRect.width()), extraRect.width()) + 4);
    r.setHeight(pixRect.height() + textRect.height() + extraRect.height() + 4);

    d->itemRect = r;

    d->itemRegPixmap = ThemeEngine::instance()->thumbRegPixmap(d->itemRect.width(),
                                                               d->itemRect.height());

    d->itemSelPixmap = ThemeEngine::instance()->thumbSelPixmap(d->itemRect.width(),
                                                               d->itemRect.height());
}

void CameraIconView::slotThemeChanged()
{
    updateItemRectsPixmap();
    viewport()->update();
}

int CameraIconView::itemsDownloaded()
{
    int downloaded = 0;

    for (IconItem* item = firstItem(); item; item = item->nextItem())
    {
        CameraIconViewItem* iconItem = static_cast<CameraIconViewItem*>(item);

        if (iconItem->itemInfo()->downloaded == GPItemInfo::DownloadedYes)
            downloaded++;
    }

    return downloaded;
}

void CameraIconView::itemsSelectionSizeInfo(unsigned long& fSizeKB, unsigned long& dSizeKB)
{
    long long fSize = 0;  // Files size
    long long dSize = 0;  // Estimated space requires to download and process files.
    for (IconItem* item = firstItem(); item; item = item->nextItem())
    {
        if (item->isSelected())
        {
            CameraIconViewItem* iconItem = static_cast<CameraIconViewItem*>(item);
            long long size = iconItem->itemInfo()->size;
            if (size < 0) // -1 if size is not provided by camera
                continue;
            fSize += size;

            if (iconItem->itemInfo()->mime == QString("image/jpeg"))
            {
                if (d->cameraUI->convertLosslessJpegFiles())
                {
                    // Estimated size is aroud 5 x original size when JPEG=>PNG.
                    dSize += size*5;
                }
                else if (d->cameraUI->autoRotateJpegFiles())
                {
                    // We need a double size to perform rotation.
                    dSize += size*2;
                }
                else
                {
                    // Real file size is added.
                    dSize += size;
                }
            }
            else
                dSize += size;

        }
    }
    
    fSizeKB = fSize / 1024;
    dSizeKB = dSize / 1024;
}

}  // namespace Digikam
