/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-18-12
 * Description : A list view to display digiKam Tags.
 *
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

#include <QList>
#include <QDropEvent>
#include <QMouseEvent>

// KDE includes.

#include <kdebug.h>
#include <kmenu.h>
#include <klocale.h>
#include <kurl.h>
#include <kcursor.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kconfig.h>
#include <kglobalsettings.h>
#include <kdialog.h>

// Local includes.

#include "albumiconitem.h"
#include "albummanager.h"
#include "albumdb.h"
#include "album.h"
#include "albumsettings.h"
#include "albumlister.h"
#include "databasetransaction.h"
#include "imageinfo.h"
#include "ddragobjects.h"
#include "imageattributeswatch.h"
#include "albumthumbnailloader.h"
#include "scancontroller.h"
#include "statusprogressbar.h"
#include "talbumlistview.h"
#include "talbumlistview.moc"

namespace Digikam
{

TAlbumCheckListItem::TAlbumCheckListItem(Q3ListView* parent, TAlbum* album)
                   : FolderCheckListItem(parent, album->title(), Q3CheckListItem::RadioButtonController)
{
    setDragEnabled(true);
    m_album = album;
    m_count = 0;

    if (m_album)
        m_album->setExtraData(listView(), this);
}

TAlbumCheckListItem::TAlbumCheckListItem(Q3CheckListItem* parent, TAlbum* album)
                   : FolderCheckListItem(parent, album->title(), Q3CheckListItem::CheckBox)
{
    setDragEnabled(true);
    m_album = album;
    m_count = 0;

    if (m_album)
        m_album->setExtraData(listView(), this);
}

void TAlbumCheckListItem::refresh()
{
    if (!m_album) return;

    if (AlbumSettings::instance()->getShowFolderTreeViewItemsCount() &&
        dynamic_cast<TAlbumCheckListItem*>(parent()))
    {
        if (isOpen())
            setText(0, QString("%1 (%2)").arg(m_album->title()).arg(m_count));
        else
        {
            int countRecursive = m_count;
            AlbumIterator it(m_album);
            while ( it.current() )
            {
                TAlbumCheckListItem *item = (TAlbumCheckListItem*)it.current()->extraData(listView());
                if (item)
                    countRecursive += item->count();
                ++it;
            }
            setText(0, QString("%1 (%2)").arg(m_album->title()).arg(countRecursive));
        }
    }
    else
    {
        setText(0, m_album->title());
    }
}

void TAlbumCheckListItem::stateChange(bool val)
{
    Q3CheckListItem::stateChange(val);
    ((TAlbumListView*)listView())->stateChanged(this);
}

void TAlbumCheckListItem::setOpen(bool o)
{
    Q3ListViewItem::setOpen(o);
    refresh();
}

TAlbum* TAlbumCheckListItem::album() const
{
    return m_album;
}

int TAlbumCheckListItem::id() const
{
    return m_album ? m_album->id() : 0;
}

void TAlbumCheckListItem::setCount(int count)
{
    m_count = count;
    refresh();
}

int TAlbumCheckListItem::count()
{
    return m_count;
}

void TAlbumCheckListItem::setStatus(MetadataHub::TagStatus status)
{
    if (status == MetadataHub::MetadataDisjoint)
    {
        if (type() != Q3CheckListItem::RadioButtonController) setTristate(true);
        setState(Q3CheckListItem::NoChange);
    }
    else
    {
        if (type() != Q3CheckListItem::RadioButtonController) setTristate(false);
        setOn(status.hasTag);
    }
}

// ------------------------------------------------------------------------

TAlbumListView::TAlbumListView(QWidget* parent)
              : FolderView(parent, "TAlbumListView")
{
    addColumn(i18n("Tags"));
    header()->hide();
    setResizeMode(Q3ListView::LastColumn);
    setRootIsDecorated(true);

    setAcceptDrops(true);
    viewport()->setAcceptDrops(true);

    connect(AlbumManager::instance(), SIGNAL(signalTAlbumsDirty(const QMap<int, int>&)),
            this, SLOT(slotRefresh(const QMap<int, int>&)));
}

TAlbumListView::~TAlbumListView()
{
}

void TAlbumListView::stateChanged(TAlbumCheckListItem *item)
{
    emit signalItemStateChanged(item);
}

QDrag* TAlbumListView::makeDragObject()
{
    TAlbumCheckListItem *item = dynamic_cast<TAlbumCheckListItem*>(dragItem());
    if(!item)
        return 0;

    if(!item->parent())
        return 0;

    QDrag *drag = new QDrag(this);
    drag->setMimeData(new DTagDrag(item->id()));
    drag->setPixmap(*item->pixmap(0));

    return drag;
}

bool TAlbumListView::acceptDrop(const QDropEvent *e) const
{
    QPoint vp = contentsToViewport(e->pos());
    TAlbumCheckListItem *itemDrop = dynamic_cast<TAlbumCheckListItem*>(itemAt(vp));
    TAlbumCheckListItem *itemDrag = dynamic_cast<TAlbumCheckListItem*>(dragItem());

    if(DTagDrag::canDecode(e->mimeData()) || DTagListDrag::canDecode(e->mimeData()))
    {
        // Allow dragging at the root, to move the tag to the root
        if(!itemDrop)
            return true;

        // Dragging an item on itself makes no sense
        if(itemDrag == itemDrop)
            return false;

        // Dragging a parent on its child makes no sense
        if(itemDrag && itemDrag->album()->isAncestorOf(itemDrop->album()))
            return false;

        return true;
    }

    if (DItemDrag::canDecode(e->mimeData()) && itemDrop && itemDrop->album()->parent())
    {
        // Only other possibility is image items being dropped
        // And allow this only if there is a Tag to be dropped
        // on and also the Tag is not root.
        return true;
    }

    return false;
}

void TAlbumListView::contentsDropEvent(QDropEvent *e)
{
    Q3ListView::contentsDropEvent(e);

    if(!acceptDrop(e))
        return;

    QPoint vp = contentsToViewport(e->pos());
    TAlbumCheckListItem *itemDrop = dynamic_cast<TAlbumCheckListItem*>(itemAt(vp));

    if(DTagDrag::canDecode(e->mimeData()))
    {
        int tagID;
        if (!DTagDrag::decode(e->mimeData(), tagID))
            return;

        AlbumManager* man = AlbumManager::instance();
        TAlbum* talbum    = man->findTAlbum(tagID);

        if(!talbum)
            return;

        if (talbum == itemDrop->album())
            return;

        KMenu popMenu(this);
        popMenu.addTitle(SmallIcon("digikam"), i18n("Tags"));
        QAction *moveAction = popMenu.addAction(SmallIcon("go-jump"), i18n("&Move Here"));
        popMenu.addSeparator();
        popMenu.addAction(SmallIcon("dialog-cancel"), i18n("C&ancel"));
        popMenu.setMouseTracking(true);
        QAction *choice = popMenu.exec(QCursor::pos());

        if(choice == moveAction)
        {
            TAlbum *newParentTag = 0;

            if (!itemDrop)
            {
                // move dragItem to the root
                newParentTag = AlbumManager::instance()->findTAlbum(0);
            }
            else
            {
                // move dragItem as child of dropItem
                newParentTag = itemDrop->album();
            }

            QString errMsg;
            if (!AlbumManager::instance()->moveTAlbum(talbum, newParentTag, errMsg))
            {
                KMessageBox::error(this, errMsg);
            }

            if(itemDrop && !itemDrop->isOpen())
                itemDrop->setOpen(true);
        }

        return;
    }

    if (DItemDrag::canDecode(e->mimeData()))
    {
        TAlbum *destAlbum = itemDrop->album();
        TAlbum *srcAlbum;

        KUrl::List urls;
        KUrl::List kioURLs;
        QList<int> albumIDs;
        QList<int> imageIDs;

        if (!DItemDrag::decode(e->mimeData(), urls, kioURLs, albumIDs, imageIDs))
            return;

        if (urls.isEmpty() || kioURLs.isEmpty() || albumIDs.isEmpty() || imageIDs.isEmpty())
            return;

        // all the albumids will be the same
        int albumID = albumIDs.first();
        srcAlbum    = AlbumManager::instance()->findTAlbum(albumID);
        if (!srcAlbum)
        {
            kWarning(50003) << "Could not find source album of drag" << endl;
            return;
        }

        if(srcAlbum == destAlbum)
        {
            // Setting the dropped image as the album thumbnail
            // If the CTRL key is pressed, when dropping the image, the
            // thumbnail is set without a popup menu
            bool set = false;
            if (e->keyboardModifiers() == Qt::ControlModifier)
            {
                set = true;
            }
            else
            {
                KMenu popMenu(this);
                popMenu.addTitle(SmallIcon("digikam"), i18n("Tags"));
                QAction *thumbnailAction = popMenu.addAction(i18n("Set as Tag Thumbnail"));
                popMenu.addSeparator();
                popMenu.addAction( SmallIcon("dialog-cancel"), i18n("C&ancel") );

                popMenu.setMouseTracking(true);
                QAction *choice = popMenu.exec(QCursor::pos());
                set = (choice == thumbnailAction);
            }

            if(set)
            {
                QString errMsg;
                AlbumManager::instance()->updateTAlbumIcon(destAlbum, QString(),
                                                                imageIDs.first(), errMsg);
            }
            return;
        }

        // If a CTRL key is pressed while dropping the drag object,
        // the tag is assigned to the images without showing a
        // popup menu.
        bool assign = false;
        if (e->keyboardModifiers() == Qt::ControlModifier)
        {
            assign = true;
        }
        else
        {
            KMenu popMenu(this);
            popMenu.addTitle(SmallIcon("digikam"), i18n("Tags"));
            QAction *assignAction =
                    popMenu.addAction( SmallIcon("tag"), i18n("Assign Tag '%1' to Items", destAlbum->prettyUrl()));
            popMenu.addSeparator();
            popMenu.addAction( SmallIcon("dialog-cancel"), i18n("C&ancel") );

            popMenu.setMouseTracking(true);
            QAction *choice = popMenu.exec(QCursor::pos());
            assign = (choice == assignAction);
        }

        if (assign)
        {
            emit signalProgressBarMode(StatusProgressBar::ProgressBarMode,
                                       i18n("Assign tag to images. Please wait..."));

            int i = 0;
            AlbumLister::instance()->blockSignals(true);
            DatabaseTransaction transaction;
            ScanController::instance()->suspendCollectionScan();
            MetadataHub         hub;

            for (QList<int>::const_iterator it = imageIDs.begin(); it != imageIDs.end(); ++it)
            {
                // create temporary ImageInfo object
                ImageInfo info(*it);

                hub.load(info);
                hub.setTag(destAlbum, true);
                hub.write(info, MetadataHub::PartialWrite);
                hub.write(info.filePath(), MetadataHub::FullWriteIfChanged);

                emit signalProgressValue((int)((i++/(float)imageIDs.count())*100.0));
                kapp->processEvents();
            }
            ScanController::instance()->resumeCollectionScan();
            AlbumLister::instance()->blockSignals(false);

            //ImageAttributesWatch::instance().imagesChanged(destAlbum->id());

            emit signalProgressBarMode(StatusProgressBar::TextMode, QString());
        }
    }
}

void TAlbumListView::refresh()
{
    Q3ListViewItemIterator it(this);

    while (it.current())
    {
        TAlbumCheckListItem* item = dynamic_cast<TAlbumCheckListItem*>(*it);
        if (item)
            item->refresh();
        ++it;
    }
}

void TAlbumListView::slotRefresh(const QMap<int, int>& tagsStatMap)
{
    Q3ListViewItemIterator it(this);

    while (it.current())
    {
        TAlbumCheckListItem* item = dynamic_cast<TAlbumCheckListItem*>(*it);
        if (item)
        {
            if (item->album())
            {
                int id = item->id();
                QMap<int, int>::const_iterator it2 = tagsStatMap.find(id);
                if ( it2 != tagsStatMap.end() )
                    item->setCount(it2.value());
            }
        }
        ++it;
    }

    refresh();
}

}  // namespace Digikam
