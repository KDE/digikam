/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-05
 * Description : tags filter view
 *
 * Copyright (C) 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
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

#include <qheader.h>
#include <qpixmap.h>
#include <qpainter.h>
#include <qtimer.h>
#include <qcursor.h>

// KDE includes.

#include <kabc/stdaddressbook.h>
#include <kpopupmenu.h>
#include <klocale.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kiconloader.h>
#include <kglobalsettings.h>
#include <kcursor.h>
#include <kmessagebox.h>

// Local includes.

#include "ddebug.h"
#include "albummanager.h"
#include "albumsettings.h"
#include "albumdb.h"
#include "album.h"
#include "albumlister.h"
#include "albumthumbnailloader.h"
#include "syncjob.h"
#include "dragobjects.h"
#include "folderitem.h"
#include "imageattributeswatch.h"
#include "imageinfo.h"
#include "metadatahub.h"
#include "tageditdlg.h"
#include "statusprogressbar.h"
#include "tagfilterview.h"
#include "tagfilterview.moc"

// X11 includes.

extern "C"
{
#include <X11/Xlib.h>
}

namespace Digikam
{

class TagFilterViewItem : public FolderCheckListItem
{

public:

    TagFilterViewItem(QListView* parent, TAlbum* tag, bool untagged=false);
    TagFilterViewItem(QListViewItem* parent, TAlbum* tag);

    TAlbum* album() const;
    int     id() const;
    bool    untagged() const;
    void    refresh();
    void    setOpen(bool o);
    void    setCount(int count);
    int     count();
    int     compare(QListViewItem* i, int column, bool ascending) const;

private:

    void    stateChange(bool val);
    void    paintCell(QPainter* p, const QColorGroup & cg, int column, int width, int align);

private:

    bool    m_untagged;

    int     m_count;

    TAlbum *m_album;
};

TagFilterViewItem::TagFilterViewItem(QListView* parent, TAlbum* album, bool untagged)
                 : FolderCheckListItem(parent, album ? album->title() : i18n("Not Tagged"),
                                       QCheckListItem::CheckBox/*Controller*/)
{
    m_album    = album;
    m_untagged = untagged;
    m_count    = 0;
    setDragEnabled(!untagged);

    if (m_album)
        m_album->setExtraData(listView(), this);
}

TagFilterViewItem::TagFilterViewItem(QListViewItem* parent, TAlbum* album)
                 : FolderCheckListItem(parent, album->title(),
                                       QCheckListItem::CheckBox/*Controller*/)
{
    m_album     = album;
    m_untagged  = false;
    m_count     = 0;
    setDragEnabled(true);

    if (m_album)
        m_album->setExtraData(listView(), this);
}

void TagFilterViewItem::refresh()
{
    if (!m_album) return;

    if (AlbumSettings::instance()->getShowFolderTreeViewItemsCount())
    {
        if (isOpen())
            setText(0, QString("%1 (%2)").arg(m_album->title()).arg(m_count));
        else
        {
            int countRecursive = m_count;
            AlbumIterator it(m_album);
            while ( it.current() )
            {
                TagFilterViewItem *item = (TagFilterViewItem*)it.current()->extraData(listView());
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

void TagFilterViewItem::stateChange(bool val)
{
    QCheckListItem::stateChange(val);

    /* NOTE G.Caulier 2007/01/08: this code is now disable because TagFilterViewItem
                        have been changed from QCheckListItem::CheckBoxController
                        to QCheckListItem::CheckBox.

    // All TagFilterViewItems are CheckBoxControllers. If they have no children,
    // they should be of type CheckBox, but that is not possible with our way of adding items.
    // When clicked, children-less items first change to the NoChange state, and a second
    // click is necessary to set them to On and make the filter take effect.
    // So set them to On if the condition is met.
    if (!firstChild() && state() == NoChange)
    {
        setState(On);
    }
    */

    ((TagFilterView*)listView())->stateChanged(this);
}

int TagFilterViewItem::compare(QListViewItem* i, int column, bool ascending) const
{
    if (m_untagged)
        return 1;

    TagFilterViewItem* dItem = dynamic_cast<TagFilterViewItem*>(i);
    if (!dItem)
        return 0;

    if (dItem && dItem->m_untagged)
        return -1;

    return QListViewItem::compare(i, column, ascending);
}

void TagFilterViewItem::paintCell(QPainter* p, const QColorGroup & cg, int column, int width, int align)
{
    if (!m_untagged)
    {
        FolderCheckListItem::paintCell(p, cg, column, width, align);
        return;
    }

    QFont f(listView()->font());
    f.setBold(true);
    f.setItalic(true);
    p->setFont(f);

    QColorGroup mcg(cg);
    mcg.setColor(QColorGroup::Text, Qt::darkRed);

    FolderCheckListItem::paintCell(p, mcg, column, width, align);
}

void TagFilterViewItem::setOpen(bool o)
{
    QListViewItem::setOpen(o);
    refresh();
}

TAlbum* TagFilterViewItem::album() const
{
    return m_album;
}

int TagFilterViewItem::id() const
{
    return m_album ? m_album->id() : 0;
}

void TagFilterViewItem::setCount(int count)
{
    m_count = count;
    refresh();
}

int TagFilterViewItem::count()
{
    return m_count;
}

bool TagFilterViewItem::untagged() const
{
    return m_untagged;
}

// ---------------------------------------------------------------------

class TagFilterViewPrivate
{

public:

    TagFilterViewPrivate()
    {
        ABCMenu        = 0;
        timer          = 0;
        toggleAutoTags = TagFilterView::NoToggleAuto;
        matchingCond   = AlbumLister::OrCondition;
    }

    QTimer                         *timer;

    QPopupMenu                     *ABCMenu;

    TagFilterView::ToggleAutoTags   toggleAutoTags;

    AlbumLister::MatchingCondition  matchingCond;
};

TagFilterView::TagFilterView(QWidget* parent)
             : FolderView(parent, "TagFilterView")
{
    d = new TagFilterViewPrivate;
    d->timer = new QTimer(this);

    addColumn(i18n("Tag Filters"));
    setResizeMode(QListView::LastColumn);
    setRootIsDecorated(true);

    setAcceptDrops(true);
    viewport()->setAcceptDrops(true);

    TagFilterViewItem* notTaggedItem = new TagFilterViewItem(this, 0, true);
    notTaggedItem->setPixmap(0, AlbumThumbnailLoader::instance()->getStandardTagIcon());

    // ------------------------------------------------------------------------

    connect(AlbumManager::instance(), SIGNAL(signalTAlbumsDirty(const QMap<int, int>&)),
            this, SLOT(slotRefresh(const QMap<int, int>&)));

    connect(AlbumManager::instance(), SIGNAL(signalAlbumAdded(Album*)),
            this, SLOT(slotTagAdded(Album*)));

    connect(AlbumManager::instance(), SIGNAL(signalAlbumDeleted(Album*)),
            this, SLOT(slotTagDeleted(Album*)));

    connect(AlbumManager::instance(), SIGNAL(signalAlbumRenamed(Album*)),
            this, SLOT(slotTagRenamed(Album*)));

    connect(AlbumManager::instance(), SIGNAL(signalAlbumsCleared()),
            this, SLOT(slotClear()));

    connect(AlbumManager::instance(), SIGNAL(signalAlbumIconChanged(Album*)),
            this, SLOT(slotAlbumIconChanged(Album*)));

    connect(AlbumManager::instance(), SIGNAL(signalTAlbumMoved(TAlbum*, TAlbum*)),
            this, SLOT(slotTagMoved(TAlbum*, TAlbum*)));

    // ------------------------------------------------------------------------

    AlbumThumbnailLoader *loader = AlbumThumbnailLoader::instance();

    connect(loader, SIGNAL(signalThumbnail(Album *, const QPixmap&)),
            this, SLOT(slotGotThumbnailFromIcon(Album *, const QPixmap&)));

    connect(loader, SIGNAL(signalFailed(Album *)),
            this, SLOT(slotThumbnailLost(Album *)));

    connect(loader, SIGNAL(signalReloadThumbnails()),
            this, SLOT(slotReloadThumbnails()));

    connect(this, SIGNAL(contextMenuRequested(QListViewItem*, const QPoint&, int)),
            this, SLOT(slotContextMenu(QListViewItem*, const QPoint&, int)));

    connect(d->timer, SIGNAL(timeout()),
            this, SLOT(slotTimeOut()));

    // ------------------------------------------------------------------------

    KConfig* config = kapp->config();
    config->setGroup("Tag Filters View");
    d->matchingCond = (AlbumLister::MatchingCondition)(config->readNumEntry("Matching Condition",
                                                       AlbumLister::OrCondition));

    d->toggleAutoTags = (ToggleAutoTags)(config->readNumEntry("Toggle Auto Tags", NoToggleAuto));
}

TagFilterView::~TagFilterView()
{
    KConfig* config = kapp->config();
    config->setGroup("Tag Filters View");
    config->writeEntry("Matching Condition", (int)(d->matchingCond));
    config->writeEntry("Toggle Auto Tags", (int)(d->toggleAutoTags));
    config->sync();

    delete d->timer;
    delete d;
}

void TagFilterView::slotTextTagFilterChanged(const QString& filter)
{
    if (filter.isEmpty())
    {
        collapseView();
        return;
    }

    QString search = filter.lower();

    bool atleastOneMatch = false;

    AlbumList tList = AlbumManager::instance()->allTAlbums();
    for (AlbumList::iterator it = tList.begin(); it != tList.end(); ++it)
    {
        TAlbum* talbum  = (TAlbum*)(*it);

        // don't touch the root Album
        if (talbum->isRoot())
            continue;

        bool match = talbum->title().lower().contains(search);
        bool doesExpand = false;
        if (!match)
        {
            // check if any of the parents match the search
            Album* parent = talbum->parent();
            while (parent && !parent->isRoot())
            {
                if (parent->title().lower().contains(search))
                {
                    match = true;
                    break;
                }

                parent = parent->parent();
            }
        }

        if (!match)
        {
            // check if any of the children match the search
            AlbumIterator it(talbum);
            while (it.current())
            {
                if ((*it)->title().lower().contains(search))
                {
                    match = true;
                    doesExpand = true;
                    break;
                }
                ++it;
            }
        }

        TagFilterViewItem* viewItem = (TagFilterViewItem*) talbum->extraData(this);

        if (match)
        {
            atleastOneMatch = true;

            if (viewItem)
            {
                viewItem->setVisible(true);
                viewItem->setOpen(doesExpand);
        }
        }
        else
        {
            if (viewItem)
            {
                viewItem->setVisible(false);
                viewItem->setOpen(false);
            }
        }
    }

    emit signalTextTagFilterMatch(atleastOneMatch);
}

void TagFilterView::stateChanged(TagFilterViewItem* item)
{
    ToggleAutoTags oldAutoTags = d->toggleAutoTags;

    switch(d->toggleAutoTags)
    {
        case Children:
            d->toggleAutoTags = TagFilterView::NoToggleAuto;
            toggleChildTags(item, item->isOn());
            d->toggleAutoTags = oldAutoTags;
            break;
        case Parents:
            d->toggleAutoTags = TagFilterView::NoToggleAuto;
            toggleParentTags(item, item->isOn());
            d->toggleAutoTags = oldAutoTags;
            break;
        case ChildrenAndParents:
            d->toggleAutoTags = TagFilterView::NoToggleAuto;
            toggleChildTags(item, item->isOn());
            toggleParentTags(item, item->isOn());
            d->toggleAutoTags = oldAutoTags;
            break;
        default:
            break;
    }

    triggerChange();
}

void TagFilterView::triggerChange()
{
    d->timer->start(50, true);
}

QDragObject* TagFilterView::dragObject()
{
    TagFilterViewItem *item = dynamic_cast<TagFilterViewItem*>(dragItem());
    if(!item)
        return 0;

    TagDrag *t = new TagDrag(item->id(), this);
    t->setPixmap(*item->pixmap(0));

    return t;
}

bool TagFilterView::acceptDrop(const QDropEvent *e) const
{
    QPoint vp = contentsToViewport(e->pos());
    TagFilterViewItem *itemDrop = dynamic_cast<TagFilterViewItem*>(itemAt(vp));
    TagFilterViewItem *itemDrag = dynamic_cast<TagFilterViewItem*>(dragItem());

    if(TagDrag::canDecode(e) || TagListDrag::canDecode(e))
    {
        // Allow dragging at the root, to move the tag to the root
        if(!itemDrop)
            return true;

        // Do not allow dragging at the "Not Tagged" item.
        if (itemDrop->untagged())
            return false;

        // Dragging an item on itself makes no sense
        if(itemDrag == itemDrop)
            return false;

        // Dragging a parent on its child makes no sense
        if(itemDrag && itemDrag->album()->isAncestorOf(itemDrop->album()))
            return false;

        return true;
    }

    if (ItemDrag::canDecode(e) && itemDrop && !itemDrop->untagged())
    {
        TAlbum *tag = itemDrop->album();

        if (tag)
        {
            if (tag->parent())
            {
                // Only other possibility is image items being dropped
                // And allow this only if there is a Tag to be dropped
                // on and also the Tag is not root or "Not Tagged" item.
                return true;
            }
        }
    }

    return false;
}

void TagFilterView::contentsDropEvent(QDropEvent *e)
{
    FolderView::contentsDropEvent(e);

    if (!acceptDrop(e))
        return;

    QPoint vp = contentsToViewport(e->pos());
    TagFilterViewItem *itemDrop = dynamic_cast<TagFilterViewItem*>(itemAt(vp));

    if (!itemDrop || itemDrop->untagged())
        return;

    if(TagDrag::canDecode(e))
    {
        QByteArray ba = e->encodedData("digikam/tag-id");
        QDataStream ds(ba, IO_ReadOnly);
        int tagID;
        ds >> tagID;

        AlbumManager* man = AlbumManager::instance();
        TAlbum* talbum    = man->findTAlbum(tagID);

        if(!talbum)
            return;

        if (talbum == itemDrop->album())
            return;

        KPopupMenu popMenu(this);
        popMenu.insertTitle(SmallIcon("digikam"), i18n("Tag Filters"));
        popMenu.insertItem(SmallIcon("goto"), i18n("&Move Here"), 10);
        popMenu.insertSeparator(-1);
        popMenu.insertItem(SmallIcon("cancel"), i18n("C&ancel"), 20);
        popMenu.setMouseTracking(true);
        int id = popMenu.exec(QCursor::pos());

        if(id == 10)
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

    if (ItemDrag::canDecode(e))
    {
        TAlbum *destAlbum = itemDrop->album();

        KURL::List      urls;
        KURL::List      kioURLs;
        QValueList<int> albumIDs;
        QValueList<int> imageIDs;

        if (!ItemDrag::decode(e, urls, kioURLs, albumIDs, imageIDs))
            return;

        if (urls.isEmpty() || kioURLs.isEmpty() || albumIDs.isEmpty() || imageIDs.isEmpty())
            return;

        int id = 0;
        char keys_return[32];
        XQueryKeymap(x11Display(), keys_return);
        int key_1 = XKeysymToKeycode(x11Display(), 0xFFE3);
        int key_2 = XKeysymToKeycode(x11Display(), 0xFFE4);

        // If a ctrl key is pressed while dropping the drag object,
        // the tag is assigned to the images without showing a
        // popup menu.
        if (((keys_return[key_1 / 8]) && (1 << (key_1 % 8))) ||
            ((keys_return[key_2 / 8]) && (1 << (key_2 % 8))))
        {
            id = 10;
        }
        else
        {
            KPopupMenu popMenu(this);
            popMenu.insertTitle(SmallIcon("digikam"), i18n("Tag Filters"));
            popMenu.insertItem(SmallIcon("tag"), i18n("Assign Tag '%1' to Items")
                                .arg(destAlbum->prettyURL()), 10) ;
            popMenu.insertItem(i18n("Set as Tag Thumbnail"),  11);
            popMenu.insertSeparator(-1);
            popMenu.insertItem(SmallIcon("cancel"), i18n("C&ancel"));

            popMenu.setMouseTracking(true);
            id = popMenu.exec(QCursor::pos());
        }

        if (id == 10)
        {
            emit signalProgressBarMode(StatusProgressBar::ProgressBarMode,
                                       i18n("Assigning image tags. Please wait..."));

            AlbumLister::instance()->blockSignals(true);
            AlbumManager::instance()->albumDB()->beginTransaction();
            int i=0;
            for (QValueList<int>::const_iterator it = imageIDs.begin();
                 it != imageIDs.end(); ++it)
            {
                // create temporary ImageInfo object
                ImageInfo info(*it);

                MetadataHub hub;
                hub.load(&info);
                hub.setTag(destAlbum, true);
                hub.write(&info, MetadataHub::PartialWrite);
                hub.write(info.filePath(), MetadataHub::FullWriteIfChanged);

                emit signalProgressValue((int)((i++/(float)imageIDs.count())*100.0));
                kapp->processEvents();
            }
            AlbumLister::instance()->blockSignals(false);
            AlbumManager::instance()->albumDB()->commitTransaction();

            ImageAttributesWatch::instance()->imagesChanged(destAlbum->id());

            emit signalProgressBarMode(StatusProgressBar::TextMode, QString());
        }
        else if(id == 11)
        {
            QString errMsg;
            AlbumManager::instance()->updateTAlbumIcon(destAlbum, QString(),
                                                       imageIDs.first(), errMsg);
        }
    }
}

void TagFilterView::slotTagAdded(Album* album)
{
    if (!album || album->isRoot())
        return;

    TAlbum* tag = dynamic_cast<TAlbum*>(album);
    if (!tag)
        return;

    if (tag->parent()->isRoot())
    {
        new TagFilterViewItem(this, tag);
    }
    else
    {
        TagFilterViewItem* parent = (TagFilterViewItem*)(tag->parent()->extraData(this));
        if (!parent)
        {
            DWarning() << k_funcinfo << " Failed to find parent for Tag "
                       << tag->tagPath() << endl;
            return;
        }

        new TagFilterViewItem(parent, tag);
    }

    setTagThumbnail(tag);
}

void TagFilterView::slotTagRenamed(Album* album)
{
    if (!album)
        return;

    TAlbum* tag = dynamic_cast<TAlbum*>(album);
    if (!tag)
        return;

    TagFilterViewItem* item = (TagFilterViewItem*)(tag->extraData(this));
    if (item)
        item->refresh();
}

void TagFilterView::slotTagMoved(TAlbum* tag, TAlbum* newParent)
{
    if (!tag || !newParent)
        return;

    TagFilterViewItem* item = (TagFilterViewItem*)(tag->extraData(this));
    if (!item)
        return;

    if (item->parent())
    {
        QListViewItem* oldPItem = item->parent();
        oldPItem->takeItem(item);

        TagFilterViewItem* newPItem = (TagFilterViewItem*)(newParent->extraData(this));
        if (newPItem)
            newPItem->insertItem(item);
        else
            insertItem(item);
    }
    else
    {
        takeItem(item);

        TagFilterViewItem* newPItem = (TagFilterViewItem*)(newParent->extraData(this));

        if (newPItem)
            newPItem->insertItem(item);
        else
            insertItem(item);
    }
}

void TagFilterView::slotTagDeleted(Album* album)
{
    if (!album || album->isRoot())
        return;

    TAlbum* tag = dynamic_cast<TAlbum*>(album);
    if (!tag)
        return;

    TagFilterViewItem* item = (TagFilterViewItem*)(album->extraData(this));
    if (!item)
        return;

    // NOTE: see B.K.O #158558: unselected tag filter and all childrens before to delete it.
    toggleChildTags(item, false);
    item->setOn(false);

    album->removeExtraData(this);
    delete item;
}

void TagFilterView::setTagThumbnail(TAlbum *album)
{
    if(!album)
        return;

    TagFilterViewItem* item = (TagFilterViewItem*) album->extraData(this);

    if(!item)
        return;

    AlbumThumbnailLoader *loader = AlbumThumbnailLoader::instance();
    QPixmap icon;
    if (!loader->getTagThumbnail(album, icon))
    {
        if (icon.isNull())
        {
            item->setPixmap(0, loader->getStandardTagIcon(album));
        }
        else
        {
            QPixmap blendedIcon = loader->blendIcons(loader->getStandardTagIcon(), icon);
            item->setPixmap(0, blendedIcon);
        }
    }
    else
    {
        // for the time being, set standard icon
        item->setPixmap(0, loader->getStandardTagIcon(album));
    }
}

void TagFilterView::slotGotThumbnailFromIcon(Album *album, const QPixmap& thumbnail)
{
    if(!album || album->type() != Album::TAG)
        return;

    TagFilterViewItem* item = (TagFilterViewItem*)album->extraData(this);

    if(!item)
        return;

    AlbumThumbnailLoader *loader = AlbumThumbnailLoader::instance();
    QPixmap blendedIcon = loader->blendIcons(loader->getStandardTagIcon(), thumbnail);
    item->setPixmap(0, blendedIcon);
}

void TagFilterView::slotThumbnailLost(Album *)
{
    // we already set the standard icon before loading
}

void TagFilterView::slotReloadThumbnails()
{
    AlbumList tList = AlbumManager::instance()->allTAlbums();
    for (AlbumList::iterator it = tList.begin(); it != tList.end(); ++it)
    {
        TAlbum* tag  = (TAlbum*)(*it);
        setTagThumbnail(tag);
    }
}

void TagFilterView::slotAlbumIconChanged(Album* album)
{
    if(!album || album->type() != Album::TAG)
        return;

    setTagThumbnail((TAlbum *)album);
}

void TagFilterView::slotClear()
{
    clear();

    TagFilterViewItem* notTaggedItem = new TagFilterViewItem(this, 0, true);
    notTaggedItem->setPixmap(0, AlbumThumbnailLoader::instance()->getStandardTagIcon());
}

void TagFilterView::slotTimeOut()
{
    QValueList<int> filterTags;

    bool showUnTagged = false;

    QListViewItemIterator it(this, QListViewItemIterator::Checked);
    while (it.current())
    {
        TagFilterViewItem* item = (TagFilterViewItem*)it.current();
        if (item->album())
            filterTags.append(item->album()->id());
        else if (item->untagged())
            showUnTagged = true;
        ++it;
    }

    AlbumLister::instance()->setTagFilter(filterTags, d->matchingCond, showUnTagged);
}

void TagFilterView::slotContextMenu(QListViewItem* it, const QPoint&, int)
{
    TagFilterViewItem *item = dynamic_cast<TagFilterViewItem*>(it);
    if (item && item->untagged())
        return;

    d->ABCMenu = new QPopupMenu;

    connect(d->ABCMenu, SIGNAL( aboutToShow() ),
            this, SLOT( slotABCContextMenu() ));

    KPopupMenu popmenu(this);
    popmenu.insertTitle(SmallIcon("digikam"), i18n("Tag Filters"));
    popmenu.insertItem(SmallIcon("tag-new"), i18n("New Tag..."), 10);
    popmenu.insertItem(SmallIcon("tag-addressbook"), i18n("Create Tag From AddressBook"), d->ABCMenu);

    if (item)
    {
        popmenu.insertItem(SmallIcon("tag-properties"), i18n("Edit Tag Properties..."), 11);
        popmenu.insertItem(SmallIcon("tag-reset"),      i18n("Reset Tag Icon"),         13);
        popmenu.insertSeparator(-1);
        popmenu.insertItem(SmallIcon("tag-delete"),     i18n("Delete Tag"),             12);
    }

    popmenu.insertSeparator(-1);

    QPopupMenu selectTagsMenu;
    selectTagsMenu.insertItem(i18n("All Tags"),   14);
    if (item)
    {
        selectTagsMenu.insertSeparator(-1);
        selectTagsMenu.insertItem(i18n("Children"),     17);
        selectTagsMenu.insertItem(i18n("Parents"),    19);
    }
    popmenu.insertItem(i18n("Select"), &selectTagsMenu);

    QPopupMenu deselectTagsMenu;
    deselectTagsMenu.insertItem(i18n("All Tags"), 15);
    if (item)
    {
        deselectTagsMenu.insertSeparator(-1);
        deselectTagsMenu.insertItem(i18n("Children"),   18);
        deselectTagsMenu.insertItem(i18n("Parents"),  20);
    }
    popmenu.insertItem(i18n("Deselect"), &deselectTagsMenu);

    popmenu.insertItem(i18n("Invert Selection"),  16);
    popmenu.insertSeparator(-1);

    QPopupMenu toggleAutoMenu;
    toggleAutoMenu.setCheckable(true);
    toggleAutoMenu.insertItem(i18n("None"),    21);
    toggleAutoMenu.insertSeparator(-1);
    toggleAutoMenu.insertItem(i18n("Children"),  22);
    toggleAutoMenu.insertItem(i18n("Parents"), 23);
    toggleAutoMenu.insertItem(i18n("Both"),    24);
    toggleAutoMenu.setItemChecked(21 + d->toggleAutoTags, true);
    popmenu.insertItem(i18n("Toggle Auto"), &toggleAutoMenu);

    QPopupMenu matchingCongMenu;
    matchingCongMenu.setCheckable(true);
    matchingCongMenu.insertItem(i18n("Or Between Tags"),  25);
    matchingCongMenu.insertItem(i18n("And Between Tags"), 26);
    matchingCongMenu.setItemChecked((d->matchingCond == AlbumLister::OrCondition) ? 25 : 26, true);
    popmenu.insertItem(i18n("Matching Condition"), &matchingCongMenu);

    ToggleAutoTags oldAutoTags = d->toggleAutoTags;

    int choice = popmenu.exec((QCursor::pos()));
    switch( choice )
    {
        case 10:    // New Tag.
        {
            tagNew(item);
            break;
        }
        case 11:    // Edit Tag Properties.
        {
            tagEdit(item);
            break;
        }
        case 12:    // Delete Tag.
        {
            tagDelete(item);
            break;
        }
        case 13:    // Reset Tag Icon.
        {
            QString errMsg;
            AlbumManager::instance()->updateTAlbumIcon(item->album(), QString("tag"), 0, errMsg);
            break;
        }
        case 14:    // Select All Tags.
        {
            d->toggleAutoTags = TagFilterView::NoToggleAuto;
            QListViewItemIterator it(this, QListViewItemIterator::NotChecked);
            while (it.current())
            {
                TagFilterViewItem* item = (TagFilterViewItem*)it.current();

                // Ignore "Not Tagged" tag filter.
                if (!item->untagged())
                    item->setOn(true);

                ++it;
            }
            triggerChange();
            d->toggleAutoTags = oldAutoTags;
            break;
        }
        case 15:    // Deselect All Tags.
        {
            d->toggleAutoTags = TagFilterView::NoToggleAuto;
            QListViewItemIterator it(this, QListViewItemIterator::Checked);
            while (it.current())
            {
                TagFilterViewItem* item = (TagFilterViewItem*)it.current();

                // Ignore "Not Tagged" tag filter.
                if (!item->untagged())
                    item->setOn(false);

                ++it;
            }
            triggerChange();
            d->toggleAutoTags = oldAutoTags;
            break;
        }
        case 16:       // Invert All Tags Selection.
        {
            d->toggleAutoTags = TagFilterView::NoToggleAuto;
            QListViewItemIterator it(this);
            while (it.current())
            {
                TagFilterViewItem* item = (TagFilterViewItem*)it.current();

                // Ignore "Not Tagged" tag filter.
                if (!item->untagged())
                    item->setOn(!item->isOn());

                ++it;
            }
            triggerChange();
            d->toggleAutoTags = oldAutoTags;
            break;
        }
        case 17:   // Select Child Tags.
        {
            d->toggleAutoTags = TagFilterView::NoToggleAuto;
            toggleChildTags(item, true);
            TagFilterViewItem *tItem = (TagFilterViewItem*)item->album()->extraData(this);
            tItem->setOn(true);
            d->toggleAutoTags = oldAutoTags;
            break;
        }
        case 18:   // Deselect Child Tags.
        {
            d->toggleAutoTags = TagFilterView::NoToggleAuto;
            toggleChildTags(item, false);
            TagFilterViewItem *tItem = (TagFilterViewItem*)item->album()->extraData(this);
            tItem->setOn(false);
            d->toggleAutoTags = oldAutoTags;
            break;
        }
        case 19:   // Select Parent Tags.
        {
            d->toggleAutoTags = TagFilterView::NoToggleAuto;
            toggleParentTags(item, true);
            TagFilterViewItem *tItem = (TagFilterViewItem*)item->album()->extraData(this);
            tItem->setOn(true);
            d->toggleAutoTags = oldAutoTags;
            break;
        }
        case 20:   // Deselect Parent Tags.
        {
            d->toggleAutoTags = TagFilterView::NoToggleAuto;
            toggleParentTags(item, false);
            TagFilterViewItem *tItem = (TagFilterViewItem*)item->album()->extraData(this);
            tItem->setOn(false);
            d->toggleAutoTags = oldAutoTags;
            break;
        }
        case 21:   // No toggle auto tags.
        {
            d->toggleAutoTags = NoToggleAuto;
            break;
        }
        case 22:   // Toggle auto Children tags.
        {
            d->toggleAutoTags = Children;
            break;
        }
        case 23:   // Toggle auto Parents tags.
        {
            d->toggleAutoTags = Parents;
            break;
        }
        case 24:   // Toggle auto Children and Parents tags.
        {
            d->toggleAutoTags = ChildrenAndParents;
            break;
        }
        case 25:    // Or Between Tags.
        {
            d->matchingCond = AlbumLister::OrCondition;
            triggerChange();
            break;
        }
        case 26:    // And Between Tags.
        {
            d->matchingCond = AlbumLister::AndCondition;
            triggerChange();
            break;
        }
        default:
            break;
    }

    if ( choice > 100 )
    {
        tagNew(item, d->ABCMenu->text( choice ), "tag-people" );
    }

    delete d->ABCMenu;
    d->ABCMenu = 0;
}

void TagFilterView::slotABCContextMenu()
{
    d->ABCMenu->clear();

    int counter = 100;
    KABC::AddressBook* ab = KABC::StdAddressBook::self();
    QStringList names;
    for ( KABC::AddressBook::Iterator it = ab->begin(); it != ab->end(); ++it )
    {
        names.push_back(it->formattedName());
    }

    qHeapSort(names);

    for ( QStringList::Iterator it = names.begin(); it != names.end(); ++it )
    {
        QString name = *it;
        if ( !name.isNull() )
            d->ABCMenu->insertItem( name, ++counter );
    }

    if (counter == 100)
    {
        d->ABCMenu->insertItem( i18n("No AddressBook entries found"), ++counter );
        d->ABCMenu->setItemEnabled( counter, false );
    }
}

void TagFilterView::tagNew(TagFilterViewItem* item, const QString& _title, const QString& _icon)
{
    TAlbum  *parent;
    QString  title    = _title;
    QString  icon     = _icon;
    AlbumManager *man = AlbumManager::instance();

    if (!item)
        parent = man->findTAlbum(0);
    else
        parent = item->album();

    if (title.isNull())
    {
        if (!TagEditDlg::tagCreate(kapp->activeWindow(), parent, title, icon))
            return;
    }

    QMap<QString, QString> errMap;
    AlbumList tList = TagEditDlg::createTAlbum(parent, title, icon, errMap);
    TagEditDlg::showtagsListCreationError(kapp->activeWindow(), errMap);

    for (AlbumList::iterator it = tList.begin(); it != tList.end(); ++it)
    {
        TagFilterViewItem* item = (TagFilterViewItem*)(*it)->extraData(this);
        if (item)
        {
            clearSelection();
            setSelected(item, true);
            setCurrentItem(item);
            ensureItemVisible(item);
        }
    }
}

void TagFilterView::tagEdit(TagFilterViewItem* item)
{
    if (!item)
        return;

    TAlbum *tag = item->album();
    if (!tag)
        return;

    QString title, icon;
    if (!TagEditDlg::tagEdit(kapp->activeWindow(), tag, title, icon))
    {
        return;
    }

    AlbumManager* man = AlbumManager::instance();

    if (tag->title() != title)
    {
        QString errMsg;
        if(!man->renameTAlbum(tag, title, errMsg))
            KMessageBox::error(0, errMsg);
        else
            item->refresh();
    }

    if (tag->icon() != icon)
    {
        QString errMsg;
        if (!man->updateTAlbumIcon(tag, icon, 0, errMsg))
            KMessageBox::error(0, errMsg);
        else
            setTagThumbnail(tag);
    }
}

void TagFilterView::tagDelete(TagFilterViewItem* item)
{
    if (!item)
        return;

    TAlbum *tag = item->album();
    if (!tag || tag->isRoot())
        return;

    // find number of subtags
    int children = 0;
    AlbumIterator iter(tag);
    while(iter.current())
    {
        children++;
        ++iter;
    }

    AlbumManager* man = AlbumManager::instance();

    if (children)
    {
        int result = KMessageBox::warningContinueCancel(this,
                     i18n("Tag '%1' has one subtag. "
                          "Deleting this will also delete "
                          "the subtag. "
                          "Do you want to continue?",
                          "Tag '%1' has %n subtags. "
                          "Deleting this will also delete "
                          "the subtags. "
                          "Do you want to continue?",
                          children).arg(tag->title()));

        if(result != KMessageBox::Continue)
            return;
    }

    QString message;
    LLongList assignedItems = man->albumDB()->getItemIDsInTag(tag->id());
    if (!assignedItems.isEmpty())
    {
        message = i18n("Tag '%1' is assigned to one item. "
                        "Do you want to continue?",
                        "Tag '%1' is assigned to %n items. "
                        "Do you want to continue?",
                        assignedItems.count()).arg(tag->title());
    }
    else
    {
        message = i18n("Delete '%1' tag?").arg(tag->title());
    }

    int result = KMessageBox::warningContinueCancel(0, message,
                                                    i18n("Delete Tag"),
                                                    KGuiItem(i18n("Delete"),
                                                    "editdelete"));

    if (result == KMessageBox::Continue)
    {
        QString errMsg;
        if (!man->deleteTAlbum(tag, errMsg))
            KMessageBox::error(0, errMsg);
    }
}

void TagFilterView::toggleChildTags(TagFilterViewItem* tItem, bool b)
{
    if (!tItem)
        return;

    TAlbum *album = tItem->album();
    if (!album)
        return;

    AlbumIterator it(album);
    while ( it.current() )
    {
        TAlbum *ta              = (TAlbum*)it.current();
        TagFilterViewItem *item = (TagFilterViewItem*)ta->extraData(this);
        if (item)
        {
            if (item->isVisible())
                item->setOn(b);
        }
        ++it;
    }
}

void TagFilterView::toggleParentTags(TagFilterViewItem* tItem, bool b)
{
    if (!tItem)
        return;

    TAlbum *album = tItem->album();
    if (!album)
        return;

    QListViewItemIterator it(this);
    while (it.current())
    {
        TagFilterViewItem* item = dynamic_cast<TagFilterViewItem*>(it.current());
        if (item->isVisible())
        {
            Album *a = dynamic_cast<Album*>(item->album());
            if (a)
            {
                if (a == album->parent())
                {
                    item->setOn(b);
                    toggleParentTags(item, b);
                }
            }
        }
        ++it;
    }
}

void TagFilterView::refresh()
{
    QListViewItemIterator it(this);

    while (it.current())
    {
        TagFilterViewItem* item = dynamic_cast<TagFilterViewItem*>(*it);
        if (item)
            item->refresh();
        ++it;
    }
}

void TagFilterView::slotRefresh(const QMap<int, int>& tagsStatMap)
{
    QListViewItemIterator it(this);

    while (it.current())
    {
        TagFilterViewItem* item = dynamic_cast<TagFilterViewItem*>(*it);
        if (item)
        {
            if (item->album())
            {
                int id = item->id();
                QMap<int, int>::const_iterator it2 = tagsStatMap.find(id);
                if ( it2 != tagsStatMap.end() )
                    item->setCount(it2.data());
            }
        }
        ++it;
    }

    refresh();
}

void TagFilterView::slotResetTagFilters()
{
    QListViewItemIterator it(this);

    while (it.current())
    {
        TagFilterViewItem* item = dynamic_cast<TagFilterViewItem*>(*it);
        if (item && item->isOn())
            item->setOn(false);
        ++it;
    }
}

}  // namespace Digikam
