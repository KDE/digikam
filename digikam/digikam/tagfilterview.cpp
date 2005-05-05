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

#include <qheader.h>
#include <qintdict.h>
#include <qpixmap.h>
#include <qpainter.h>
#include <qtimer.h>

#include "albummanager.h"
#include "albumlister.h"
#include "album.h"
#include "syncjob.h"
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

    QString icon(album->getIcon());

    QPixmap pix = SyncJob::getTagThumbnail(album->getIcon(), 20);

    if (!pix.isNull())
    {
        QPainter p(&baseIcon);
        p.drawPixmap(6, 9, pix, 0, 0, -1, -1);
        p.end();
    }

    return baseIcon;
}

class TagFilterViewItem : public QCheckListItem
{
public:

    TagFilterViewItem(QListView* parent, TAlbum* tag)
        : QCheckListItem(parent, tag->getTitle(), QCheckListItem::CheckBoxController)
    {
        m_tag = tag;
    }

    TagFilterViewItem(QListViewItem* parent, TAlbum* tag)
        : QCheckListItem(parent, tag->getTitle(), QCheckListItem::CheckBoxController)
    {
        m_tag = tag;
    }

    virtual void stateChange(bool val)
    {
        QCheckListItem::stateChange(val);

        ((TagFilterView*)listView())->triggerChange();
    }

    TAlbum* m_tag;
};

class TagFilterViewPriv
{
public:

    QIntDict<TagFilterViewItem> dict;
    QTimer*                     timer;
};

TagFilterView::TagFilterView(QWidget* parent)
    : QListView(parent)
{
    d = new TagFilterViewPriv;
    d->timer = new QTimer(this);
    
    addColumn(i18n("My Tags"));
    setResizeMode(QListView::LastColumn);
    setRootIsDecorated(true);
    header()->hide();
    
    connect(AlbumManager::instance(), SIGNAL(signalAlbumAdded(Album*)),
            SLOT(slotTagAdded(Album*)));
    connect(AlbumManager::instance(), SIGNAL(signalAlbumsCleared()),
            SLOT(slotClear()));

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

void TagFilterView::slotTagAdded(Album* album)
{
    if (!album || album->isRoot())
        return;

    TAlbum* tag = dynamic_cast<TAlbum*>(album);
    if (!tag)
        return;

    if (tag->getParent()->isRoot())
    {
        TagFilterViewItem* item = new TagFilterViewItem(this, tag);
        item->setPixmap(0, getBlendedIcon(tag));
        d->dict.insert(tag->getID(), item);
    }
    else
    {
        TagFilterViewItem* parent = d->dict.find(tag->getParent()->getID());
        if (!parent)
        {
            kdWarning() << k_funcinfo << " Failed to find parent for Tag "
                        << tag->getURL() << endl;
            return;
        }

        TagFilterViewItem* item = new TagFilterViewItem(parent, tag);
        item->setPixmap(0, getBlendedIcon(tag));
        d->dict.insert(tag->getID(), item);
    }
}

void TagFilterView::slotClear()
{
    clear();
    d->dict.clear();
}

void TagFilterView::slotTimeOut()
{
    QValueList<int> filterTags;
    
    QListViewItemIterator it(this, QListViewItemIterator::Checked);
    while (it.current())
    {
        TagFilterViewItem* item = (TagFilterViewItem*)it.current();
        if (!item->firstChild())
            filterTags.append(item->m_tag->getID());
        ++it;
    }

    AlbumLister::instance()->setTagFilter(filterTags);
}

#include "tagfilterview.moc"
