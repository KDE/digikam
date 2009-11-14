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


#include <klocale.h>
#include <kglobal.h>
#include <kdeversion.h>

// Local includes

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
        folderItem = 0;
    }

    PAlbum *folderItem;
};

FolderViewToolTip::FolderViewToolTip(QWidget *parent)
                 : DItemToolTip(parent), d(new FolderViewToolTipPriv)
{
}

FolderViewToolTip::~FolderViewToolTip()
{
    delete d;
}

void FolderViewToolTip::setAlbum(PAlbum *album)
{
    if (album)
        d->folderItem = album;
    else
        d->folderItem = 0;

    if (!d->folderItem ||
        !AlbumSettings::instance()->showAlbumToolTipsIsValid())
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

    //QRect rect = d->view->itemRect(dynamic_cast<Q3ListViewItem*>(d->folderItem));
    //rect.moveTopLeft(d->view->viewport()->mapToGlobal(rect.topLeft()));
    //return rect;
    return QRect();
}

QString FolderViewToolTip::tipContents()
{
//    if (d->folderItem && !d->folderItem->isRoot()
//                    && !d->folderItem->isAlbumRoot())
//    {
//        return ToolTipFiller::albumTipContents(d->folderItem,
//                        item->isOpen() ? item->count() : item->countRecursive());
//    }
    return QString();
}

}  // namespace Digikam
