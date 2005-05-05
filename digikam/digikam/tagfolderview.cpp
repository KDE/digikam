/* ============================================================
 * File  : tagfolderview.cpp
 * Author: Jörn Ahrens <joern.ahrens@kdemail.net>
 * Date  : 2005-05-05
 * Copyright 2005 by Jörn Ahrens
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
 * ============================================================ */

#include <qintdict.h>
#include <qpainter.h>

#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kapplication.h>

#include "tagfolderview.h"
#include "album.h"
#include "albummanager.h"
#include "syncjob.h"

static QPixmap getBlendedIcon(TAlbum* tag)
{
    KIconLoader *iconLoader = KApplication::kApplication()->iconLoader();

    QPixmap baseIcon(iconLoader->loadIcon("tag",
                     KIcon::NoGroup,
                     32,
                     KIcon::DefaultState,
                     0, true));

    if(!tag)
        return baseIcon;

    QString icon(tag->getIcon());

    QPixmap pix = SyncJob::getTagThumbnail(tag->getIcon(), 20);

    if (!pix.isNull())
    {
        QPainter p(&baseIcon);
        p.drawPixmap(6, 9, pix, 0, 0, -1, -1);
        p.end();
    }

    return baseIcon;
}

//-----------------------------------------------------------------------------
// TagFolderViewItem
//-----------------------------------------------------------------------------

class TagFolderViewItem : public QListViewItem
{
public:
    TagFolderViewItem(QListView *parent, TAlbum *tag);
    TagFolderViewItem(QListViewItem *parent, TAlbum *tag);    

    TAlbum      *m_tag;
};

TagFolderViewItem::TagFolderViewItem(QListView *parent, TAlbum *tag)
    : QListViewItem(parent, tag->getTitle())
{
    m_tag = tag;
}

TagFolderViewItem::TagFolderViewItem(QListViewItem *parent, TAlbum *tag)
    : QListViewItem(parent, tag->getTitle())
{
    m_tag = tag;
}

//-----------------------------------------------------------------------------
// TagFolderViewPriv
//-----------------------------------------------------------------------------

class TagFolderViewPriv
{
public:
    QIntDict<TagFolderViewItem>    dict;
};

//-----------------------------------------------------------------------------
// TagFolderView
//-----------------------------------------------------------------------------

TagFolderView::TagFolderView(QWidget *parent)
    : QListView(parent)
{
    d = new TagFolderViewPriv();
    
    addColumn(i18n("My Tags"));
    setResizeMode(QListView::LastColumn);
    setRootIsDecorated(true);
    
    connect(AlbumManager::instance(), SIGNAL(signalAlbumAdded(Album*)),
            SLOT(slotAlbumAdded(Album*)));
}

void TagFolderView::slotAlbumAdded(Album *album)
{
    if(!album || album->isRoot())
        return;
    
    TAlbum *tag = dynamic_cast<TAlbum*>(album);
    if(!tag)
        return;
    
    if(tag->getParent()->isRoot())
    {
        TagFolderViewItem *item = new TagFolderViewItem(this, tag);
        item->setPixmap(0, getBlendedIcon(tag));
        d->dict.insert(tag->getID(), item);
    }
    else
    {
        TagFolderViewItem *parent = d->dict.find(tag->getParent()->getID());
        if (!parent)
        {
            kdWarning() << k_funcinfo << " Failed to find parent for Tag "
                        << tag->getURL() << endl;
            return;
        }
        TagFolderViewItem *item = new TagFolderViewItem(parent, tag);
        item->setPixmap(0, getBlendedIcon(tag));
        d->dict.insert(tag->getID(), item);        
    }
}

#include "tagfolderview.moc"
