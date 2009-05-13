/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-05
 * Description : tags filter view
 *
 * Copyright (C) 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009 by Andi Clemens <andi dot clemens at gmx dot net>
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

#include "tagfilterview.h"
#include "tagfilterview.moc"

// Qt includes

#include <QList>
#include <QPixmap>
#include <QPainter>
#include <QTimer>
#include <QCursor>
#include <QDropEvent>
#include <QMouseEvent>

// KDE includes

#include <kdebug.h>
#include <kmenu.h>
#include <klocale.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kiconloader.h>
#include <kglobalsettings.h>
#include <kcursor.h>
#include <kmessagebox.h>
#include <kglobal.h>
#include <kselectaction.h>
#include <kstringhandler.h>

// Local includes

#include "albummanager.h"
#include "albumdb.h"
#include "album.h"
#include "albumlister.h"
#include "albumsettings.h"
#include "albumthumbnailloader.h"
#include "databasetransaction.h"
#include "ddragobjects.h"
#include "folderitem.h"
#include "imageattributeswatch.h"
#include "imageinfo.h"
#include "metadatahub.h"
#include "scancontroller.h"
#include "statusprogressbar.h"
#include "syncjob.h"
#include "tageditdlg.h"

#include "config-digikam.h"
#ifdef HAVE_KDEPIMLIBS
#include <kabc/stdaddressbook.h>
#endif // HAVE_KDEPIMLIBS

namespace Digikam
{

class TagFilterViewItem : public FolderCheckListItem
{

public:

    TagFilterViewItem(Q3ListView* parent, TAlbum* tag, bool untagged=false);
    TagFilterViewItem(Q3ListViewItem* parent, TAlbum* tag);

    TAlbum* album() const;
    int     id() const;
    bool    untagged() const;
    void    refresh();
    void    setOpen(bool o);
    void    setCount(int count);
    int     count();
    int     compare(Q3ListViewItem* i, int column, bool ascending) const;

private:

    void    stateChange(bool val);
    void    paintCell(QPainter* p, const QColorGroup & cg, int column, int width, int align);

private:

    bool    m_untagged;

    int     m_count;

    TAlbum *m_album;
};

TagFilterViewItem::TagFilterViewItem(Q3ListView* parent, TAlbum* album, bool untagged)
                 : FolderCheckListItem(parent, album ? album->title() : i18n("Not Tagged"),
                                       Q3CheckListItem::CheckBox/*Controller*/)
{
    m_album    = album;
    m_untagged = untagged;
    m_count    = 0;
    setDragEnabled(!untagged);

    if (m_album)
        m_album->setExtraData(listView(), this);
}

TagFilterViewItem::TagFilterViewItem(Q3ListViewItem* parent, TAlbum* album)
                 : FolderCheckListItem(parent, album->title(),
                                       Q3CheckListItem::CheckBox/*Controller*/)
{
    m_album    = album;
    m_untagged = false;
    m_count    = 0;
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
    Q3CheckListItem::stateChange(val);

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

int TagFilterViewItem::compare(Q3ListViewItem* i, int column, bool ascending) const
{
    if (m_untagged)
        return 1;

    TagFilterViewItem* dItem = dynamic_cast<TagFilterViewItem*>(i);
    if (!dItem)
        return 0;

    if (dItem && dItem->m_untagged)
        return -1;

    return KStringHandler::naturalCompare(key(column, ascending), i->key(column, ascending));
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
    Q3ListViewItem::setOpen(o);
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

    KMenu                          *ABCMenu;

    TagFilterView::ToggleAutoTags   toggleAutoTags;

    AlbumLister::MatchingCondition  matchingCond;
};


TagFilterView::TagFilterView(QWidget* parent)
             : FolderView(parent, "TagFilterView"),
               d(new TagFilterViewPrivate)
{
    d->timer = new QTimer(this);

    addColumn(i18n("Tag Filters"));
    setResizeMode(Q3ListView::LastColumn);
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

    connect(this, SIGNAL(contextMenuRequested(Q3ListViewItem*, const QPoint&, int)),
            this, SLOT(slotContextMenu(Q3ListViewItem*, const QPoint&, int)));

    connect(d->timer, SIGNAL(timeout()),
            this, SLOT(slotTimeOut()));

    // ------------------------------------------------------------------------

    connect(this, SIGNAL(assignTags(int, const QList<int> &)),
            this, SLOT(slotAssignTags(int, const QList<int> &)),
            Qt::QueuedConnection);

    // ------------------------------------------------------------------------

    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group("Tag Filters View");
    d->matchingCond = (AlbumLister::MatchingCondition)(group.readEntry("Matching Condition",
                                                       (int)AlbumLister::OrCondition));

    d->toggleAutoTags = (ToggleAutoTags)(group.readEntry("Toggle Auto Tags", (int)NoToggleAuto));
}

TagFilterView::~TagFilterView()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group("Tag Filters View");
    group.writeEntry("Matching Condition", (int)(d->matchingCond));
    group.writeEntry("Toggle Auto Tags", (int)(d->toggleAutoTags));
    config->sync();

    saveViewState();

    delete d->timer;
    delete d;
}

void TagFilterView::slotTextTagFilterChanged(const SearchTextSettings& settings)
{
    if (settings.text.isEmpty())
    {
        collapseView(FolderView::RestoreSelectedItem);
        return;
    }

    QString search       = settings.text;
    bool atleastOneMatch = false;

    AlbumList tList = AlbumManager::instance()->allTAlbums();
    for (AlbumList::const_iterator it = tList.constBegin(); it != tList.constEnd(); ++it)
    {
        TAlbum* talbum  = (TAlbum*)(*it);

        // don't touch the root Album
        if (talbum->isRoot())
            continue;

        bool match      = talbum->title().contains(search, settings.caseSensitive);
        bool doesExpand = false;
        if (!match)
        {
            // check if any of the parents match the search
            Album* parent = talbum->parent();
            while (parent && !parent->isRoot())
            {
                if (parent->title().contains(search, settings.caseSensitive))
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
                if ((*it)->title().contains(search, settings.caseSensitive))
                {
                    match      = true;
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
    d->timer->setSingleShot(true);
    d->timer->start(50);
}

QDrag* TagFilterView::makeDragObject()
{
    TagFilterViewItem *item = dynamic_cast<TagFilterViewItem*>(dragItem());
    if(!item)
        return 0;

    QDrag *drag = new QDrag(this);
    drag->setMimeData(new DTagDrag(item->album()->id()));
    drag->setPixmap(*item->pixmap(0));

    return drag;
}

bool TagFilterView::acceptDrop(const QDropEvent *e) const
{
    QPoint vp = contentsToViewport(e->pos());
    TagFilterViewItem *itemDrop = dynamic_cast<TagFilterViewItem*>(itemAt(vp));
    TagFilterViewItem *itemDrag = dynamic_cast<TagFilterViewItem*>(dragItem());

    if(DTagDrag::canDecode(e->mimeData()) || DTagListDrag::canDecode(e->mimeData()))
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

    if (DItemDrag::canDecode(e->mimeData()) && itemDrop && !itemDrop->untagged())
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
        popMenu.addTitle(SmallIcon("digikam"), i18n("Tag Filters"));
        QAction *gotoAction = popMenu.addAction(SmallIcon("go-jump"), i18n("&Move Here"));
        popMenu.addSeparator();
        popMenu.addAction(SmallIcon("dialog-cancel"), i18n("C&ancel"));
        popMenu.setMouseTracking(true);
        QAction *choice = popMenu.exec(QCursor::pos());

        if(choice == gotoAction)
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

        KUrl::List urls;
        KUrl::List kioURLs;
        QList<int> albumIDs;
        QList<int> imageIDs;

        if (!DItemDrag::decode(e->mimeData(), urls, kioURLs, albumIDs, imageIDs))
            return;

        if (urls.isEmpty() || kioURLs.isEmpty() || albumIDs.isEmpty() || imageIDs.isEmpty())
            return;


        // If a ctrl key is pressed while dropping the drag object,
        // the tag is assigned to the images without showing a
        // popup menu.
        bool assignTag = false, setThumbnail = false;
        if (e->keyboardModifiers() == Qt::ControlModifier)
        {
            assignTag = true;
        }
        else
        {
            KMenu popMenu(this);
            popMenu.addTitle(SmallIcon("digikam"), i18n("Tag Filters"));
            QAction *assignAction = popMenu.addAction(SmallIcon("tag"), i18n("Assign Tag '%1' to Items", destAlbum->prettyUrl()));
            QAction *thumbnailAction = popMenu.addAction(i18n("Set as Tag Thumbnail"));
            popMenu.addSeparator();
            popMenu.addAction(SmallIcon("dialog-cancel"), i18n("C&ancel"));

            popMenu.setMouseTracking(true);
            QAction *choice = popMenu.exec(QCursor::pos());
            assignTag = choice == assignAction;
            setThumbnail = choice == thumbnailAction;
        }

        if (assignTag)
        {
            emit assignTags(destAlbum->id(), imageIDs);
        }
        else if(setThumbnail)
        {
            QString errMsg;
            AlbumManager::instance()->updateTAlbumIcon(destAlbum, QString(),
                                                       imageIDs.first(), errMsg);
        }
    }
}

void TagFilterView::slotAssignTags(int tagId, const QList<int>& imageIDs)
{
    TAlbum *destAlbum = AlbumManager::instance()->findTAlbum(tagId);
    if (!destAlbum)
        return;

    emit signalProgressBarMode(StatusProgressBar::ProgressBarMode,
                               i18n("Assigning image tags. Please wait..."));

    AlbumLister::instance()->blockSignals(true);
    ScanController::instance()->suspendCollectionScan();
    DatabaseTransaction transaction;
    MetadataHub         hub;
    int i=0;

    for (QList<int>::const_iterator it = imageIDs.constBegin(); it != imageIDs.constEnd(); ++it)
    {
                // create temporary ImageInfo object
        ImageInfo info(*it);

        hub.load(info);
        hub.setTag(destAlbum, true);

        QString filePath = info.filePath();
        hub.write(info, MetadataHub::PartialWrite);
        bool fileChanged = hub.write(filePath, MetadataHub::FullWriteIfChanged);
        if (fileChanged)
            ScanController::instance()->scanFileDirectly(filePath);

        emit signalProgressValue((int)((i++/(float)imageIDs.count())*100.0));
        kapp->processEvents();
    }
    ScanController::instance()->resumeCollectionScan();
    AlbumLister::instance()->blockSignals(false);

    emit signalProgressBarMode(StatusProgressBar::TextMode, QString());
}

void TagFilterView::slotTagAdded(Album* album)
{
    if (!album || album->isRoot())
        return;

    TAlbum* tag = dynamic_cast<TAlbum*>(album);
    if (!tag)
        return;

    if (tag->parent() && tag->parent()->isRoot())
    {
        new TagFilterViewItem(this, tag);
    }
    else
    {
        TagFilterViewItem* parent = 0;
        if (tag->parent())
            parent = static_cast<TagFilterViewItem*>(tag->parent()->extraData(this));

        if (!parent)
        {
            kWarning(50003) << " Failed to find parent for Tag "
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
        Q3ListViewItem* oldPItem = item->parent();
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

    // NOTE: see B.K.O #158558: unselected tag filter and all children before to delete it.
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
    for (AlbumList::const_iterator it = tList.constBegin(); it != tList.constEnd(); ++it)
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
    QList<int> filterTags;

    bool showUnTagged = false;

    Q3ListViewItemIterator it(this, Q3ListViewItemIterator::Checked);
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

void TagFilterView::slotContextMenu(Q3ListViewItem* it, const QPoint&, int)
{
    TagFilterViewItem *item = dynamic_cast<TagFilterViewItem*>(it);
    if (item && item->untagged())
        return;

    KMenu popmenu(this);
    popmenu.addTitle(SmallIcon("digikam"), i18n("Tag Filters"));

    QAction *newAction, *editAction=0, *resetIconAction=0, *deleteAction=0;

    newAction = popmenu.addAction(SmallIcon("tag-new"), i18n("New Tag..."));

#ifdef HAVE_KDEPIMLIBS
    d->ABCMenu = new KMenu(this);

    connect( d->ABCMenu, SIGNAL( aboutToShow() ),
             this, SLOT( slotABCContextMenu() ) );

    popmenu.addMenu(d->ABCMenu);
    d->ABCMenu->menuAction()->setIcon(SmallIcon("tag-addressbook"));
    d->ABCMenu->menuAction()->setText(i18n("Create Tag From Address Book"));
#endif // HAVE_KDEPIMLIBS

    if (item)
    {
        resetIconAction = popmenu.addAction(SmallIcon("view-refresh"), i18n("Reset Tag Icon"));
    }

    popmenu.addSeparator();

    KMenu selectTagsMenu;
    QAction *selectAllTagsAction, *selectChildrenAction=0, *selectParentsAction=0;
    selectAllTagsAction = selectTagsMenu.addAction(i18n("All Tags"));
    if (item)
    {
        selectTagsMenu.addSeparator();
        selectChildrenAction = selectTagsMenu.addAction(i18n("Children"));
        selectParentsAction  = selectTagsMenu.addAction(i18n("Parents"));
    }
    popmenu.addMenu(&selectTagsMenu);
    selectTagsMenu.menuAction()->setText(i18nc("select tags menu", "Select"));

    KMenu deselectTagsMenu;
    QAction *deselectAllTagsAction, *deselectChildrenAction=0, *deselectParentsAction=0;
    deselectAllTagsAction = deselectTagsMenu.addAction(i18n("All Tags"));
    if (item)
    {
        deselectTagsMenu.addSeparator();
        deselectChildrenAction = deselectTagsMenu.addAction(i18n("Children"));
        deselectParentsAction  = deselectTagsMenu.addAction(i18n("Parents"));
    }
    popmenu.addMenu(&deselectTagsMenu);
    deselectTagsMenu.menuAction()->setText(i18nc("deselect tags menu", "Deselect"));

    QAction *invertAction;
    invertAction = popmenu.addAction(i18n("Invert Selection"));
    popmenu.addSeparator();


    KSelectAction *toggleAutoAction = new KSelectAction(i18n("Toggle Auto"), &popmenu);
    QAction *toggleNoneAction     = toggleAutoAction->addAction(i18nc("no auto toggle", "None"));
    toggleAutoAction->menu()->addSeparator();
    QAction *toggleChildrenAction = toggleAutoAction->addAction(i18nc("toggle child tags", "Children"));
    QAction *toggleParentsAction  = toggleAutoAction->addAction(i18nc("toggle parent tag", "Parents"));
    QAction *toggleBothAction     = toggleAutoAction->addAction(i18nc("toggle child and parent tags", "Both"));

    toggleNoneAction->setChecked(d->toggleAutoTags == TagFilterView::NoToggleAuto);
    toggleChildrenAction->setChecked(d->toggleAutoTags == TagFilterView::Children);
    toggleParentsAction->setChecked(d->toggleAutoTags == TagFilterView::Parents);
    toggleBothAction->setChecked(d->toggleAutoTags == TagFilterView::ChildrenAndParents);

    popmenu.addAction(toggleAutoAction);

    KSelectAction *matchingCondAction = new KSelectAction(i18n("Matching Condition"), &popmenu);
    QAction *orBetweenAction = matchingCondAction->addAction(i18n("Or Between Tags"));
    QAction *andBetweenAction = matchingCondAction->addAction(i18n("And Between Tags"));

    if (d->matchingCond == AlbumLister::OrCondition)
        orBetweenAction->setChecked(true);
    else
        andBetweenAction->setChecked(true);
    popmenu.addAction(matchingCondAction);

    if (item)
    {
        popmenu.addSeparator();
        deleteAction    = popmenu.addAction(SmallIcon("user-trash"),     i18n("Delete Tag"));
        popmenu.addSeparator();
        editAction      = popmenu.addAction(SmallIcon("tag-properties"),
                                            i18nc("Edit Tag Properties", "Properties..."));
    }

    ToggleAutoTags oldAutoTags = d->toggleAutoTags;

    QAction *choice = popmenu.exec((QCursor::pos()));

    if (choice)
    {
        if (choice == newAction)                    // New Tag.
        {
            tagNew(item);
        }
        else if (choice == editAction)              // Edit Tag Properties.
        {
            tagEdit(item);
        }
        else if (choice == deleteAction)            // Delete Tag.
        {
            tagDelete(item);
        }
        else if (choice == resetIconAction)         // Reset Tag Icon.
        {
            QString errMsg;
            AlbumManager::instance()->updateTAlbumIcon(item->album(), QString("tag"), 0, errMsg);
        }
        else if (choice == selectAllTagsAction)     // Select All Tags.
        {
            d->toggleAutoTags = TagFilterView::NoToggleAuto;
            Q3ListViewItemIterator it(this, Q3ListViewItemIterator::NotChecked);
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
        }
        else if (choice == deselectAllTagsAction)    // Deselect All Tags.
        {
            d->toggleAutoTags = TagFilterView::NoToggleAuto;
            Q3ListViewItemIterator it(this, Q3ListViewItemIterator::Checked);
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
        }
        else if (choice == invertAction)             // Invert All Tags Selection.
        {
            d->toggleAutoTags = TagFilterView::NoToggleAuto;
            Q3ListViewItemIterator it(this);
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
        }
        else if (choice == selectChildrenAction)     // Select Child Tags.
        {
            d->toggleAutoTags = TagFilterView::NoToggleAuto;
            toggleChildTags(item, true);
            TagFilterViewItem *tItem = (TagFilterViewItem*)item->album()->extraData(this);
            tItem->setOn(true);
            d->toggleAutoTags = oldAutoTags;
        }
        else if (choice == deselectChildrenAction)   // Deselect Child Tags.
        {
            d->toggleAutoTags = TagFilterView::NoToggleAuto;
            toggleChildTags(item, false);
            TagFilterViewItem *tItem = (TagFilterViewItem*)item->album()->extraData(this);
            tItem->setOn(false);
            d->toggleAutoTags = oldAutoTags;
        }
        else if (choice == selectParentsAction)     // Select Parent Tags.
        {
            d->toggleAutoTags = TagFilterView::NoToggleAuto;
            toggleParentTags(item, true);
            TagFilterViewItem *tItem = (TagFilterViewItem*)item->album()->extraData(this);
            tItem->setOn(true);
            d->toggleAutoTags = oldAutoTags;
        }
        else if (choice == deselectParentsAction)   // Deselect Parent Tags.
        {
            d->toggleAutoTags = TagFilterView::NoToggleAuto;
            toggleParentTags(item, false);
            TagFilterViewItem *tItem = (TagFilterViewItem*)item->album()->extraData(this);
            tItem->setOn(false);
            d->toggleAutoTags = oldAutoTags;
        }
        else if (choice == toggleNoneAction)        // No toggle auto tags.
        {
            d->toggleAutoTags = NoToggleAuto;
        }
        else if (choice == toggleChildrenAction)    // Toggle auto Children tags.
        {
            d->toggleAutoTags = Children;
        }
        else if (choice == toggleParentsAction)     // Toggle auto Parents tags.
        {
            d->toggleAutoTags = Parents;
        }
        else if (choice == toggleBothAction)        // Toggle auto Children and Parents tags.
        {
            d->toggleAutoTags = ChildrenAndParents;
        }
        else if (choice == orBetweenAction)         // Or Between Tags.
        {
            d->matchingCond = AlbumLister::OrCondition;
            triggerChange();
        }
        else if (choice == andBetweenAction)        // And Between Tags.
        {
            d->matchingCond = AlbumLister::AndCondition;
            triggerChange();
        }
        else                                        // ABC menu
        {
            tagNew(item, choice->text(), "tag-people" );
        }
    }

    delete d->ABCMenu;
    d->ABCMenu = 0;
}

void TagFilterView::slotABCContextMenu()
{
#ifdef HAVE_KDEPIMLIBS
    d->ABCMenu->clear();

    KABC::AddressBook* ab = KABC::StdAddressBook::self();
    QStringList names;
    for ( KABC::AddressBook::Iterator it = ab->begin(); it != ab->end(); ++it )
    {
        names.push_back(it->formattedName());
    }
    qSort(names);

    for ( QStringList::ConstIterator it = names.constBegin(); it != names.constEnd(); ++it )
    {
        QString name = *it;
        if (!name.isNull() )
            d->ABCMenu->addAction(name);
    }

    if (d->ABCMenu->isEmpty())
    {
        QAction *nothingFound = d->ABCMenu->addAction(i18n("No address book entries found"));
        nothingFound->setEnabled(false);
    }
#endif // HAVE_KDEPIMLIBS
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

    for (AlbumList::const_iterator it = tList.constBegin(); it != tList.constEnd(); ++it)
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
            item->setText(0, title);
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
                     i18np("Tag '%2' has one subtag. "
                           "Deleting this will also delete "
                           "the subtag. "
                           "Do you want to continue?",
                           "Tag '%2' has %1 subtags. "
                           "Deleting this will also delete "
                           "the subtags. "
                           "Do you want to continue?",
                           children, tag->title()));

        if(result != KMessageBox::Continue)
            return;
    }

    QString message;
    QList<qlonglong> assignedItems = DatabaseAccess().db()->getItemIDsInTag(tag->id());
    if (!assignedItems.isEmpty())
    {
        message = i18np("Tag '%2' is assigned to one item. "
                        "Do you want to continue?",
                        "Tag '%2' is assigned to %1 items. "
                        "Do you want to continue?",
                        assignedItems.count(), tag->title());
    }
    else
    {
        message = i18n("Delete '%1' tag?", tag->title());
    }

    int result = KMessageBox::warningContinueCancel(0, message,
                                                    i18n("Delete Tag"),
                                                    KGuiItem(i18n("Delete"),
                                                    "edit-delete"));

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
            if (item->isVisible())
                item->setOn(b);
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

    Q3ListViewItemIterator it(this);
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
    Q3ListViewItemIterator it(this);

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
    Q3ListViewItemIterator it(this);

    while (it.current())
    {
        TagFilterViewItem* item = dynamic_cast<TagFilterViewItem*>(*it);
        if (item)
        {
            if (item->album())
            {
                int id = item->id();
                QMap<int, int>::const_iterator it2 = tagsStatMap.constFind(id);
                if ( it2 != tagsStatMap.constEnd() )
                    item->setCount(it2.value());
            }
        }
        ++it;
    }

    refresh();
}

void TagFilterView::slotResetTagFilters()
{
    Q3ListViewItemIterator it(this);

    while (it.current())
    {
        TagFilterViewItem* item = dynamic_cast<TagFilterViewItem*>(*it);
        if (item && item->isOn())
            item->setOn(false);
        ++it;
    }
}

void TagFilterView::loadViewState()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(objectName());

    int selectedItem = group.readEntry("LastSelectedItem", 0);

    QList<int> openFolders;
    if(group.hasKey("OpenFolders"))
    {
        openFolders = group.readEntry("OpenFolders",QList<int>());
    }

    TagFilterViewItem *item      = 0;
    TagFilterViewItem *foundItem = 0;
    Q3ListViewItemIterator it(this->lastItem());

    for( ; it.current(); --it)
    {
        item = dynamic_cast<TagFilterViewItem*>(it.current());
        if(!item)
            continue;
        // Start the album root always open
        if(openFolders.contains(item->id()) || item->id() == 0)
            setOpen(item, true);
        else
            setOpen(item, false);

        if(item->id() == selectedItem)
        {
            // Save the found selected item so that it can be made visible.
            foundItem = item;
        }
    }

    // Important note: this cannot be done inside the previous loop
    // because opening folders prevents the visibility.
    // Fixes bug #144815.
    // (Looks a bit like a bug in Qt to me ...)
    if (foundItem)
    {
        setSelected(foundItem, true);
        ensureItemVisible(foundItem);
    }
}

void TagFilterView::saveViewState()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(objectName());

    TagFilterViewItem *item = dynamic_cast<TagFilterViewItem*>(selectedItem());
    if (item)
        group.writeEntry("LastSelectedItem", item->id());
    else
        group.writeEntry("LastSelectedItem", 0);

    QList<int> openFolders;
    Q3ListViewItemIterator it(this);
    for( ; it.current(); ++it)
    {
        item = dynamic_cast<TagFilterViewItem*>(it.current());
        if(item && isOpen(item))
            openFolders.push_back(item->id());
    }
    group.writeEntry("OpenFolders", openFolders);
}

}  // namespace Digikam
