/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-09-07
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju

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

#include <kapplication.h>
#include <kiconloader.h>

#include <qpixmap.h>
#include <qstring.h>

#include "albumiconview.h"
#include "albumiconitem.h"
#include "albumlister.h"
#include "albummanager.h"
#include "albumdb.h"
#include "album.h"
#include "syncjob.h"

#include "tagspopupmenu.h"

TagsPopupMenu::TagsPopupMenu(AlbumIconView* view, int addToID,
                             bool onlyAssignedTags)
    : QPopupMenu(0),
      m_view(view), m_addToID(addToID),
      m_onlyAssignedTags(onlyAssignedTags)
{
    connect(this, SIGNAL(aboutToShow()),
            SLOT(slotAboutToShow()));
    connect(this, SIGNAL(activated(int)),
            SLOT(slotActivated(int)));
}

TagsPopupMenu::~TagsPopupMenu()
{
}

void TagsPopupMenu::clearPopup()
{
    m_assignedTags.clear();
    clear();
}

QPopupMenu* TagsPopupMenu::buildSubMenu(int tagid)
{
    AlbumManager* man = AlbumManager::instance();
    TAlbum* album = man->findTAlbum(tagid);
    if (!album)
        return 0;

    QPopupMenu*  popup      = new QPopupMenu(this);
    connect(popup, SIGNAL(activated(int)), SLOT(slotActivated(int)));

    if (!album->isRoot())
    {
        QPixmap pix = SyncJob::getTagThumbnail(album->getIcon(), KIcon::SizeSmall);
        popup->insertItem(pix, album->getTitle(), m_addToID + album->getID());
        popup->insertSeparator();
    }

    for (Album* a = album->firstChild(); a; a = a->next())
    {
        if (m_onlyAssignedTags)
        {
            IntList::iterator it = qFind(m_assignedTags.begin(),
                                         m_assignedTags.end(),
                                         a->getID());
            if (it == m_assignedTags.end())
                continue;
        }

        QPixmap pix = SyncJob::getTagThumbnail(a->getIcon(), KIcon::SizeSmall);
        if (a->firstChild())
        {
            popup->insertItem(pix, a->getTitle(), buildSubMenu(a->getID()));
        }
        else
        {
            popup->insertItem(pix, a->getTitle(), m_addToID + a->getID());
        }
    }

    return popup;
}

void TagsPopupMenu::slotAboutToShow()
{
    clearPopup();
    
    AlbumManager* man = AlbumManager::instance();

    if (m_onlyAssignedTags)
    {
        AlbumLister* lister = m_view->albumLister();
    
        QStringList nameList;
        IntList     dirIDList;
        for (ThumbItem *it = m_view->firstItem(); it; it = it->nextItem())
        {
            if (it->isSelected())
            {
                AlbumIconItem* item = static_cast<AlbumIconItem*>(it);
                PAlbum *album = lister->findParentAlbum(item->fileItem());
                if (album)
                {
                    nameList .append(item->fileItem()->name());
                    dirIDList.append(album->getID());
                }
            }
        }

        if (nameList.isEmpty())
            return;

        m_assignedTags = man->albumDB()->getItemCommonTagIDs(dirIDList,
                                                             nameList);
        if (m_assignedTags.isEmpty())
            return;

        // also add the parents of the assigned tags
        IntList tList;
        for (IntList::iterator it = m_assignedTags.begin();
             it != m_assignedTags.end(); ++it)
        {
            TAlbum* album = man->findTAlbum(*it);
            if (album)
            {
                Album* a = album->getParent();
                while (a)
                {
                    tList.append(a->getID());
                    a = a->getParent();
                }
            }
        }

        for (IntList::iterator it = tList.begin();
             it != tList.end(); ++it)
        {
            m_assignedTags.append(*it);
        }
    }
        
    TAlbum* album = man->findTAlbum(0);
    if (!album)
        return;

    for (Album* a = album->firstChild(); a; a = a->next())
    {
        if (m_onlyAssignedTags)
        {
            IntList::iterator it = qFind(m_assignedTags.begin(),
                                         m_assignedTags.end(),
                                         a->getID());
            if (it == m_assignedTags.end())
                continue;
        }
        
        QPixmap pix = SyncJob::getTagThumbnail(a->getIcon(), KIcon::SizeSmall);
        if (a->firstChild())
        {
            insertItem(pix, a->getTitle(), buildSubMenu(a->getID()));
        }
        else
        {
            insertItem(pix, a->getTitle(), m_addToID + a->getID());
        }
    }
}

void TagsPopupMenu::slotActivated(int id)
{
    int tagID = id - m_addToID;
    emit signalTagActivated(tagID);
}

#include "tagspopupmenu.moc"
