/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Caulier Gilles <caulier dot gilles at kdemail dot net>
 * Date   : 2005-05-05
 * Description : tags filter view
 *
 * Copyright 2005 by Renchi Raju
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

// Qt includes.

#include <qheader.h>
#include <qpixmap.h>
#include <qpainter.h>
#include <qtimer.h>
#include <qcursor.h>

// KDE includes.

#include <kpopupmenu.h>
#include <klocale.h>
#include <kdebug.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kiconloader.h>
#include <kglobalsettings.h>
#include <kcursor.h>
#include <kmessagebox.h>

// X11 includes.

extern "C"
{
#include <X11/Xlib.h>
}

// Local includes.

#include "albummanager.h"
#include "albumlister.h"
#include "albumdb.h"
#include "album.h"
#include "syncjob.h"
#include "dragobjects.h"
#include "folderitem.h"
#include "imageattributeswatch.h"
#include "albumthumbnailloader.h"
#include "tagcreatedlg.h"
#include "tagfilterview.h"
#include "tagfilterview.moc"

namespace Digikam
{

class TagFilterViewItem : public FolderCheckListItem
{

public:

    TagFilterViewItem(QListView* parent, TAlbum* tag, bool untagged=false)
        : FolderCheckListItem(parent, tag ? tag->title() : i18n("Not Tagged"),
                              QCheckListItem::CheckBoxController)
    {
        m_tag      = tag;
        m_untagged = untagged;
        setDragEnabled(!untagged);

        if (tag)
            tag->setExtraData(listView(), this);
    }

    TagFilterViewItem(QListViewItem* parent, TAlbum* tag)
        : FolderCheckListItem(parent, tag->title(), QCheckListItem::CheckBoxController)
    {
        m_tag      = tag;
        m_untagged = false;
        setDragEnabled(true);

        if (tag)
            tag->setExtraData(listView(), this);
    }

    virtual void stateChange(bool val)
    {
        QCheckListItem::stateChange(val);

        // All TagFilterViewItems are CheckBoxControllers. If they have no children,
        // they should be of type CheckBox, but that is not possible with our way of adding items.
        // When clicked, children-less items first change to the NoChange state, and a second
        // click is necessary to set them to On and make the filter take effect.
        // So set them to On if the condition is met.
        if (!firstChild() && state() == NoChange)
        {
            setState(On);
        }

        ((TagFilterView*)listView())->triggerChange();
    }

    int compare(QListViewItem* i, int column, bool ascending) const
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

    void paintCell(QPainter* p, const QColorGroup & cg, int column, int width, int align)
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

    TAlbum *m_tag;
    bool    m_untagged;
};

// ---------------------------------------------------------------------

class TagFilterViewPriv
{

public:

    TagFilterViewPriv()
    {
        timer        = 0;
        matchingCond = AlbumLister::OrCondition;
    }

    QTimer                         *timer;
 
    AlbumLister::MatchingCondition  matchingCond;
};

TagFilterView::TagFilterView(QWidget* parent)
             : FolderView(parent)
{
    d = new TagFilterViewPriv;
    d->timer = new QTimer(this);

    addColumn(i18n("Tag Filters"));
    setResizeMode(QListView::LastColumn);
    setRootIsDecorated(true);
    setSelectionMode(QListView::Extended);

    setAcceptDrops(true);
    viewport()->setAcceptDrops(true);

    TagFilterViewItem* notTaggedItem = new TagFilterViewItem(this, 0, true);
    notTaggedItem->setPixmap(0, AlbumThumbnailLoader::instance()->getStandardTagIcon());

    // -- setup slots ---------------------------------------------------------

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

    AlbumThumbnailLoader *loader = AlbumThumbnailLoader::instance();
    
    connect(loader, SIGNAL(signalThumbnail(Album *, const QPixmap&)),
            this, SLOT(slotGotThumbnailFromIcon(Album *, const QPixmap&)));
            
    connect(loader, SIGNAL(signalFailed(Album *)),
            this, SLOT(slotThumbnailLost(Album *)));

    connect(this, SIGNAL(contextMenuRequested(QListViewItem*, const QPoint&, int)),
            this, SLOT(slotContextMenu(QListViewItem*, const QPoint&, int)));

    connect(d->timer, SIGNAL(timeout()),
            this, SLOT(slotTimeOut()));

    // -- read config ---------------------------------------------------------

    KConfig* config = kapp->config();
    config->setGroup("Tag Filters View");
    d->matchingCond = (AlbumLister::MatchingCondition)(config->readNumEntry("Matching Condition", 
                                                       AlbumLister::OrCondition));
}

TagFilterView::~TagFilterView()
{
    KConfig* config = kapp->config();
    config->setGroup("Tag Filters View");
    config->writeEntry("Matching Condition", (int)(d->matchingCond));
    config->sync();

    delete d->timer;
    delete d;
}

void TagFilterView::triggerChange()
{
    d->timer->start(50, true);
}

QDragObject* TagFilterView::dragObject()
{
    QValueList<int> dragTagIDs;

    QListViewItemIterator it(this, QListViewItemIterator::Selected);
    while (it.current())
    {
        TagFilterViewItem* item = (TagFilterViewItem*)it.current();
        if (item->m_tag)
        {
            dragTagIDs.append(item->m_tag->id());
        }
        ++it;
    }

    TagListDrag *drag = new TagListDrag(dragTagIDs, this);
    drag->setPixmap(AlbumThumbnailLoader::instance()->getStandardTagIcon());
    return drag;
}

bool TagFilterView::acceptDrop(const QDropEvent *e) const
{
    QPoint vp = contentsToViewport(e->pos());
    TagFilterViewItem *itemDrop = dynamic_cast<TagFilterViewItem*>(itemAt(vp));

    if (!itemDrop || itemDrop->m_untagged)
    {
        return false;
    }

    if (ItemDrag::canDecode(e))
    {
        return true;
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

    if (!itemDrop || itemDrop->m_untagged)
    {
        return;
    }

    if (ItemDrag::canDecode(e))
    {
        TAlbum *destAlbum = itemDrop->m_tag;

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
            popMenu.insertItem(SmallIcon("tag"), i18n("Assign Tag '%1' to Dropped Items")
                                .arg(destAlbum->prettyURL()), 10) ;
            popMenu.insertItem(i18n("Set as Tag Thumbnail"),  11);
            popMenu.insertSeparator(-1);
            popMenu.insertItem(SmallIcon("cancel"), i18n("C&ancel"));

            popMenu.setMouseTracking(true);
            id = popMenu.exec(QCursor::pos());
        }

        if (id == 10)
        {
            AlbumDB* db = AlbumManager::instance()->albumDB();

            db->beginTransaction();
            for (QValueList<int>::const_iterator it = imageIDs.begin();
                 it != imageIDs.end(); ++it)
            {
                db->addItemTag(*it, destAlbum->id());
            }
            db->commitTransaction();

            ImageAttributesWatch::instance()->imagesChanged(destAlbum->id());
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
            kdWarning() << k_funcinfo << " Failed to find parent for Tag "
                        << tag->url() << endl;
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
    {
        item->setText(0, tag->title());
    }
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
        if (item->m_tag)
            filterTags.append(item->m_tag->id());
        else if (item->m_untagged)
            showUnTagged = true;
        ++it;
    }

    AlbumLister::instance()->setTagFilter(filterTags, d->matchingCond, showUnTagged);
}

void TagFilterView::slotContextMenu(QListViewItem* it, const QPoint&, int)
{
    KPopupMenu popmenu(this);
    popmenu.insertTitle(SmallIcon("digikam"), i18n("Tag Filters"));

    TagFilterViewItem *item = dynamic_cast<TagFilterViewItem*>(it);
    if (item && item->m_untagged)
        return;

    popmenu.insertItem(SmallIcon("tag"), i18n("New Tag..."), 10);

    if (item)
    {
        popmenu.insertItem(SmallIcon("pencil"),      i18n("Edit Tag Properties..."), 11);
        popmenu.insertItem(SmallIcon("reload_page"), i18n("Reset Tag Icon"),         13);
        popmenu.insertSeparator(-1);
        popmenu.insertItem(SmallIcon("edittrash"),   i18n("Delete Tag"),             12);
    }
 
    popmenu.insertSeparator(-1);
    popmenu.insertItem(i18n("Select All"),       14);
    popmenu.insertItem(i18n("Deselect"),         15);
    popmenu.insertItem(i18n("Invert Selection"), 16);
    popmenu.insertSeparator(-1);

    QPopupMenu matchingCongMenu;
    matchingCongMenu.setCheckable(true);
    matchingCongMenu.insertItem(i18n("Or Between Tags"),  17);
    matchingCongMenu.insertItem(i18n("And Between Tags"), 18);
    matchingCongMenu.setItemChecked((d->matchingCond == AlbumLister::OrCondition) ? 17 : 18, true);
    popmenu.insertItem(i18n("Matching Condition"), &matchingCongMenu);

    int choice = popmenu.exec((QCursor::pos()));
    switch( choice )
    {
        case 10:
        {
            tagNew(item);
            break;
        }
        case 11:
        {
            tagEdit(item);
            break;
        }
        case 12:
        {
            tagDelete(item);
            break;
        }
        case 13:
        {
            QString errMsg;
            AlbumManager::instance()->updateTAlbumIcon(item->m_tag, QString("tag"), 0, errMsg);
            break;
        }        
        case 14:
        {
            QListViewItemIterator it(this, QListViewItemIterator::NotChecked);
            while (it.current())
            {
                TagFilterViewItem* item = (TagFilterViewItem*)it.current();
                item->setOn(true);
                ++it;
            }
            triggerChange();
            break;
        }
        case 15:
        {
            QListViewItemIterator it(this, QListViewItemIterator::Checked);
            while (it.current())
            {
                TagFilterViewItem* item = (TagFilterViewItem*)it.current();
                item->setOn(false);
                ++it;
            }
            triggerChange();
            break;
        }
        case 16:
        {
            QListViewItemIterator it(this);
            while (it.current())
            {
                TagFilterViewItem* item = (TagFilterViewItem*)it.current();

                // Toggle all root tags filter.
                TAlbum *tag = item->m_tag;
                if (tag)
                    if (tag->parent()->isRoot())
                        item->setOn(!item->isOn());

                // Toggle "Not Tagged" item tag filter.
                if (item->m_untagged)
                    item->setOn(!item->isOn());

                ++it;
            }
            triggerChange();
            break;
        }
        case 17:
        {
            d->matchingCond = AlbumLister::OrCondition;
            triggerChange();
            break;
        }
        case 18:
        {
            d->matchingCond = AlbumLister::AndCondition;
            triggerChange();
            break;
        }
        default:
            break;
    }
}

void TagFilterView::tagNew(TagFilterViewItem* item)
{
    TAlbum *parent;
    AlbumManager* man = AlbumManager::instance();

    if (!item)
        parent = man->findTAlbum(0);
    else
        parent = item->m_tag;

    QString title;
    QString icon;
    if (!TagCreateDlg::tagCreate(kapp->activeWindow(), parent, title, icon))
        return;

    QString errMsg;
    TAlbum* newAlbum = man->createTAlbum(parent, title, icon, errMsg);

    if( !newAlbum )
    {
        KMessageBox::error(0, errMsg);
    }
    else
    {
        TagFilterViewItem *item = (TagFilterViewItem*)newAlbum->extraData(this);
        if ( item )
        {
            clearSelection();
            setSelected(item, true);
            setCurrentItem(item);
            ensureItemVisible( item );
        }
    }
}

void TagFilterView::tagEdit(TagFilterViewItem* item)
{
    if (!item)
        return;

    TAlbum *tag = item->m_tag;
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

    TAlbum *tag = item->m_tag;
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
                          "Are you sure you want to continue?",
                          "Tag '%1' has %n subtags. "
                          "Deleting this will also delete "
                          "the subtags. "
                          "Are you sure you want to continue?",
                          children).arg(tag->title()),
                          i18n("Delete Tag"),
                          KGuiItem(i18n("Delete"),"editdelete"));

        if(result == KMessageBox::Continue)
        {
            QString errMsg;
            if (!man->deleteTAlbum(tag, errMsg))
                KMessageBox::error(0, errMsg);
        }
    }
    else
    {
        int result = KMessageBox::warningContinueCancel(0, i18n("Delete '%1' tag?")
                                                        .arg(tag->title()),i18n("Delete Tag"),
                                                        KGuiItem(i18n("Delete"), "editdelete"));

        if (result == KMessageBox::Continue)
        {
            QString errMsg;
            if (!man->deleteTAlbum(tag, errMsg))
                KMessageBox::error(0, errMsg);
        }
    }
}

}  // namespace Digikam

