/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Caulier Gilles <caulier dot gilles at kdemail dot net>
 * Date   : 2004-09-18
 * Description : camera icon view
 * 
 * Copyright 2004-2005 by Renchi Raju
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

// C++ includes.

#include <ctime>

// Qt includes.

#include <qfile.h>
#include <qfileinfo.h>
#include <qtimer.h>
#include <qpixmap.h>
#include <qcursor.h>
#include <qfontmetrics.h>
#include <qfont.h>
#include <qdragobject.h>
#include <qclipboard.h>

// KDE includes.

#include <kpopupmenu.h>
#include <kurldrag.h>
#include <kmimetype.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <kaction.h>
#include <kapplication.h>

// Local includes.

#include "themeengine.h"
#include "thumbnailsize.h"
#include "gpiteminfo.h"
#include "renamecustomizer.h"
#include "icongroupitem.h"
#include "cameraui.h"
#include "cameradragobject.h"
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
        renamer   = 0;
        groupItem = 0;
        cameraUI  = 0;
        thumbSize = ThumbnailSize::Large;
    }

    QDict<CameraIconViewItem>  itemDict;

    QRect                      itemRect;

    QPixmap                    itemRegPixmap;
    QPixmap                    itemSelPixmap;

    RenameCustomizer          *renamer;

    IconGroupItem             *groupItem;

    ThumbnailSize              thumbSize;

    CameraUI                  *cameraUI;
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

void CameraIconView::setRenameCustomizer(RenameCustomizer* renamer)
{
    d->renamer = renamer;
    
    connect(d->renamer, SIGNAL(signalChanged()),
            this, SLOT(slotDownloadNameChanged()));
}

void CameraIconView::addItem(const GPItemInfo& info)
{
    KMimeType::Ptr mime;

    // Just to have a generic image thumb from desktop with KDE < 3.5.0
    mime = KMimeType::mimeType(info.mime == QString("image/x-raw") ? QString("image/tiff") : info.mime);

    QImage thumb(mime->pixmap(KIcon::Desktop, ThumbnailSize::Huge, KIcon::DefaultState).convertToImage());
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

    setDelayedUpdate(true);
    delete item;
    setDelayedUpdate(false);
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
                QString ext = fi.extension().upper();
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
                QString ext = fi.extension().upper();
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

    rearrangeItems();
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
    
    KPopupMenu menu(this);
    menu.insertTitle(SmallIcon("digikam"), d->cameraUI->cameraTitle());
    menu.insertItem(SmallIcon("editimage"), i18n("&View"), 0);
    menu.insertSeparator(-1);
    menu.insertItem(SmallIcon("down"),i18n("Download"), 1);
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
    popMenu.insertItem( SmallIcon("goto"), i18n("&Upload into camera"), 10 );
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
        rearrangeItems(true);
        if (IconView::currentItem())
            IconView::ensureItemVisible(IconView::currentItem());
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
    QPalette plt(palette());
    QColorGroup cg(plt.active());
    cg.setColor(QColorGroup::Base, ThemeEngine::instance()->baseColor());
    cg.setColor(QColorGroup::Text, ThemeEngine::instance()->textRegColor());
    cg.setColor(QColorGroup::HighlightedText, ThemeEngine::instance()->textSelColor());
    plt.setActive(cg);
    plt.setInactive(cg);
    setPalette(plt);

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

}  // namespace Digikam

