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

#include <QPixmap>
#include <QHeaderView>

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
        id     = 0;
        filter = 0;
    }

    int                 id;
    DImgThreadedFilter* filter;
};

PreviewListItem::PreviewListItem(QTreeWidget* parent)
               : QTreeWidgetItem(parent), d(new PreviewListItemPriv)
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

void PreviewListItem::setPixmap(const QPixmap& pix)
{
    setIcon(0, QIcon(pix));
}

void PreviewListItem::setId(int id)
{
    d->id = id;
}

int PreviewListItem::id()
{
    return d->id;
}

// ---------------------------------------------------------------------

class PreviewListPriv
{

public:

    PreviewListPriv()
    {

    }

};

PreviewList::PreviewList(QWidget* parent)
           : QTreeWidget(parent), d(new PreviewListPriv)
{
    setSelectionMode(QAbstractItemView::SingleSelection);
    setDropIndicatorShown(true);
    setSortingEnabled(false);
    setAllColumnsShowFocus(true);
    setRootIsDecorated(false);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setColumnCount(1);
    setIconSize(QSize(128, 128));
    setHeaderHidden(true);
    header()->setResizeMode(QHeaderView::Stretch);
}

PreviewList::~PreviewList()
{
    delete d;
}

PreviewListItem* PreviewList::addItem(DImgThreadedFilter* filter, const QString& txt, int id)
{
    if (!filter) return 0;

    PreviewListItem* item  = new PreviewListItem(this);
    item->setFilter(filter);
    item->setText(0, txt);
    item->setId(id);

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
    item->setIcon(0, SmallIconSet("system-run", 128));
}

void PreviewList::slotFilterFinished(bool /*success*/)
{
    DImgThreadedFilter* filter = dynamic_cast<DImgThreadedFilter*>(sender());
    if (!filter) return;

    PreviewListItem* item = findItem(filter);
    item->setPixmap(filter->getTargetImage().convertToPixmap().scaled(128, 128, Qt::KeepAspectRatio));
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
    QTreeWidgetItemIterator it(this);
    while (*it)
    {
        PreviewListItem* item = dynamic_cast<PreviewListItem*>(*it);
        if (item && item->filter() == filter)
            return item;

        ++it;
    }
    return 0;
}

void PreviewList::setCurrentId(int id)
{
    QTreeWidgetItemIterator it(this);
    while (*it)
    {
        PreviewListItem* item = dynamic_cast<PreviewListItem*>(*it);
        if (item && item->id() == id)
        {
            setCurrentItem(item);
            item->setSelected(true);
            return;
        }
        ++it;
    }
}

int PreviewList::currentId()
{
    PreviewListItem* item = dynamic_cast<PreviewListItem*>(currentItem());
    if (item ) return item->id();

    return 0;
}

}  // namespace Digikam
