/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-03-03
 * Description : queue tool tip
 *
 * Copyright (C) 2009-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "queuetooltip.h"

// Qt includes

#include <QDateTime>
#include <QPainter>
#include <QPixmap>
#include <QTextDocument>

// KDE includes

#include <klocale.h>

// Local includes

#include "albumsettings.h"
#include "queuelist.h"
#include "tooltipfiller.h"

namespace Digikam
{

class QueueToolTip::QueueToolTipPriv
{
public:

    QueueToolTipPriv() :
        view(0),
        item(0)
    {
    }

    QueueListView*     view;
    QueueListViewItem* item;
};

QueueToolTip::QueueToolTip(QueueListView* view)
    : DItemToolTip(), d(new QueueToolTipPriv)
{
    d->view = view;
}

QueueToolTip::~QueueToolTip()
{
    delete d;
}

void QueueToolTip::setQueueItem(QueueListViewItem* item)
{
    d->item = item;

    if (!d->item ||
        !AlbumSettings::instance()->showToolTipsIsValid())
    {
        hide();
    }
    else
    {
        updateToolTip();
        reposition();

        if (isHidden() && !toolTipIsEmpty())
        {
            show();
        }
    }
}

QRect QueueToolTip::repositionRect()
{
    if (!d->item)
    {
        return QRect();
    }

    QRect rect = d->view->visualItemRect(d->item);
    rect.moveTopLeft(d->view->viewport()->mapToGlobal(rect.topLeft()));
    return rect;
}

QString QueueToolTip::tipContents()
{
    if (!d->item)
    {
        return QString();
    }

    ImageInfo info = d->item->info();
    return ToolTipFiller::imageInfoTipContents(info);
}

}  // namespace Digikam
