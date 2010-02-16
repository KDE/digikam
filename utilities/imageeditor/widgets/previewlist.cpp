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
#include <QMap>

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

class PreviewThreadWrapperPriv
{

public:

    PreviewThreadWrapperPriv()
    {
    }

    QMap<int, DImgThreadedFilter*> map;
};

PreviewThreadWrapper::PreviewThreadWrapper(QObject* parent)
                    : QObject(parent), d(new PreviewThreadWrapperPriv)
{
}

PreviewThreadWrapper::~PreviewThreadWrapper()
{
    delete d;
}

void PreviewThreadWrapper::registerFilter(int id, DImgThreadedFilter* filter)
{
    filter->setParent(this);
    d->map.insert(id, filter);
        
    connect(filter, SIGNAL(started()),
            this, SLOT(slotFilterStarted()));

    connect(filter, SIGNAL(finished(bool)),
            this, SLOT(slotFilterFinished(bool)));

    connect(filter, SIGNAL(progress(int)),
            this, SLOT(slotFilterProgress(int)));
}
    
void PreviewThreadWrapper::slotFilterStarted()
{
    DImgThreadedFilter* filter = dynamic_cast<DImgThreadedFilter*>(sender());
    if (!filter) return;

    emit signalFilterStarted(d->map.key(filter));
}

void PreviewThreadWrapper::slotFilterFinished(bool success)
{
    DImgThreadedFilter* filter = dynamic_cast<DImgThreadedFilter*>(sender());
    if (!filter) return;

    if (success)
    {
        QPixmap pix = filter->getTargetImage().convertToPixmap().scaled(128, 128, Qt::KeepAspectRatio);
        emit signalFilterFinished(d->map.key(filter), pix);
    }
}

void PreviewThreadWrapper::slotFilterProgress(int /*progress*/)
{
    DImgThreadedFilter* filter = dynamic_cast<DImgThreadedFilter*>(sender());
    if (!filter) return;

//    kDebug() << filter->filterName() << " : " << progress << " %";
}

void PreviewThreadWrapper::startFilters()
{
    QList<DImgThreadedFilter*> list = d->map.values();
    foreach(DImgThreadedFilter* filter, list)
    {
        filter->startFilter();
    }
}

void PreviewThreadWrapper::stopFilters()
{
    QList<DImgThreadedFilter*> list = d->map.values();
    foreach(DImgThreadedFilter* filter, list)
    {
        filter->cancelFilter();
        filter->deleteLater();
    }
}

// ---------------------------------------------------------------------
  
class PreviewListItemPriv
{

public:

    PreviewListItemPriv()
    {
        busy = false;
        id   = 0;
    }

    bool busy;
    int  id;
};

PreviewListItem::PreviewListItem(QTreeWidget* parent)
               : QTreeWidgetItem(parent), d(new PreviewListItemPriv)
{
}

PreviewListItem::~PreviewListItem()
{
    delete d;
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
        wrapper       = 0;
        progressCount = 0;
        progressTimer = 0;
        progressPix   = SmallIcon("process-working", 22);
    }

    int                   progressCount;

    QTimer*               progressTimer;

    QPixmap               progressPix;
    
    PreviewThreadWrapper* wrapper;
};

PreviewList::PreviewList(QObject* parent)
           : QTreeWidget(), d(new PreviewListPriv)
{
    d->wrapper = new PreviewThreadWrapper(parent);
    
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

    connect(d->wrapper, SIGNAL(signalFilterStarted(int)),
            this, SLOT(slotFilterStarted(int)));

    connect(d->wrapper, SIGNAL(signalFilterFinished(int, const QPixmap&)),
            this, SLOT(slotFilterFinished(int, const QPixmap&)));    
}

PreviewList::~PreviewList()
{
    stopFilters();
    delete d;
}

void PreviewList::startFilters()
{
    d->progressTimer->start();
    d->wrapper->startFilters();
}

void PreviewList::stopFilters()
{
    d->progressTimer->stop();
    d->wrapper->stopFilters();
}

PreviewListItem* PreviewList::addItem(DImgThreadedFilter* filter, const QString& txt, int id)
{
    if (!filter) return 0;

    d->wrapper->registerFilter(id, filter);

    PreviewListItem* item = new PreviewListItem(this);
    item->setText(0, txt);
    item->setId(id);
    return item;
}

PreviewListItem* PreviewList::findItem(int id)
{
    QTreeWidgetItemIterator it(this);
    while (*it)
    {
        PreviewListItem* item = dynamic_cast<PreviewListItem*>(*it);
        if (item && item->id() == id)
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

void PreviewList::slotFilterStarted(int id)
{
    PreviewListItem* item = findItem(id);
    item->setBusy(true);
}

void PreviewList::slotFilterFinished(int id, const QPixmap& pix)
{
    PreviewListItem* item = findItem(id);
    item->setBusy(false);
    item->setPixmap(pix);
}

}  // namespace Digikam
