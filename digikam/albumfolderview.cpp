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
    AlbumFolderViewNewPriv() :
        albumModificationHelper(0),
        enableToolTips(false)
    {
    }

    AlbumModificationHelper *albumModificationHelper;
    bool enableToolTips;

    QAction *renameAction;
    QAction *resetIconAction;
    QAction *findDuplAction;

};

AlbumFolderViewNew::AlbumFolderViewNew(QWidget *parent, AlbumModel *model,
                AlbumModificationHelper *albumModificationHelper) :
    AlbumTreeView(model, parent), d(new AlbumFolderViewNewPriv)
{

    d->albumModificationHelper = albumModificationHelper;

    d->renameAction    = new QAction(SmallIcon("edit-rename"), i18n("Rename..."), this);
    d->resetIconAction = new QAction(SmallIcon("view-refresh"), i18n("Reset Album Icon"), this);
    d->findDuplAction  = new QAction(SmallIcon("tools-wizard"), i18n("Find Duplicates..."), this);

    setSortingEnabled(true);
    setSelectAlbumOnClick(true);
    setEnableContextMenu(true);

    // connections
    connect(this, SIGNAL(clicked(const QModelIndex&)),
            this, SLOT(slotAlbumSelected(const QModelIndex&)));

}

AlbumFolderViewNew::~AlbumFolderViewNew()
{
    delete d;
}

void AlbumFolderViewNew::setEnableToolTips(bool enable)
{
    d->enableToolTips = enable;
}

QString AlbumFolderViewNew::contextMenuTitle() const
{
    return i18n("My Albums");
}

void AlbumFolderViewNew::addCustomContextMenuActions(ContextMenuHelper &cmh, Album *a)
{

    PAlbum *album = dynamic_cast<PAlbum*> (a);
    if (!a)
    {
        return;
    }

    d->renameAction->setEnabled(!album->isAlbumRoot());

    // --------------------------------------------------------

    cmh.addAction("album_new");
    cmh.addAction(d->renameAction);
    cmh.addAction(d->resetIconAction);
    cmh.addAction("album_openinkonqui");
    cmh.addSeparator();
    // --------------------------------------------------------
    cmh.addAction(d->findDuplAction);
    cmh.addImportMenu();
    cmh.addExportMenu();
    cmh.addBatchMenu();
    cmh.addAlbumActions();
    cmh.addSeparator();
    // --------------------------------------------------------
    cmh.addAction("album_delete");
    cmh.addSeparator();
    // --------------------------------------------------------
    cmh.addAction("album_propsEdit");

}

void AlbumFolderViewNew::handleCustomContextMenuAction(QAction *action, Album *a)
{

    PAlbum *album = dynamic_cast<PAlbum*> (a);

    if (!action || !album)
    {
        return;
    }

    if (action == d->resetIconAction)
    {
        QString err;
        AlbumManager::instance()->updatePAlbumIcon(album, 0, err);
    }
    else if (action == d->renameAction)
    {
        d->albumModificationHelper->slotAlbumRename(album);
    }
    else if (action == d->findDuplAction)
    {
        kDebug() << "emitting signal for finding duplicates";
        emit signalFindDuplicatesInAlbum(album);
    }

}

bool AlbumFolderViewNew::viewportEvent(QEvent *event)
{

    // let the base class handle the event if the extended tool tips aren't
    // requested by the user or the event is not related to tool tips at all
    if (!d->enableToolTips || event->type() != QEvent::ToolTip)
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

}
