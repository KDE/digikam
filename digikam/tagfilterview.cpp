/* ============================================================
 * File  : tagfilterview.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2005-05-05
 * Description : 
 * 
 * Copyright 2005 by Renchi Raju

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

#include <klocale.h>
#include <kdebug.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <kglobalsettings.h>
#include <kcursor.h>
#include <kmessagebox.h>

#include <qheader.h>
#include <qintdict.h>
#include <qpixmap.h>
#include <qpainter.h>
#include <qtimer.h>
#include <qpopupmenu.h>
#include <qcursor.h>

#include "albummanager.h"
#include "albumlister.h"
#include "album.h"
#include "syncjob.h"
#include "dragobjects.h"
#include "folderitem.h"
#include "tagcreatedlg.h"
#include "tagfilterview.h"

static QPixmap getBlendedIcon(TAlbum* album)
{
    KIconLoader *iconLoader = KApplication::kApplication()->iconLoader();

    QPixmap baseIcon(iconLoader->loadIcon("tag",
                                          KIcon::NoGroup,
                                          32,
                                          KIcon::DefaultState,
                                          0, true));

    if(!album)
        return baseIcon;

    QPixmap pix = SyncJob::getTagThumbnail(album->icon(), 20);

    if (!pix.isNull())
    {
        QPainter p(&baseIcon);
        p.drawPixmap(6, 9, pix, 0, 0, -1, -1);
        p.end();
    }

    return baseIcon;
}

class TagFilterViewItem : public FolderCheckListItem
{
public:

    TagFilterViewItem(QListView* parent, TAlbum* tag, bool untagged=false)
        : FolderCheckListItem(parent, tag ? tag->title() : i18n("Not Tagged"),
                              QCheckListItem::CheckBoxController)
    {
        m_tag = tag;
        m_untagged = untagged;
        setDragEnabled(!untagged);
    }

    TagFilterViewItem(QListViewItem* parent, TAlbum* tag)
        : FolderCheckListItem(parent, tag->title(), QCheckListItem::CheckBoxController)
    {
        m_tag = tag;
        m_untagged = false;
        setDragEnabled(true);
    }

    virtual void stateChange(bool val)
    {
        QCheckListItem::stateChange(val);

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
    
    TAlbum* m_tag;
    bool    m_untagged;
};

class TagFilterViewPriv
{
public:

    QIntDict<TagFilterViewItem> dict;
    QTimer*                     timer;
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

    TagFilterViewItem* notTaggedItem = new TagFilterViewItem(this, 0, true);
    notTaggedItem->setPixmap(0, getBlendedIcon(0));
    
    connect(AlbumManager::instance(), SIGNAL(signalAlbumAdded(Album*)),
            SLOT(slotTagAdded(Album*)));
    connect(AlbumManager::instance(), SIGNAL(signalAlbumDeleted(Album*)),
            SLOT(slotTagDeleted(Album*)));
    connect(AlbumManager::instance(), SIGNAL(signalTAlbumMoved(TAlbum*, TAlbum*)),
            SLOT(slotTagMoved(TAlbum*, TAlbum*)));
    connect(AlbumManager::instance(), SIGNAL(signalAlbumsCleared()),
            SLOT(slotClear()));

    connect(this, SIGNAL(contextMenuRequested(QListViewItem*, const QPoint&, int)),
            SLOT(slotContextMenu(QListViewItem*, const QPoint&, int)));
    
    connect(d->timer, SIGNAL(timeout()),
            SLOT(slotTimeOut()));
}

TagFilterView::~TagFilterView()
{
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

    KIconLoader *iconLoader = KApplication::kApplication()->iconLoader();
    QPixmap icon(iconLoader->loadIcon("tag", KIcon::NoGroup,
                                      32,
                                      KIcon::DefaultState,
                                      0, true));
    
    TagListDrag *drag = new TagListDrag(dragTagIDs, this);
    drag->setPixmap(icon);
    return drag;
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
        TagFilterViewItem* item = new TagFilterViewItem(this, tag);
        item->setPixmap(0, getBlendedIcon(tag));
        d->dict.insert(tag->id(), item);
    }
    else
    {
        TagFilterViewItem* parent = d->dict.find(tag->parent()->id());
        if (!parent)
        {
            kdWarning() << k_funcinfo << " Failed to find parent for Tag "
                        << tag->url() << endl;
            return;
        }

        TagFilterViewItem* item = new TagFilterViewItem(parent, tag);
        item->setPixmap(0, getBlendedIcon(tag));
        d->dict.insert(tag->id(), item);
    }
}

void TagFilterView::slotTagMoved(TAlbum* tag, TAlbum* newParent)
{
    if (!tag || !newParent)
        return;

    TagFilterViewItem* item = d->dict.find(tag->id());
    if (!item)
        return;

    if (item->parent())
    {
        QListViewItem* oldPItem = item->parent();
        oldPItem->takeItem(item);
        
        QListViewItem* newPItem = d->dict.find(newParent->id());
        if (newPItem)
            newPItem->insertItem(item);
        else
            insertItem(item);
    }
    else
    {
        takeItem(item);

        QListViewItem* newPItem = d->dict.find(newParent->id());
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

    TagFilterViewItem* item = d->dict.find(tag->id());
    if (!item)
        return;

    d->dict.remove(tag->id());
    delete item;
}

void TagFilterView::slotClear()
{
    clear();
    d->dict.clear();
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

    AlbumLister::instance()->setTagFilter(filterTags, showUnTagged);
}

void TagFilterView::slotContextMenu(QListViewItem* it, const QPoint&, int)
{
    QPopupMenu popmenu(this);

    TagFilterViewItem *item = dynamic_cast<TagFilterViewItem*>(it);
    if (item && item->m_untagged)
        return;
    
    popmenu.insertItem(SmallIcon("tag"), i18n("New Tag..."), 10);

    if (item)
    {
        popmenu.insertItem(SmallIcon("pencil"), i18n("Edit Tag Properties..."), 11);
        popmenu.insertItem(SmallIcon("edittrash"), i18n("Delete Tag"), 12);
    }

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
    if (!TagCreateDlg::tagCreate(parent, title, icon))
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
            setSelected(item, true);
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
    if (!TagEditDlg::tagEdit(tag, title, icon))
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
            item->setPixmap(0, getBlendedIcon(tag));
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
        int result =
            KMessageBox::warningYesNo(this, i18n("Tag '%1' has %2 subtag(s). "
                                                 "Deleting this will also delete "
                                                 "the subtag(s). "
                                                 "Are you sure you want to continue?")
                                      .arg(tag->title())
                                      .arg(children));

        if(result == KMessageBox::Yes)
        {
            QString errMsg;
            if (!man->deleteTAlbum(tag, errMsg))
                KMessageBox::error(0, errMsg);
        }
    }
    else
    {
        int result =
            KMessageBox::questionYesNo(0, i18n("Delete '%1' tag?")
                                       .arg(tag->title()));

        if (result == KMessageBox::Yes)
        {
            QString errMsg;
            if (!man->deleteTAlbum(tag, errMsg))
                KMessageBox::error(0, errMsg);
        }
    }
}

#include "tagfilterview.moc"
