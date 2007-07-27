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
 * Copyright (C) 2006-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <QSet>
#include <QPixmap>
#include <QString>
#include <QPainter>
#include <QStyle>
#include <QPixmap>

// KDE includes.

#include <kapplication.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ktoggleaction.h>

// Local includes.

#include "ddebug.h"
#include "albumiconview.h"
#include "albumiconitem.h"
#include "albummanager.h"
#include "albumdb.h"
#include "album.h"
#include "syncjob.h"
#include "tagcreatedlg.h"
#include "tagspopupmenu.h"
#include "tagspopupmenu.moc"

namespace Digikam
{

/*

class TagsPopupCheckedMenuItem : public QCustomMenuItem
{

public:

    TagsPopupCheckedMenuItem(Q3PopupMenu* popup, const QString& txt, const QPixmap& pix)
        : QCustomMenuItem(), m_popup(popup), m_txt(txt), m_pix(pix)
    {
    }

    virtual QSize sizeHint()
    {
        QFont fn = m_popup->font();
        QFontMetrics fm(fn);
        int w = fm.width(m_txt) + 5 + kapp->style().pixelMetric(QStyle::PM_IndicatorWidth, 0);
        int h = qMax(fm.height(), m_pix.height());
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

    Q3PopupMenu *m_popup;

    QString     m_txt;

    QPixmap     m_pix;
};
*/

class TagToggleAction : public KToggleAction
{
public:
    TagToggleAction(const KIcon &icon, const QString &text, QObject *parent)
    : KToggleAction(icon, text, parent)
    {
    }
};

// ------------------------------------------------------------------------

class TagsPopupMenuPriv
{
public:

    TagsPopupMenuPriv(){}

    QPixmap                addTagPix;

    QSet<int>              assignedTags;
    QList<qlonglong>       selectedImageIDs;

    TagsPopupMenu::Mode    mode;

    QActionGroup          *addTagActions;
    QActionGroup          *toggleTagActions;
};

TagsPopupMenu::TagsPopupMenu(qlonglong selectedImageId, Mode mode)
             : QMenu(0)
{
    d = new TagsPopupMenuPriv;
    d->selectedImageIDs << selectedImageId;
    setup(mode);
}

TagsPopupMenu::TagsPopupMenu(const QList<qlonglong>& selectedImageIds, Mode mode)
             : QMenu(0)
{
    d = new TagsPopupMenuPriv;
    d->selectedImageIDs = selectedImageIds;
    setup(mode);
}

void TagsPopupMenu::setup(Mode mode)
{
    d->mode             = mode;

    KIconLoader *iconLoader = KIconLoader::global();
    d->addTagPix            = iconLoader->loadIcon("tag", K3Icon::NoGroup, K3Icon::SizeSmall);

    d->addTagActions    = new QActionGroup(this);
    d->toggleTagActions = new QActionGroup(this);

    connect(d->addTagActions, SIGNAL(triggered(QAction*)),
            this, SLOT(slotAddTag(QAction*)));

    connect(d->toggleTagActions, SIGNAL(triggered(QAction*)),
            this, SLOT(slotToggleTag(QAction*)));

    connect(this, SIGNAL(aboutToShow()),
            this, SLOT(slotAboutToShow()));
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

void TagsPopupMenu::slotAboutToShow()
{
    clearPopup();

    AlbumManager* man = AlbumManager::componentData();

    if (d->mode == REMOVE)
    {
        if (d->selectedImageIDs.isEmpty())
            return;

        d->assignedTags = QSet<int>::fromList(DatabaseAccess().db()->getItemCommonTagIDs(d->selectedImageIDs));

        if (d->assignedTags.isEmpty())
            return;

        // also add the parents of the assigned tags
        QSet<int> parents;
        for (QSet<int>::iterator it = d->assignedTags.begin();
             it != d->assignedTags.end(); ++it)
        {
            TAlbum* album = man->findTAlbum(*it);
            if (album)
            {
                Album* a = album->parent();
                while (a)
                {
                    parents << a->id();
                    a = a->parent();
                }
            }
        }
        d->assignedTags += parents;
    }
    else if (d->mode == ASSIGN)
    {
        if (d->selectedImageIDs.count() == 1)
        {
            d->assignedTags = QSet<int>::fromList(DatabaseAccess().db()->getItemCommonTagIDs(d->selectedImageIDs));
        }
    }

    TAlbum* album = man->findTAlbum(0);
    if (!album)
        return;

    if (d->mode == ASSIGN)
    {
        QAction *action = addAction(d->addTagPix, i18n("Add New Tag..."));
        action->setData(0); // root id
        d->addTagActions->addAction(action);

        if (album->firstChild())
        {
            addSeparator();
        }
    }

    iterateAndBuildMenu(this, album);
}

// for qSort
bool lessThanByTitle(const Album *first, const Album *second)
{
    return first->title() < second->title();
}

void TagsPopupMenu::iterateAndBuildMenu(QMenu *menu, TAlbum *album)
{
    QList<Album*> sortedTags;
    for (Album* a = album->firstChild(); a; a = a->next())
    {
        sortedTags << a;
    }
    qStableSort(sortedTags.begin(), sortedTags.end(), lessThanByTitle);

    for (QList<Album*>::iterator it = sortedTags.begin(); it != sortedTags.end(); ++it)
    {
        Album *a = *it;

        if (d->mode == REMOVE)
        {
            if (!d->assignedTags.contains(a->id()))
                continue;
        }

        QPixmap pix = SyncJob::getTagThumbnail((TAlbum*)a);
        QString t = a->title();
        t.replace('&',"&&");

        if (a->firstChild())
        {
            QAction *menuAction = menu->addMenu(buildSubMenu(a->id()));
            menuAction->setText(t);
            menuAction->setIcon(pix);
        }
        else
        {
            KToggleAction *action;
            if ((d->mode == ASSIGN) && (d->assignedTags.contains(a->id())))
            {
                action = new TagToggleAction(KIcon(pix), t, this);
            }
            else
            {
                action = new KToggleAction(KIcon(pix), t, this);
            }

            action->setData(a->id());
            d->toggleTagActions->addAction(action);
            menu->addAction(action);
        }
    }
}

QMenu* TagsPopupMenu::buildSubMenu(int tagid)
{
    AlbumManager* man = AlbumManager::componentData();
    TAlbum* album     = man->findTAlbum(tagid);
    if (!album)
        return 0;

    QMenu* popup = new QMenu(this);

    if (d->mode == ASSIGN)
    {
        QAction *action = popup->addAction(d->addTagPix, i18n("Add New Tag..."));
        action->setData(album->id());
        d->addTagActions->addAction(action);

        popup->addSeparator();
    }

    QPixmap pix = SyncJob::getTagThumbnail(album);

    KToggleAction *action;
    if ((d->mode == ASSIGN) && (d->assignedTags.contains(album->id())))
    {
        action = new TagToggleAction(KIcon(pix), album->title(), d->toggleTagActions);
    }
    else
    {
        action = new KToggleAction(KIcon(pix), album->title(), d->toggleTagActions);
    }

    action->setData(album->id());
    d->toggleTagActions->addAction(action);
    popup->addAction(action);

    if (d->mode == REMOVE || album->firstChild())
    {
        popup->addSeparator();
    }

    iterateAndBuildMenu(popup, album);

    return popup;
}

void TagsPopupMenu::slotToggleTag(QAction *action)
{
    int tagID = action->data().toInt();
    emit signalTagActivated(tagID);
}

void TagsPopupMenu::slotAddTag(QAction *action)
{
    int tagID = action->data().toInt();

    AlbumManager* man = AlbumManager::componentData();
    TAlbum* parent    = man->findTAlbum(tagID);
    if (!parent)
    {
        DWarning() << "Failed to find album with id "
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

}  // namespace Digikam
