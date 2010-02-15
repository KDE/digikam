/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-02-13
 * Description : a list of selectable options with preview
 *               effects as thumbnails.
 *
 * Copyright (C) 2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "previewlist.moc"

// Qt includes

#include <QListWidgetItem>
#include <QPixmap>

// KDE includes

#include <kapplication.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kstandarddirs.h>

// Local includes

#include "dimg.h"
#include "dimgthreadedfilter.h"
#include "imageiface.h"

namespace Digikam
{

class PreviewListItemPriv
{

public:

    PreviewListItemPriv()
    {
        filter = 0;
    }

    DImgThreadedFilter* filter;
};

PreviewListItem::PreviewListItem(QListWidget* parent)
               : QListWidgetItem(parent), d(new PreviewListItemPriv)
{
}

PreviewListItem::~PreviewListItem()
{
    delete d;
}

void PreviewListItem::setFilter(DImgThreadedFilter* filter)
{
    d->filter = filter;
}

DImgThreadedFilter* PreviewListItem::filter() const
{
    return d->filter;
}

// ---------------------------------------------------------------------

class PreviewListPriv
{

public:

    PreviewListPriv()
    {
        image = 0;
    }

    DImg* image;
};

PreviewList::PreviewList(QWidget* parent)
           : QListWidget(parent), d(new PreviewListPriv)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setIconSize(QSize(128, 128));

    ImageIface iface(0, 0);
    d->image = iface.getOriginalImg();
}

PreviewList::~PreviewList()
{
    delete d;
}

PreviewListItem* PreviewList::addItem(DImgThreadedFilter* filter, const QString& txt)
{
    if (!filter) return 0;

    PreviewListItem* item  = new PreviewListItem(this);
    item->setFilter(filter);
    item->setText(txt);

    connect(filter, SIGNAL(started()),
            this, SLOT(slotFilterStarted()));

    connect(filter, SIGNAL(finished(bool)),
            this, SLOT(slotFilterFinished(bool)));

    connect(filter, SIGNAL(progress(int)),
            this, SLOT(slotFilterProgress(int)));

    filter->startFilter();

    return item;
}

void PreviewList::slotFilterStarted()
{
    DImgThreadedFilter* filter = dynamic_cast<DImgThreadedFilter*>(sender());
    if (!filter) return;

    PreviewListItem* item = findItem(filter);
    item->setIcon(SmallIcon("run"));
}

void PreviewList::slotFilterFinished(bool /*success*/)
{
    DImgThreadedFilter* filter = dynamic_cast<DImgThreadedFilter*>(sender());
    if (!filter) return;

    PreviewListItem* item = findItem(filter);
    item->setIcon(QIcon(filter->getTargetImage().convertToPixmap().scaled(128, 128, Qt::KeepAspectRatio)));
}

void PreviewList::slotFilterProgress(int /*progress*/)
{
    DImgThreadedFilter* filter = dynamic_cast<DImgThreadedFilter*>(sender());
    if (!filter) return;

    PreviewListItem* item = findItem(filter);
    //item->setIcon();
}

PreviewListItem* PreviewList::findItem(DImgThreadedFilter* filter)
{
    for (int i = 0 ; i < count() ; ++i)
    {
        PreviewListItem* it = dynamic_cast<PreviewListItem*>(item(i));
        if (it && it->filter() == filter)
            return it;
    }
    return 0;
}

}  // namespace Digikam
