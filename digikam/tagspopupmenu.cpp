/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-09-07
 * Description : a pop-up menu implementation to display a 
 *               hierarchical view of digiKam tags.
 * 
 * Copyright (C) 2004 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
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

#define ADDTAGID 10000

// Qt includes.

#include <qpixmap.h>
#include <qstring.h>
#include <qpainter.h>
#include <qstyle.h>
#include <qvaluelist.h>
#include <qpixmap.h>
#include <qvaluevector.h>

// KDE includes.

#include <kapplication.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>

// Local includes.

#include "ddebug.h"
#include "albumiconview.h"
#include "albumiconitem.h"
#include "albummanager.h"
#include "albumdb.h"
#include "album.h"
#include "syncjob.h"
#include "tageditdlg.h"
#include "tagspopupmenu.h"
#include "tagspopupmenu.moc"

namespace Digikam
{

class TagsPopupCheckedMenuItem : public QCustomMenuItem
{
public:

    TagsPopupCheckedMenuItem(QPopupMenu* popup, const QString& txt, const QPixmap& pix)
        : QCustomMenuItem(), m_popup(popup), m_txt(txt), m_pix(pix)
    {
    }

    virtual QSize sizeHint()
    {
        QFont fn = m_popup->font();
        QFontMetrics fm(fn);
        int w = fm.width(m_txt) + 5 + kapp->style().pixelMetric(QStyle::PM_IndicatorWidth, 0);
        int h = QMAX(fm.height(), m_pix.height());
        return QSize( w, h );
    }

    virtual void paint(QPainter* p, const QColorGroup& cg, bool act, bool enabled,
                       int x, int y, int w, int h )
    {
        p->save();
        p->setPen(act ? cg.highlightedText() : cg.highlight());
        p->drawText(x, y, w, h, Qt::AlignLeft|Qt::AlignVCenter, m_txt);
        p->restore();

        if (!m_pix.isNull())
        {
            QRect pixRect(x/2 - m_pix.width()/2, y, m_pix.width(), m_pix.height());
            p->drawPixmap( pixRect.topLeft(), m_pix );
        }

        int checkWidth  = kapp->style().pixelMetric(QStyle::PM_IndicatorWidth,  0);
        int checkHeight = kapp->style().pixelMetric(QStyle::PM_IndicatorHeight, 0);

        QStyle::SFlags flags = QStyle::Style_Default;
        flags |= QStyle::Style_On;
        if (enabled)
            flags |= QStyle::Style_Enabled;
        if (act)
            flags |= QStyle::Style_Active;
        
        QFont fn = m_popup->font();
        QFontMetrics fm(fn);
        QRect r(x + 5 + fm.width(m_txt), y + (h/2-checkHeight/2), checkWidth, checkHeight);
        kapp->style().drawPrimitive(QStyle::PE_CheckMark, p, r, cg, flags);
    }

private:

    QPopupMenu *m_popup;

    QString     m_txt;

    QPixmap     m_pix;
};

// ------------------------------------------------------------------------

class TagsPopupMenuPriv
{
public:

    TagsPopupMenuPriv(){}

    int                 addToID;

    QPixmap             addTagPix;

    QValueList<int>     assignedTags;
    QValueList<Q_LLONG> selectedImageIDs;

    TagsPopupMenu::Mode mode;
};

TagsPopupMenu::TagsPopupMenu(const QValueList<Q_LLONG>& selectedImageIDs, int addToID, Mode mode)
             : QPopupMenu(0)
{
    d = new TagsPopupMenuPriv;
    d->selectedImageIDs = selectedImageIDs;
    d->addToID          = addToID;
    d->mode             = mode;

    KIconLoader *iconLoader = KApplication::kApplication()->iconLoader();
    d->addTagPix            = iconLoader->loadIcon("tag",
                                        KIcon::NoGroup,
                                        KIcon::SizeSmall,
                                        KIcon::DefaultState,
                                        0, true);
    
    connect(this, SIGNAL(aboutToShow()),
            this, SLOT(slotAboutToShow()));

    connect(this, SIGNAL(activated(int)),
            this, SLOT(slotActivated(int)));
}

TagsPopupMenu::~TagsPopupMenu()
{
    delete d;
}

void TagsPopupMenu::clearPopup()
{
    d->assignedTags.clear();
    clear();
}

QPopupMenu* TagsPopupMenu::buildSubMenu(int tagid)
{
    AlbumManager* man = AlbumManager::instance();
    TAlbum* album     = man->findTAlbum(tagid);
    if (!album)
        return 0;

    QPopupMenu* popup = new QPopupMenu(this);

    connect(popup, SIGNAL(activated(int)), 
            this, SLOT(slotActivated(int)));

    if (d->mode == ASSIGN)
    {
        popup->insertItem(d->addTagPix, i18n("Add New Tag..."), ADDTAGID + album->id());
        popup->insertSeparator();
                
        QPixmap pix = SyncJob::getTagThumbnail(album);
        if ((d->mode == ASSIGN) && (d->assignedTags.contains(album->id())))
        {
            popup->insertItem(new TagsPopupCheckedMenuItem(popup, album->title(), pix),
                              d->addToID + album->id());
        }
        else
        {
            popup->insertItem(pix, album->title(), d->addToID + album->id());
        }                
        
        if (album->firstChild())
        {
            popup->insertSeparator();
        }
    }
    else
    {
        if (!album->isRoot())
        {
            QPixmap pix = SyncJob::getTagThumbnail(album);
            popup->insertItem(pix, album->title(), d->addToID + album->id());
            popup->insertSeparator();
        }
    }
    
    iterateAndBuildMenu(popup, album);

    return popup;
}

void TagsPopupMenu::slotAboutToShow()
{
    clearPopup();

    AlbumManager* man = AlbumManager::instance();

    if (d->mode == REMOVE || d->mode == DISPLAY)
    {
        if (d->selectedImageIDs.isEmpty())
            return;

        d->assignedTags = man->albumDB()->getItemCommonTagIDs(d->selectedImageIDs);

        if (d->assignedTags.isEmpty())
            return;

        // also add the parents of the assigned tags
        IntList tList;
        for (IntList::iterator it = d->assignedTags.begin();
             it != d->assignedTags.end(); ++it)
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
            d->assignedTags.append(*it);
        }
    }
    else if (d->mode == ASSIGN)
    {
        if (d->selectedImageIDs.count() == 1)
        {
            d->assignedTags = man->albumDB()->getItemCommonTagIDs(d->selectedImageIDs);
        }
    }
        
    TAlbum* album = man->findTAlbum(0);
    if (!album)
        return;

    if (d->mode == ASSIGN)
    {
        insertItem(d->addTagPix, i18n("Add New Tag..."), ADDTAGID);
        if (album->firstChild())
        {
            insertSeparator();
        }
    }
    
    iterateAndBuildMenu(this, album);
}

// for qHeapSort
typedef QPair<QString, Album*> TagsMenuSortType;
bool operator<(const TagsMenuSortType &lhs, const TagsMenuSortType &rhs)
{
    return lhs.first < rhs.first;
}

void TagsPopupMenu::iterateAndBuildMenu(QPopupMenu *menu, TAlbum *album)
{
    QValueVector<TagsMenuSortType> sortedTags;

    for (Album* a = album->firstChild(); a; a = a->next())
    {
        sortedTags.push_back(qMakePair(a->title(), a));
    }

    qHeapSort(sortedTags);
    
    for (QValueVector<TagsMenuSortType>::Iterator i = sortedTags.begin(); i != sortedTags.end(); ++i)
    {
        Album *a = i->second;
        
        if (d->mode == REMOVE || d->mode == DISPLAY)
        {
            IntList::iterator it = qFind(d->assignedTags.begin(), 
                                         d->assignedTags.end(), a->id());
            if (it == d->assignedTags.end())
                continue;
        }
        
        QPixmap pix = SyncJob::getTagThumbnail((TAlbum*)a);
        QString t = a->title();
        t.replace('&',"&&");

        if (a->firstChild())
        {
            menu->insertItem(pix, t, buildSubMenu(a->id()));
        }
        else
        {
            if ((d->mode == ASSIGN) && (d->assignedTags.contains(a->id())))
            {
                menu->insertItem(new TagsPopupCheckedMenuItem(this, a->title(), pix),
                                                              d->addToID + a->id());
            }
            else
            {
                menu->insertItem(pix, t, d->addToID + a->id());
            }
        }
    }
}

void TagsPopupMenu::slotActivated(int id)
{
    if (id >= ADDTAGID)
    {
        int tagID = id - ADDTAGID;

        AlbumManager* man = AlbumManager::instance();
        TAlbum* parent    = man->findTAlbum(tagID);
        if (!parent)
        {
            DWarning() << "Failed to find album with id "
                       << tagID << endl;
            return;
        }

        QString title, icon;
        if (!TagEditDlg::tagCreate(kapp->activeWindow(), parent, title, icon))
            return;

        QMap<QString, QString> errMap;
        AlbumList tList = TagEditDlg::createTAlbum(parent, title, icon, errMap);
        TagEditDlg::showtagsListCreationError(kapp->activeWindow(), errMap);

        for (AlbumList::iterator it = tList.begin(); it != tList.end(); ++it)
            emit signalTagActivated((*it)->id());
    }
    else
    {
        int tagID = id - d->addToID;
        emit signalTagActivated(tagID);
    }
}

}  // namespace Digikam
