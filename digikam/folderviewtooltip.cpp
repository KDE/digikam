/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-12-29
 * Description : folder view tool tip
 *
 * Copyright (C) 2008-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "folderviewtooltip.h"

// Qt includes

#include <QPixmap>
#include <QPainter>
#include <QTextDocument>
#include <QDateTime>

// KDE includes

#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>
#include <kdeversion.h>

// Local includes

#include "albumfolderview.h"
#include "albummanager.h"
#include "albumsettings.h"
#include "album.h"
#include "tooltipfiller.h"

namespace Digikam
{

class FolderViewToolTipPriv
{
public:

    FolderViewToolTipPriv()
    {
        view       = 0;
        folderItem = 0;
    }

    FolderView *view;

    FolderItem *folderItem;
};

FolderViewToolTip::FolderViewToolTip(FolderView* view)
                 : DItemToolTip(), d(new FolderViewToolTipPriv)
{
    d->view = view;
}

FolderViewToolTip::~FolderViewToolTip()
{
    delete d;
}

void FolderViewToolTip::setFolderItem(FolderItem* folderItem)
{
    AlbumFolderViewItem *item = dynamic_cast<AlbumFolderViewItem*>(folderItem);
    if (item)
        d->folderItem = folderItem;
    else
        d->folderItem = 0;

    if (!d->folderItem ||
        !AlbumSettings::instance()->showToolTipsIsValid())
    {
        hide();
    }
    else
    {
        updateToolTip();
        reposition();
        if (isHidden() && !toolTipIsEmpty())
            show();
    }
}

QRect FolderViewToolTip::repositionRect()
{
    if (!d->folderItem) return QRect();

    QRect rect = d->view->itemRect(dynamic_cast<Q3ListViewItem*>(d->folderItem));
    rect.moveTopLeft(d->view->viewport()->mapToGlobal(rect.topLeft()));
    return rect;
}

QString FolderViewToolTip::tipContents()
{
    if (d->folderItem)
    {
        // NOTE: For the moment only Physical Album Tooltips are supported.
        //       Extend to all virtual album types when album metadata will 
        //       be the same everywhere (comments, rating, date, etc.)
        AlbumFolderViewItem *item = dynamic_cast<AlbumFolderViewItem*>(d->folderItem);
        if (item)
        {
            PAlbum *album = item->album();
            if (album && !album->isRoot() && !album->isAlbumRoot())
                return ToolTipFiller::albumTipContents(album, item->isOpen() ? item->count()
                                                                     : item->countRecursive());
        }
    }
    return QString();
}

}  // namespace Digikam
