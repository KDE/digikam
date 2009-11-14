/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-06
 * Description : Albums folder view.
 *
 * Copyright (C) 2005-2006 by Joern Ahrens <joern dot ahrens at kdemail dot net>
 * Copyright (C) 2006-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009 by Andi Clemens <andi dot clemens at gmx dot net>
 * Copyright (C) 2009 by Johannes Wienke <languitar at semipol dot de>
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

#include "albumfolderview.h"
#include "albumfolderview.moc"

// QT includes
#include <qaction.h>
#include <qsortfilterproxymodel.h>
#include <qtooltip.h>
#include <qevent.h>

// KDE includes
#include <kdebug.h>
#include <kmenu.h>
#include <kiconloader.h>

// Local includes
#include "contextmenuhelper.h"
#include "tooltipfiller.h"
#include "albummanager.h"

namespace Digikam
{

class AlbumFolderViewNewPriv
{

public:
    AlbumFolderViewNewPriv() : lastContextMenuAlbum(0)
    {
    }

    AlbumModificationHelper *albumModificationHelper;
    PAlbum *lastContextMenuAlbum;

};

AlbumFolderViewNew::AlbumFolderViewNew(QWidget *parent, AlbumModificationHelper *albumModificationHelper) :
    AlbumTreeView(parent), d(new AlbumFolderViewNewPriv)
{

    d->albumModificationHelper = albumModificationHelper;

    setSortingEnabled(true);
    setDragEnabled(true);

    // connections
    connect(this, SIGNAL(clicked(const QModelIndex&)),
            this, SLOT(slotAlbumSelected(const QModelIndex&)));

}

AlbumFolderViewNew::~AlbumFolderViewNew()
{
    delete d;
}

PAlbum* AlbumFolderViewNew::lastContextMenuAlbum() const
{
    return d->lastContextMenuAlbum;
}

void AlbumFolderViewNew::contextMenuEvent(QContextMenuEvent *event)
{

    Q_UNUSED(event);

    PAlbum *album = dynamic_cast<PAlbum*> (albumFilterModel()->albumForIndex(
                    indexAt(event->pos())));
    if (!album)
    {
        kDebug() << "No album clicked, displaying no context menu";
        return;
    }

    // we need to switch to the selected album here also on right click.
    // Otherwise most of the actions in the context menu will not be enabled
    // because they are provided by the global action collection. That in turn
    // is controlled by the selected album
    slotSelectAlbum(album);

    // ensure that we are not working on the album root that doesn't allow
    // anything else than collections
    kDebug() << "context menu requested at album " << album->title();
    if (album->isRoot())
    {
        kDebug() << "returning because there is no album or album is root";
        // if collection/date return
        d->lastContextMenuAlbum = 0;
        return;
    }
    d->lastContextMenuAlbum = album;

    // temporary actions  -------------------------------------

    QAction *renameAction    = new QAction(SmallIcon("edit-rename"), i18n("Rename..."), this);
    QAction *resetIconAction = new QAction(SmallIcon("view-refresh"), i18n("Reset Album Icon"), this);
    QAction *findDuplAction  = new QAction(SmallIcon("tools-wizard"), i18n("Find Duplicates..."), this);

    if (album->isAlbumRoot())
    {
        renameAction->setEnabled(false);
    }

    // --------------------------------------------------------

    KMenu popmenu(this);
    popmenu.addTitle(SmallIcon("digikam"), i18n("My Albums"));
    ContextMenuHelper cmhelper(&popmenu);

    cmhelper.addAction("album_new");
    cmhelper.addAction(renameAction);
    cmhelper.addAction(resetIconAction);
    cmhelper.addAction("album_openinkonqui");
    popmenu.addSeparator();
    // --------------------------------------------------------
    cmhelper.addAction(findDuplAction);
    cmhelper.addImportMenu();
    cmhelper.addExportMenu();
    cmhelper.addBatchMenu();
    cmhelper.addAlbumActions();
    popmenu.addSeparator();
    // --------------------------------------------------------
    cmhelper.addAction("album_delete");
    popmenu.addSeparator();
    // --------------------------------------------------------
    cmhelper.addAction("album_propsEdit");

    // special action handling --------------------------------

    QAction* choice = cmhelper.exec(QCursor::pos());
    if (choice)
    {
        if (choice == resetIconAction)
        {
            QString err;
            AlbumManager::instance()->updatePAlbumIcon(album, 0, err);
        }
        else if (choice == renameAction)
        {
            d->albumModificationHelper->slotAlbumRename(album);
        }
        else if (choice == findDuplAction)
        {
            kDebug() << "emitting signal for finding duplicates";
            emit signalFindDuplicatesInAlbum(album);
        }
    }

}

bool AlbumFolderViewNew::viewportEvent(QEvent *event)
{

    if (event->type() != QEvent::ToolTip)
    {
        return AlbumTreeView::viewportEvent(event);
    }

    // check that we got a correct event
    QHelpEvent *helpEvent = dynamic_cast<QHelpEvent*> (event);
    if (!helpEvent)
    {
        kError() << "Unable to determine the correct type of the event. "
                 << "This should not happen.";
        return false;
    }

    // find the item this tool tip belongs to
    PAlbum *album = albumForIndex(indexAt(helpEvent->pos()));
    if (!album || album->isRoot() || album->isAlbumRoot())
    {
        // there was no album so we really dont want to show a tooltip.
        return true;
    }

    // TODO use a custom tool tip
    QToolTip::showText(helpEvent->globalPos(), ToolTipFiller::albumTipContents(
                    album, albumModel()->albumCount(album)));

    return true;

}

void AlbumFolderViewNew::slotAlbumSelected(const QModelIndex &index)
{

    kDebug() << "slotAlbumSelected: " << index;

    AlbumManager::instance()->setCurrentAlbum(albumForIndex(index));

}

void AlbumFolderViewNew::slotSelectAlbum(Album *album)
{
    kDebug() << "Selecting album " << album;
    setCurrentIndex(albumFilterModel()->mapFromSource(
                    albumModel()->indexForAlbum(album)));
    AlbumManager::instance()->setCurrentAlbum(album);
}

}
