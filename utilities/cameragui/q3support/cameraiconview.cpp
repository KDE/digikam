/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-09-18
 * Description : camera icon view
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2006-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009 by Andi Clemens <andi dot clemens at googlemail dot com>
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

#include "cameraiconview.moc"

// Qt includes

#include <QClipboard>
#include <QCursor>
#include <QDir>
#include <QDropEvent>
#include <QFile>
#include <QFileInfo>
#include <QFont>
#include <QFontMetrics>
#include <QHash>
#include <QPainter>
#include <QPixmap>
#include <QTimer>

// KDE includes

#include <kaction.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmenu.h>
#include <kmimetype.h>
#include <kpixmapsequence.h>
#include <kdebug.h>

// Local includes

#include "advancedrenamemanager.h"
#include "advancedsettings.h"
#include "parsesettings.h"
#include "cameraiconitem.h"
#include "cameraiconviewtooltip.h"
#include "cameraui.h"
#include "ddragobjects.h"
#include "icongroupitem.h"
#include "renamecustomizer.h"
#include "thememanager.h"
#include "thumbnailsize.h"

namespace Digikam
{

class CameraIconView::CameraIconViewPriv
{

public:

    CameraIconViewPriv() :
        thumbSize(ThumbnailSize::Large),
        pixmapNewPicture(SmallIcon("get-hot-new-stuff")),
        pixmapDownloadUnknown(SmallIcon("svn_status")),
        progressPix(KPixmapSequence("process-working", KIconLoader::SizeSmallMedium)),
        pixmapLocked(SmallIcon("object-locked")),
        pixmapDownloaded(SmallIcon("dialog-ok")),
        pixmapDownloadFailed(SmallIcon("dialog-cancel")),
        renamer(0),
        groupItem(0),
        cameraUI(0),
        thumbCtrl(0),
        toolTip(0)
    {
    }

    int                              thumbSize;

    QHash<QString, CameraIconItem*>  itemDict;

    QRect                            itemRect;

    QPixmap                          itemRegPixmap;
    QPixmap                          itemSelPixmap;
    QPixmap                          pixmapNewPicture;
    QPixmap                          pixmapDownloadUnknown;
    KPixmapSequence                  progressPix;
    QPixmap                          pixmapLocked;
    QPixmap                          pixmapDownloaded;
    QPixmap                          pixmapDownloadFailed;

    RenameCustomizer*                renamer;

    IconGroupItem*                   groupItem;

    CameraUI*                        cameraUI;
    CameraThumbsCtrl*                thumbCtrl;
    CameraIconViewToolTip*           toolTip;
};

CameraIconView::CameraIconView(CameraUI* ui, QWidget* parent)
    : IconView(parent), d(new CameraIconViewPriv)
{
    d->cameraUI  = ui;
    d->groupItem = new IconGroupItem(this);
    d->toolTip   = new CameraIconViewToolTip(this);

    setHScrollBarMode(Q3ScrollView::AlwaysOff);
    setMinimumSize(400, 300);

    setAcceptDrops(true);
    viewport()->setAcceptDrops(true);

    // ----------------------------------------------------------------

    connect(this, SIGNAL(signalSelectionChanged()),
            this, SLOT(slotSelectionChanged()));

    connect(this, SIGNAL(signalNewSelection(bool)),
            this, SLOT(slotUpdateDownloadNames(bool)));

    connect(this, SIGNAL(signalRightButtonClicked(IconItem*,QPoint)),
            this, SLOT(slotContextMenu(IconItem*,QPoint)));

    connect(this, SIGNAL(signalRightButtonClicked(QPoint)),
            this, SLOT(slotRightButtonClicked(QPoint)));

    connect(this, SIGNAL(signalDoubleClicked(IconItem*)),
            this, SLOT(slotDoubleClicked(IconItem*)));

    connect(ThemeManager::instance(), SIGNAL(signalThemeChanged()),
            this, SLOT(slotThemeChanged()));

    connect(this, SIGNAL(signalShowToolTip(IconItem*)),
            this, SLOT(slotShowToolTip(IconItem*)));

    // ----------------------------------------------------------------

    updateItemRectsPixmap();
    slotThemeChanged();
}

CameraIconView::~CameraIconView()
{
    clear();
    delete d->toolTip;
    delete d;
}

KPixmapSequence CameraIconView::progressPixmap() const
{
    return d->progressPix;
}

QPixmap CameraIconView::itemBaseRegPixmap() const
{
    return d->itemRegPixmap;
}

QPixmap CameraIconView::itemBaseSelPixmap() const
{
    return d->itemSelPixmap;
}

QPixmap CameraIconView::newPicturePixmap() const
{
    return d->pixmapNewPicture;
}

QPixmap CameraIconView::downloadUnknownPixmap() const
{
    return d->pixmapDownloadUnknown;
}

QPixmap CameraIconView::lockedPixmap() const
{
    return d->pixmapLocked;
}

QPixmap CameraIconView::downloadedPixmap() const
{
    return d->pixmapDownloaded;
}

QPixmap CameraIconView::downloadFailedPixmap() const
{
    return d->pixmapDownloadFailed;
}

void CameraIconView::setRenameCustomizer(RenameCustomizer* renamer)
{
    d->renamer = renamer;

    connect(d->renamer, SIGNAL(signalChanged()),
            this, SLOT(slotDownloadNameChanged()));
}

void CameraIconView::setThumbControler(CameraThumbsCtrl* controler)
{
    d->thumbCtrl = controler;

    connect(d->thumbCtrl, SIGNAL(signalThumbInfoReady(CamItemInfo)),
            this, SLOT(slotThumbInfoReady(CamItemInfo)));
}

CachedItem CameraIconView::getThumbInfo(const CamItemInfo& itemInfo) const
{
    // If thumb/Info are not yet in cache, there will arrive later with slotThumbInfoReady()
    CachedItem item;
    d->thumbCtrl->getThumbInfo(itemInfo, item);
    return item;
}

void CameraIconView::slotThumbInfoReady(const CamItemInfo& info)
{
    CameraIconItem* item = findItem(info.folder, info.name);

    if (item)
    {
        // Updating item, thumb controller will be called to refresh icon item. See repaint() method for details.
        item->update();
    }

    viewport()->update();
}

void CameraIconView::addItem(const CamItemInfo& info)
{
    QString downloadName;

    //    if (d->renamer)
    //    {
    //        if (!d->renamer->useDefault())
    //        {
    //            downloadName = getTemplatedName(&info);
    //        }
    //        else
    //        {
    //            downloadName = getCasedName( d->renamer->changeCase(), &info);
    //        }
    //    }

    CamItemInfo newinfo  = info;
    newinfo.downloadName = downloadName;
    CameraIconItem* item = new CameraIconItem(d->groupItem, newinfo);
    QString sep;

    if (!newinfo.folder.isEmpty() && !newinfo.folder.endsWith('/'))
    {
        sep = '/';
    }

    d->itemDict.insert(newinfo.folder + sep + newinfo.name, item);
}

void CameraIconView::removeItem(const CamItemInfo& info)
{
    CameraIconItem* item = findItem(info.folder, info.name);

    if (!item)
    {
        return;
    }

    QString sep;

    if (!info.folder.isEmpty() && !info.folder.endsWith('/'))
    {
        sep = '/';
    }

    d->itemDict.remove(info.folder + sep + info.name);

    setDelayedRearrangement(true);
    delete item;
    setDelayedRearrangement(false);
}

CamItemInfo CameraIconView::findItemInfo(const QString& folder, const QString& filename) const
{
    CamItemInfo     info;
    CameraIconItem* item = findItem(folder, filename);

    if (item)
    {
        info = item->itemInfo();
    }

    return info;
}

CameraIconItem* CameraIconView::findItem(const QString& folder, const QString& filename) const
{
    QString sep;

    if (!folder.isEmpty() && !folder.endsWith('/'))
    {
        sep = '/';
    }

    return d->itemDict.value(folder + sep + filename);
}

QMap<QString, int> CameraIconView::countItemsByFolders() const
{
    QString                      path;
    QMap<QString, int>           map;
    QMap<QString, int>::iterator it;
    CameraIconItem*              iconItem = 0;

    for (IconItem* item = firstItem(); item; item = item->nextItem())
    {
        iconItem = static_cast<CameraIconItem*>(item);
        path     = iconItem->itemInfo().folder;

        if (!path.isEmpty() && path.endsWith('/'))
        {
            path.truncate(path.length() - 1);
        }

        it = map.find(path);

        if (it == map.end())
        {
            map.insert(path, 1);
        }
        else
        {
            it.value() ++;
        }
    }

    return map;
}

void CameraIconView::setDownloaded(const CamItemInfo& itemInfo, int status)
{
    CameraIconItem* iconItem = findItem(itemInfo.folder, itemInfo.name);

    if (iconItem)
    {
        iconItem->setDownloaded(status);
    }
}

bool CameraIconView::isDownloaded(const CamItemInfo& itemInfo)
{
    CameraIconItem* iconItem = findItem(itemInfo.folder, itemInfo.name);

    if (iconItem)
    {
        return iconItem->isDownloaded();
    }

    return false;
}

void CameraIconView::toggleLock(const CamItemInfo& itemInfo)
{
    CameraIconItem* iconItem = findItem(itemInfo.folder, itemInfo.name);

    if (iconItem)
    {
        iconItem->toggleLock();
    }
}

void CameraIconView::ensureItemVisible(CameraIconItem* item)
{
    IconView::ensureItemVisible(item);
}

void CameraIconView::ensureItemVisible(const CamItemInfo& itemInfo)
{
    ensureItemVisible(itemInfo.folder, itemInfo.name);
}

void CameraIconView::ensureItemVisible(const QString& folder, const QString& filename)
{
    CameraIconItem* item = findItem(folder, filename);

    if (!item)
    {
        return;
    }

    ensureItemVisible(item);
}

bool CameraIconView::isSelected(const CamItemInfo& itemInfo)
{
    CameraIconItem* iconItem = findItem(itemInfo.folder, itemInfo.name);

    if (iconItem)
    {
        return iconItem->isSelected();
    }

    return false;
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
    if (!count())
    {
        return;
    }

    bool useDefault = true;
    int  startIndex = 0;

    if (d->renamer)
    {
        useDefault = d->renamer->useDefault();
        startIndex = d->renamer->startIndex();
    }

    DownloadSettings settings = d->cameraUI->downloadSettings();

    viewport()->setUpdatesEnabled(false);

    // NOTE: see B.K.O #182352: ordering of item count must be adapted sort of icon view,
    // since items are ordered from the most recent to the older one.
    bool revOrder = !d->cameraUI->chronologicOrder();
    // Camera items selection.

    // reset the start index
    d->renamer->reset();
    d->renamer->setStartIndex(startIndex);

    QList<ParseSettings> cameraFiles;

    for (IconItem* item = (revOrder ? lastItem() : firstItem()); item;
         (revOrder ? item = item->prevItem() : item = item->nextItem()))
    {
        CameraIconItem* viewItem = static_cast<CameraIconItem*>(item);

        if ((hasSelection && item->isSelected()) || !hasSelection)
        {
            QFileInfo fi;
            fi.setFile(QDir(viewItem->itemInfo().folder), viewItem->itemInfo().name);
            ParseSettings ps;
            ps.fileUrl      = KUrl(fi.absoluteFilePath());
            ps.creationTime = viewItem->itemInfo().mtime;
            cameraFiles << ps;
        }
    }

    d->renamer->renameManager()->addFiles(cameraFiles);
    d->renamer->renameManager()->parseFiles();

    for (IconItem* item = (revOrder ? lastItem() : firstItem()); item;
         (revOrder ? item = item->prevItem() : item = item->nextItem()))
    {
        QString downloadName;
        CameraIconItem* viewItem = static_cast<CameraIconItem*>(item);

        if ((hasSelection && item->isSelected()) || !hasSelection)
        {
            if (!useDefault)
            {
                QFileInfo fi;
                fi.setFile(QDir(viewItem->itemInfo().folder), viewItem->itemInfo().name);
                downloadName = d->renamer->renameManager()->newName(fi.absoluteFilePath());
            }
            else
            {
                downloadName = getCasedName(d->renamer->changeCase(), viewItem->itemInfo());
            }
        }

        if (settings.convertJpeg && !downloadName.isEmpty())
        {
            QFileInfo fi(downloadName);
            QString ext = fi.suffix().toUpper();

            if (ext == QString("JPEG") || ext == QString("JPG") || ext == QString("JPE"))
            {
                downloadName.truncate(downloadName.length() - ext.length());
                downloadName.append(settings.losslessFormat.toLower());
            }
        }

        viewItem->setDownloadName(downloadName);
    }

    viewport()->setUpdatesEnabled(true);
    viewport()->update();
}

QString CameraIconView::defaultDownloadName(const CamItemInfo& info) const
{
    RenameCustomizer::Case renamecase = RenameCustomizer::NONE;

    if (d->renamer)
    {
        renamecase = d->renamer->changeCase();
    }

    return getCasedName(renamecase, info);
}

QString CameraIconView::getTemplatedName(const CamItemInfo& itemInfo) const
{
    QFileInfo fi;
    fi.setFile(QDir(itemInfo.folder), itemInfo.name);

    return d->renamer->newName(fi.absoluteFilePath(), itemInfo.mtime);
}

QString CameraIconView::getCasedName(const RenameCustomizer::Case ccase, const CamItemInfo& itemInfo) const
{
    QString dname;

    switch (ccase)
    {
        case (RenameCustomizer::UPPER):
        {
            dname = itemInfo.name.toUpper();
            break;
        }

        case (RenameCustomizer::LOWER):
        {
            dname = itemInfo.name.toLower();
            break;
        }

        default:
        {
            dname = itemInfo.name;
            break;
        }
    };

    return dname;
}

void CameraIconView::slotSelectionChanged()
{
    bool selected           = false;
    CameraIconItem* camItem = 0;

    for (IconItem* item = firstItem(); item; item = item->nextItem())
    {
        if (item->isSelected())
        {
            camItem  = static_cast<CameraIconItem*>(item);
            selected = true;
            break;
        }
    }

    emit signalNewSelection(selected);
    emit signalSelected(camItem ? camItem->itemInfo() : CamItemInfo(), selected);

    viewport()->update();
}

CamItemInfo CameraIconView::firstItemSelected() const
{
    CameraIconItem* camItem = 0;

    for (IconItem* item = firstItem(); item; item = item->nextItem())
    {
        if (item->isSelected())
        {
            camItem = static_cast<CameraIconItem*>(item);
            break;
        }
    }

    return(camItem ? camItem->itemInfo() : CamItemInfo());
}


void CameraIconView::slotContextMenu(IconItem* item, const QPoint&)
{
    if (!item)
    {
        return;
    }

    // don't popup context menu if the camera is busy
    if (d->cameraUI->isBusy())
    {
        return;
    }

    CameraIconItem* camItem = static_cast<CameraIconItem*>(item);

    KMenu menu(this);
    QAction* viewAction      = menu.addAction(SmallIcon("editimage"),
                                              i18nc("View the selected image", "&View"));
    menu.addSeparator();
    QAction* downAction      = menu.addAction(SmallIcon("computer"), i18n("Download"));
    QAction* downDelAction   = menu.addAction(SmallIcon("computer"), i18n("Download && Delete"));
    QAction* encryptedAction = menu.addAction(SmallIcon("object-locked"), i18n("Toggle Lock"));
    menu.addSeparator();
    QAction* deleteAction    = menu.addAction(SmallIcon("edit-delete"), i18n("Delete"));

    downDelAction->setEnabled(d->cameraUI->cameraDeleteSupport());
    deleteAction->setEnabled(d->cameraUI->cameraDeleteSupport());

    QAction* choice = menu.exec(QCursor::pos());

    if (choice)
    {
        if (choice == viewAction)
        {
            emit signalFileView(camItem->itemInfo());
        }
        else if (choice == downAction)
        {
            emit signalDownload();
        }
        else if (choice == deleteAction)
        {
            emit signalDelete();
        }
        else if (choice == encryptedAction)
        {
            emit signalToggleLock();
        }
        else if (choice == downDelAction)
        {
            emit signalDownloadAndDelete();
        }
    }
}

void CameraIconView::slotDoubleClicked(IconItem* item)
{
    if (!item)
    {
        return;
    }

    if (d->cameraUI->isBusy())
    {
        return;
    }

    emit signalFileView(static_cast<CameraIconItem*>(item)->itemInfo());
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
        CameraIconItem* viewItem = static_cast<CameraIconItem*>(item);

        if (viewItem->itemInfo().downloaded == CamItemInfo::NewPicture)
        {
            viewItem->setSelected(true, false);
        }
    }

    blockSignals(false);
    emit signalSelectionChanged();
}

void CameraIconView::slotSelectLocked()
{
    blockSignals(true);
    clearSelection();

    for (IconItem* item = firstItem(); item;
         item = item->nextItem())
    {
        CameraIconItem* viewItem = static_cast<CameraIconItem*>(item);

        if (viewItem->itemInfo().writePermissions == 0)
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
        {
            continue;
        }

        CameraIconItem* iconItem = static_cast<CameraIconItem*>(item);
        QString itemPath = iconItem->itemInfo().folder + iconItem->itemInfo().name;
        lst.append(itemPath);
    }

    QDrag* drag = new QDrag(d->cameraUI);
    drag->setMimeData(new DCameraItemListDrag(lst));

    QPixmap icon(DesktopIcon("image-jp2", 48));
    int w = icon.width();
    int h = icon.height();

    QPixmap pix(w + 4, h + 4);
    QString text(QString::number(lst.count()));

    QPainter p(&pix);
    p.fillRect(0, 0, pix.width() - 1, pix.height() - 1, QColor(Qt::white));
    p.setPen(QPen(Qt::black, 1));
    p.drawRect(0, 0, pix.width() - 1, pix.height() - 1);
    p.drawPixmap(2, 2, icon);
    QRect r = p.boundingRect(2, 2, w, h, Qt::AlignLeft | Qt::AlignTop, text);
    r.setWidth(qMax(r.width(), r.height()));
    r.setHeight(qMax(r.width(), r.height()));
    p.fillRect(r, QColor(0, 80, 0));
    p.setPen(Qt::white);
    QFont f(font());
    f.setBold(true);
    p.setFont(f);
    p.drawText(r, Qt::AlignCenter, text);
    p.end();

    drag->setPixmap(pix);
    drag->exec();
}

void CameraIconView::contentsDragEnterEvent(QDragEnterEvent* e)
{
    // Don't popup context menu if the camera is busy or if camera do not support upload.
    if (d->cameraUI->isBusy() || !d->cameraUI->cameraUploadSupport())
    {
        return;
    }

    if ((!KUrl::List::canDecode(e->mimeData()) && !DCameraDragObject::canDecode(e->mimeData()))
        || e->source() == this)
    {
        e->ignore();
        return;
    }

    e->acceptProposedAction();
}

void CameraIconView::contentsDropEvent(QDropEvent* e)
{
    // Don't popup context menu if the camera is busy or if camera do not support upload.
    if (d->cameraUI->isBusy() || !d->cameraUI->cameraUploadSupport())
    {
        return;
    }

    if ((!KUrl::List::canDecode(e->mimeData()) && !DCameraDragObject::canDecode(e->mimeData()))
        || e->source() == this)
    {
        e->ignore();
        return;
    }

    KUrl::List srcURLs = KUrl::List::fromMimeData(e->mimeData());
    uploadItemPopupMenu(srcURLs);
}

void CameraIconView::slotRightButtonClicked(const QPoint&)
{
    // Don't popup context menu if the camera is busy or if camera do not support upload.
    if (d->cameraUI->isBusy() || d->cameraUI->cameraUploadSupport())
    {
        return;
    }

    const QMimeData* data = kapp->clipboard()->mimeData(QClipboard::Clipboard);

    if (!data || !KUrl::List::canDecode(data))
    {
        return;
    }

    KUrl::List srcURLs = KUrl::List::fromMimeData(data);
    uploadItemPopupMenu(srcURLs);
}

void CameraIconView::uploadItemPopupMenu(const KUrl::List& srcURLs)
{
    KMenu popMenu(this);
    popMenu.addTitle(SmallIcon("digikam"), d->cameraUI->cameraTitle());
    QAction* uploadAction = popMenu.addAction(SmallIcon("media-flash-smart-media"), i18n("&Upload to camera"));
    popMenu.addSeparator();
    popMenu.addAction(SmallIcon("dialog-cancel"), i18n("C&ancel"));

    popMenu.setMouseTracking(true);
    QAction* choice = popMenu.exec(QCursor::pos());

    if (choice == uploadAction)
    {
        emit signalUpload(srcURLs);
    }
}

QRect CameraIconView::itemRect() const
{
    return d->itemRect;
}

void CameraIconView::setThumbnailSize(int size)
{
    if (d->thumbSize != size)
    {
        if (size > ThumbnailSize::Huge)
        {
            d->thumbSize = ThumbnailSize::Huge;
        }
        else if (size < ThumbnailSize::Small)
        {
            d->thumbSize = ThumbnailSize::Small;
        }
        else
        {
            d->thumbSize = size;
        }

        updateItemRectsPixmap();
        triggerRearrangement();
        emit signalThumbSizeChanged(d->thumbSize);
    }
}

int CameraIconView::thumbnailSize() const
{
    return d->thumbSize;
}

void CameraIconView::updateItemRectsPixmap()
{
    QRect pixRect;
    QRect textRect;
    QRect extraRect;

    pixRect.setWidth(d->thumbSize);
    pixRect.setHeight(d->thumbSize);

    QFontMetrics fm(font());
    QRect r = QRect(fm.boundingRect(0, 0, d->thumbSize, 0xFFFFFFFF,
                                    Qt::AlignHCenter | Qt::AlignTop,
                                    "XXXXXXXXX"));
    textRect.setWidth(r.width());
    textRect.setHeight(r.height());

    QFont fn(font());

    if (fn.pointSize() > 0)
    {
        fn.setPointSize(qMax(fn.pointSize() - 2, 6));
    }

    fm = QFontMetrics(fn);
    r  = QRect(fm.boundingRect(0, 0, d->thumbSize, 0xFFFFFFFF,
                               Qt::AlignHCenter | Qt::AlignTop,
                               "XXXXXXXXX"));
    extraRect.setWidth(r.width());
    extraRect.setHeight(r.height());

    r = QRect();
    r.setWidth(qMax(qMax(pixRect.width(), textRect.width()), extraRect.width()) + 4);
    r.setHeight(pixRect.height() + textRect.height() + extraRect.height() + 4);

    d->itemRect = r;

    d->itemRegPixmap = QPixmap(d->itemRect.width(), d->itemRect.height());
    d->itemRegPixmap.fill(kapp->palette().color(QPalette::Base));
    QPainter p1(&d->itemRegPixmap);
    p1.setPen(kapp->palette().color(QPalette::Midlight));
    p1.drawRect(0, 0, d->itemRect.width() - 1, d->itemRect.height() - 1);

    d->itemSelPixmap = QPixmap(d->itemRect.width(), d->itemRect.height());
    d->itemSelPixmap.fill(kapp->palette().color(QPalette::Highlight));
    QPainter p2(&d->itemSelPixmap);
    p2.setPen(kapp->palette().color(QPalette::Midlight));
    p2.drawRect(0, 0, d->itemRect.width() - 1, d->itemRect.height() - 1);

    clearThumbnailBorderCache();
}

void CameraIconView::slotThemeChanged()
{
    updateItemRectsPixmap();
    viewport()->update();
}

bool CameraIconView::acceptToolTip(IconItem* item, const QPoint& mousePos)
{
    CameraIconItem* iconItem = dynamic_cast<CameraIconItem*>(item);

    if (iconItem && iconItem->clickToOpenRect().contains(mousePos))
    {
        return true;
    }
    else
    {
        return false;
    }
}

void CameraIconView::slotShowToolTip(IconItem* item)
{
    d->toolTip->setIconItem(dynamic_cast<CameraIconItem*>(item));
}

int CameraIconView::itemsDownloaded() const
{
    int downloaded = 0;

    for (IconItem* item = firstItem(); item; item = item->nextItem())
    {
        CameraIconItem* iconItem = static_cast<CameraIconItem*>(item);

        if (iconItem->itemInfo().downloaded == CamItemInfo::DownloadedYes)
        {
            ++downloaded;
        }
    }

    return downloaded;
}

void CameraIconView::itemsSelectionSizeInfo(unsigned long& fSizeKB, unsigned long& dSizeKB)
{
    qint64 fSize = 0;  // Files size
    qint64 dSize = 0;  // Estimated space requires to download and process files.
    DownloadSettings settings = d->cameraUI->downloadSettings();

    for (IconItem* item = firstItem(); item; item = item->nextItem())
    {
        if (item->isSelected())
        {
            CameraIconItem* iconItem = static_cast<CameraIconItem*>(item);
            qint64 size = iconItem->itemInfo().size;

            if (size < 0) // -1 if size is not provided by camera
            {
                continue;
            }

            fSize += size;

            if (iconItem->itemInfo().mime == QString("image/jpeg"))
            {
                if (settings.convertJpeg)
                {
                    // Estimated size is around 5 x original size when JPEG=>PNG.
                    dSize += size * 5;
                }
                else if (settings.autoRotate)
                {
                    // We need a double size to perform rotation.
                    dSize += size * 2;
                }
                else
                {
                    // Real file size is added.
                    dSize += size;
                }
            }
            else
            {
                dSize += size;
            }

        }
    }

    fSizeKB = fSize / 1024;
    dSizeKB = dSize / 1024;
}

CamItemInfoList CameraIconView::selectedItems() const
{
    CamItemInfoList list;

    for (IconItem* item = firstItem(); item; item = item->nextItem())
    {
        CameraIconItem* iconItem = static_cast<CameraIconItem*>(item);

        if (iconItem->isSelected())
        {
            list.append(iconItem->itemInfo());
        }
    }

    return list;
}

CamItemInfoList CameraIconView::allItems(bool lastPhotoFirst) const
{
    CamItemInfoList list;

    for (IconItem* item = (lastPhotoFirst ? lastItem() : firstItem()); item;
         item = (lastPhotoFirst ? item->prevItem() : item->nextItem()))
    {
        CameraIconItem* iconItem = static_cast<CameraIconItem*>(item);
        list.append(iconItem->itemInfo());
    }

    return list;
}

void CameraIconView::slotFirstItem()
{
    CameraIconItem* currItem = dynamic_cast<CameraIconItem*>(firstItem());
    clearSelection();
    updateContents();

    if (currItem)
    {
        setCurrentItem(currItem);
    }
}

void CameraIconView::slotPrevItem()
{
    CameraIconItem* currItem = dynamic_cast<CameraIconItem*>(currentItem());
    clearSelection();
    updateContents();

    if (currItem)
    {
        setCurrentItem(currItem->prevItem());
    }
}

void CameraIconView::slotNextItem()
{
    CameraIconItem* currItem = dynamic_cast<CameraIconItem*>(currentItem());
    clearSelection();
    updateContents();

    if (currItem)
    {
        setCurrentItem(currItem->nextItem());
    }
}

void CameraIconView::slotLastItem()
{
    CameraIconItem* currItem = dynamic_cast<CameraIconItem*>(lastItem());
    clearSelection();
    updateContents();

    if (currItem)
    {
        setCurrentItem(currItem);
    }
}

}  // namespace Digikam
