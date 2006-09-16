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

#define ADDTAGID 10000

// Qt includes.

#include <qpixmap.h>
#include <qstring.h>
#include <qpainter.h>
#include <qstyle.h>
#include <qvaluevector.h>

// KDE includes.

#include <kapplication.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kdebug.h>

// Local includes.

#include "albumiconview.h"
#include "albumiconitem.h"
#include "albummanager.h"
#include "albumdb.h"
#include "album.h"
#include "syncjob.h"
#include "tagcreatedlg.h"

// Local includes.

#include "tagspopupmenu.h"

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
        QRect r(x + 5 + fm.width(m_txt), y, checkWidth, checkHeight);
        kapp->style().drawPrimitive(QStyle::PE_Indicator, p, r, cg, flags);
    }

private:

    QPopupMenu* m_popup;
    QString     m_txt;
    QPixmap     m_pix;
};

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
        popup->insertItem(m_addTagPix, i18n("Add New Tag..."),
                          ADDTAGID + album->id());
        popup->insertSeparator();
                
        QPixmap pix = SyncJob::getTagThumbnail(album);
        if ((m_mode == ASSIGN) && (m_assignedTags.contains(album->id())))
        {
            popup->insertItem(new TagsPopupCheckedMenuItem(popup, album->title(), pix),
                              m_addToID + album->id());
        }
        else
        {
            popup->insertItem(pix, album->title(), m_addToID + album->id());
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
            popup->insertItem(pix, album->title(), m_addToID + album->id());
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
    else if (m_mode == ASSIGN)
    {
        if (m_selectedImageIDs.count() == 1)
        {
            m_assignedTags = man->albumDB()->getItemCommonTagIDs(m_selectedImageIDs);
        }
    }
        
    TAlbum* album = man->findTAlbum(0);
    if (!album)
        return;

    if (m_mode == ASSIGN)
    {
        insertItem(m_addTagPix, i18n("Add New Tag..."), ADDTAGID);
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
        
        if (m_mode == REMOVE)
        {
            IntList::iterator it = qFind(m_assignedTags.begin(),
                                         m_assignedTags.end(),
                                         a->id());
            if (it == m_assignedTags.end())
                continue;
        }
        
        QPixmap pix = SyncJob::getTagThumbnail((TAlbum*)a);
        if (a->firstChild())
        {
            menu->insertItem(pix, a->title(), buildSubMenu(a->id()));
        }
        else
        {
            if ((m_mode == ASSIGN) && (m_assignedTags.contains(a->id())))
            {
                menu->insertItem(new TagsPopupCheckedMenuItem(this, a->title(), pix),
                           m_addToID + a->id());
            }
            else
            {
                menu->insertItem(pix, a->title(), m_addToID + a->id());
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
        TAlbum* parent = man->findTAlbum(tagID);
        if (!parent)
        {
            kdWarning() << "Failed to find album with id "
                        << tagID << endl;
            return;
        }

        QString title, icon;
        if (!TagCreateDlg::tagCreate(kapp->activeWindow(), parent, title, icon))
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

}  // namespace Digikam

#include "tagspopupmenu.moc"
