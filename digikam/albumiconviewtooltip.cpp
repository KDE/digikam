/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-12-10
 * Description : album icon view tool tip
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

#include "albumiconviewtooltip.h"

// Qt includes

#include <QPixmap>
#include <QPainter>
#include <QTextDocument>
#include <QDateTime>

// KDE includes

#include <kdebug.h>
#include <klocale.h>
#include <kfileitem.h>
#include <kglobal.h>
#include <kdeversion.h>

// Local includes

#include "albumiconview.h"
#include "albumiconitem.h"
#include "albumsettings.h"
#include "tooltipfiller.h"

namespace Digikam
{

class AlbumIconViewToolTipPriv
{
public:

    AlbumIconViewToolTipPriv()
    {
        view     = 0;
        iconItem = 0;
    }

    AlbumIconView *view;

    AlbumIconItem *iconItem;
};

AlbumIconViewToolTip::AlbumIconViewToolTip(AlbumIconView* view)
                    : DItemToolTip(), d(new AlbumIconViewToolTipPriv)
{
    d->view = view;
}

AlbumIconViewToolTip::~AlbumIconViewToolTip()
{
    delete d;
}

void AlbumIconViewToolTip::setIconItem(AlbumIconItem* iconItem)
{
    d->iconItem = iconItem;

    if (!d->iconItem ||
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

QRect AlbumIconViewToolTip::repositionRect()
{
    if (!d->iconItem) return QRect();

    QRect rect = d->iconItem->clickToOpenRect();
    rect.moveTopLeft(d->view->contentsToViewport(rect.topLeft()));
    rect.moveTopLeft(d->view->viewport()->mapToGlobal(rect.topLeft()));
    return rect;
}

QString AlbumIconViewToolTip::tipContents()
{
    if (!d->iconItem) return QString();
    ImageInfo info = d->iconItem->imageInfo();
    return ToolTipFiller::imageInfoTipContents(info);
}

}  // namespace Digikam
