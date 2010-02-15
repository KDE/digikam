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
#include <QTimer>
#include <QPainter>

// KDE includes

#include <kapplication.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kdebug.h>

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
        busy   = false;
        id     = 0;
        filter = 0;
    }

    bool                busy;
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

void PreviewListItem::setBusy(bool b)
{
    d->busy = b;
    kDebug() << "busy: " << b;
}

bool PreviewListItem::isBusy()
{
    return d->busy;
}

// ---------------------------------------------------------------------

class PreviewListPriv
{

public:

    PreviewListPriv()
    {
        progressCount = 0;
        progressTimer = 0;
        progressPix   = SmallIcon("process-working", 22);
    }

    int     progressCount;

    QTimer* progressTimer;

    QPixmap progressPix;
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
    
    d->progressTimer = new QTimer(this);
    d->progressTimer->setInterval(300);
    
    connect(d->progressTimer, SIGNAL(timeout()),
            this, SLOT(slotProgressTimerDone()));    
}

PreviewList::~PreviewList()
{
    stopFilters();
    delete d;
}

void PreviewList::startFilters()
{
    d->progressTimer->start();

    QTreeWidgetItemIterator it(this);
    while (*it)
    {
        PreviewListItem* item = dynamic_cast<PreviewListItem*>(*it);
        if (item)
            item->filter()->startFilter();

        ++it;
    }    
}

void PreviewList::stopFilters()
{
    d->progressTimer->stop();

    QTreeWidgetItemIterator it(this);
    while (*it)
    {
        PreviewListItem* item = dynamic_cast<PreviewListItem*>(*it);
        if (item)
        {
            if (item->isBusy())
                item->filter()->cancelFilter();
            
            delete item->filter();
        }

        ++it;
    }
}

PreviewListItem* PreviewList::addItem(DImgThreadedFilter* filter, const QString& txt, int id)
{
    if (!filter) return 0;

    PreviewListItem* item = new PreviewListItem(this);
    item->setFilter(filter);
    item->setText(0, txt);
    item->setId(id);

    connect(filter, SIGNAL(started()),
            this, SLOT(slotFilterStarted()));

    connect(filter, SIGNAL(finished(bool)),
            this, SLOT(slotFilterFinished(bool)));

    connect(filter, SIGNAL(progress(int)),
            this, SLOT(slotFilterProgress(int)));

    return item;
}

void PreviewList::slotFilterStarted()
{
    DImgThreadedFilter* filter = dynamic_cast<DImgThreadedFilter*>(sender());
    if (!filter) return;

    PreviewListItem* item = findItem(filter);
    item->setBusy(true);
}

void PreviewList::slotFilterFinished(bool success)
{
    DImgThreadedFilter* filter = dynamic_cast<DImgThreadedFilter*>(sender());
    if (!filter) return;

    PreviewListItem* item = findItem(filter);
    item->setBusy(false);
    if (success)
        item->setPixmap(filter->getTargetImage().convertToPixmap().scaled(128, 128, Qt::KeepAspectRatio));
}

void PreviewList::slotFilterProgress(int /*progress*/)
{
    DImgThreadedFilter* filter = dynamic_cast<DImgThreadedFilter*>(sender());
    if (!filter) return;

//    kDebug() << filter->filterName() << " : " << progress << " %";
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

void PreviewList::slotProgressTimerDone()
{
    kDebug() << "timer shot";

    QPixmap ppix(d->progressPix.copy(0, d->progressCount*22, 22, 22));
    QPixmap pixmap(128, 128);
    pixmap.fill(Qt::transparent);
    QPainter p(&pixmap);
    p.drawPixmap((pixmap.width()/2) - (ppix.width()/2), (pixmap.height()/2) - (ppix.height()/2), ppix);
    
    int busy = 0;
    QTreeWidgetItemIterator it(this);
    while (*it)
    {
        PreviewListItem* item = dynamic_cast<PreviewListItem*>(*it);
        if (item && item->isBusy())
        {
            item->setPixmap(pixmap);
            ++busy;
        }

        ++it;
    }
    d->progressCount++;    
    if (d->progressCount == 8) d->progressCount = 0;

    kDebug() << "item busy : " << busy;
    if (!busy)
        d->progressTimer->stop();
}

}  // namespace Digikam
