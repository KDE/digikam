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
#include <klocale.h>
#include <kmessagebox.h>
#include <kdebug.h>

#include <qpixmap.h>
#include <qstring.h>

#include "albumiconview.h"
#include "albumiconitem.h"
#include "albummanager.h"
#include "albumdb.h"
#include "album.h"
#include "syncjob.h"
#include "tagcreatedlg.h"

#include "tagspopupmenu.h"

#define ADDTAGID 10000

TagsPopupMenu::TagsPopupMenu(const QValueList<Q_LLONG>& selectedImageIDs,
                             int addToID,
                             Mode mode)
    : QPopupMenu(0),
      m_selectedImageIDs(selectedImageIDs),
      m_addToID(addToID),
      m_mode(mode)
{
    KIconLoader *iconLoader = KApplication::kApplication()->iconLoader();
    m_addTagPix =  iconLoader->loadIcon("tag",
                                        KIcon::NoGroup,
                                        KIcon::SizeSmall,
                                        KIcon::DefaultState,
                                        0, true);
    
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

    if (m_mode == ASSIGN)
    {
        popup->insertItem(m_addTagPix, i18n("Add new Tag..."),
                          ADDTAGID + album->id());
        if (album->firstChild())
        {
            popup->insertSeparator();
        }
    }
    else
    {
        if (!album->isRoot())
        {
            QPixmap pix = SyncJob::getTagThumbnail(album->icon(), KIcon::SizeSmall);
            popup->insertItem(pix, album->title(), m_addToID + album->id());
            popup->insertSeparator();
        }
    }
    
    for (Album* a = album->firstChild(); a; a = a->next())
    {
        if (m_mode == REMOVE)
        {
            IntList::iterator it = qFind(m_assignedTags.begin(),
                                         m_assignedTags.end(),
                                         a->id());
            if (it == m_assignedTags.end())
                continue;
        }

        QPixmap pix = SyncJob::getTagThumbnail(((TAlbum*)a)->icon(), KIcon::SizeSmall);
        if (a->firstChild())
        {
            popup->insertItem(pix, a->title(), buildSubMenu(a->id()));
        }
        else
        {
            popup->insertItem(pix, a->title(), m_addToID + a->id());
        }
    }

    return popup;
}

void TagsPopupMenu::slotAboutToShow()
{
    clearPopup();

    AlbumManager* man = AlbumManager::instance();

    if (m_mode == REMOVE)
    {
        if (m_selectedImageIDs.isEmpty())
            return;

        m_assignedTags = man->albumDB()->getItemCommonTagIDs(m_selectedImageIDs);

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
                Album* a = album->parent();
                while (a)
                {
                    tList.append(a->id());
                    a = a->parent();
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

    if (m_mode == ASSIGN)
    {
        insertItem(m_addTagPix, i18n("Add new Tag..."), ADDTAGID);
        if (album->firstChild())
        {
            insertSeparator();
        }
    }
    
    for (Album* a = album->firstChild(); a; a = a->next())
    {
        if (m_mode == REMOVE)
        {
            IntList::iterator it = qFind(m_assignedTags.begin(),
                                         m_assignedTags.end(),
                                         a->id());
            if (it == m_assignedTags.end())
                continue;
        }
        
        QPixmap pix = SyncJob::getTagThumbnail(((TAlbum*)a)->icon(), KIcon::SizeSmall);
        if (a->firstChild())
        {
            insertItem(pix, a->title(), buildSubMenu(a->id()));
        }
        else
        {
            insertItem(pix, a->title(), m_addToID + a->id());
        }
    }
}

void TagsPopupMenu::slotActivated(int id)
{
    if (id >= ADDTAGID)
    {
        int tagID = id - ADDTAGID;

        AlbumManager* man = AlbumManager::instance();
        TAlbum* parent = man->findTAlbum(tagID);
        if (!parent)
        {
            kdWarning() << "Failed to find album with id "
                        << tagID << endl;
            return;
        }

        QString title, icon;
        if (!TagCreateDlg::tagCreate(parent, title, icon))
            return;

        QString errMsg;
        TAlbum* newAlbum = man->createTAlbum(parent, title, icon, errMsg);

        if( !newAlbum )
        {
            KMessageBox::error(this, errMsg);
            return;
        }

        emit signalTagActivated(newAlbum->id());
    }
    else
    {
        int tagID = id - m_addToID;
        emit signalTagActivated(tagID);
    }
}

#include "tagspopupmenu.moc"
