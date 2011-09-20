/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-02-13
 * Description : a list of selectable options with preview
 *               effects as thumbnails.
 *
 * Copyright (C) 2010-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include <QTimer>
#include <QPainter>
#include <QMap>

// KDE includes

#include <kapplication.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kpixmapsequence.h>
#include <kstandarddirs.h>
#include <kdebug.h>

// Local includes

#include "dimg.h"
#include "dimgthreadedfilter.h"
#include "imageiface.h"

namespace Digikam
{

class PreviewThreadWrapper::PreviewThreadWrapperPriv
{

public:

    PreviewThreadWrapperPriv() {}

    QMap<int, DImgThreadedFilter*> map;
};

PreviewThreadWrapper::PreviewThreadWrapper(QObject* parent)
    : QObject(parent), d(new PreviewThreadWrapperPriv)
{
}

PreviewThreadWrapper::~PreviewThreadWrapper()
{
    foreach (DImgThreadedFilter* filter, d->map.values())
    {
        delete filter;
    }

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

    if (!filter)
    {
        return;
    }

    emit signalFilterStarted(d->map.key(filter));
}

void PreviewThreadWrapper::slotFilterFinished(bool success)
{
    DImgThreadedFilter* filter = dynamic_cast<DImgThreadedFilter*>(sender());

    if (!filter)
    {
        return;
    }

    if (success)
    {
        int key     = d->map.key(filter);
        QPixmap pix = filter->getTargetImage().smoothScale(128, 128, Qt::KeepAspectRatio).convertToPixmap();
        emit signalFilterFinished(key, pix);
    }
}

void PreviewThreadWrapper::slotFilterProgress(int /*progress*/)
{
    DImgThreadedFilter* filter = dynamic_cast<DImgThreadedFilter*>(sender());

    if (!filter)
    {
        return;
    }

    //kDebug() << filter->filterName() << " : " << progress << " %";
}

void PreviewThreadWrapper::startFilters()
{
    foreach(DImgThreadedFilter* filter, d->map)
    {
        filter->startFilter();
    }
}

void PreviewThreadWrapper::stopFilters()
{
    foreach(DImgThreadedFilter* filter, d->map)
    {
        filter->cancelFilter();
        filter->deleteLater();
    }
}

// ---------------------------------------------------------------------

class PreviewListItem::PreviewListItemPriv
{

public:

    PreviewListItemPriv() :
        busy(false),
        id(0)
    {
    }

    bool busy;
    int  id;
};

PreviewListItem::PreviewListItem(QListWidget* parent)
    : QListWidgetItem(parent), d(new PreviewListItemPriv)
{
}

PreviewListItem::~PreviewListItem()
{
    delete d;
}

void PreviewListItem::setPixmap(const QPixmap& pix)
{
    QIcon icon = QIcon(pix);
    //  We make sure the preview icon stays the same regardless of the role
    icon.addPixmap(pix, QIcon::Selected, QIcon::On);
    icon.addPixmap(pix, QIcon::Selected, QIcon::Off);
    icon.addPixmap(pix, QIcon::Active, QIcon::On);
    icon.addPixmap(pix, QIcon::Active, QIcon::Off);
    icon.addPixmap(pix, QIcon::Normal, QIcon::On);
    icon.addPixmap(pix, QIcon::Normal, QIcon::Off);
    setIcon(icon);
}

void PreviewListItem::setId(int id)
{
    d->id = id;
}

int PreviewListItem::id() const
{
    return d->id;
}

void PreviewListItem::setBusy(bool b)
{
    d->busy = b;
}

bool PreviewListItem::isBusy() const
{
    return d->busy;
}

// ---------------------------------------------------------------------

class PreviewList::PreviewListPriv
{

public:

    PreviewListPriv() :
        progressCount(0),
        progressTimer(0),
        progressPix(KPixmapSequence("process-working", KIconLoader::SizeSmallMedium)),
        wrapper(0)
    {
    }

    int                   progressCount;

    QTimer*               progressTimer;

    KPixmapSequence       progressPix;

    PreviewThreadWrapper* wrapper;
};

PreviewList::PreviewList(QObject* /*parent*/)
    : QListWidget(), d(new PreviewListPriv)
{
    d->wrapper = new PreviewThreadWrapper(this);

    setSelectionMode(QAbstractItemView::SingleSelection);
    setDropIndicatorShown(true);
    setSortingEnabled(false);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    setIconSize(QSize(128, 128));
    setViewMode(QListView::IconMode);
    setWrapping(true);
    setWordWrap(false);
    setMovement(QListView::Static);
    setSpacing(5);
    setGridSize(QSize(130,130 + fontMetrics().height()));
    setMinimumHeight(400);
    setResizeMode(QListView::Adjust);
    setTextElideMode(Qt::ElideRight);
    setCursor(Qt::PointingHandCursor);
    setStyleSheet("QListWidget::item:selected:!active {show-decoration-selected: 0}");

    d->progressTimer = new QTimer(this);
    d->progressTimer->setInterval(300);

    connect(d->progressTimer, SIGNAL(timeout()),
            this, SLOT(slotProgressTimerDone()));

    connect(d->wrapper, SIGNAL(signalFilterStarted(int)),
            this, SLOT(slotFilterStarted(int)));

    connect(d->wrapper, SIGNAL(signalFilterFinished(int,QPixmap)),
            this, SLOT(slotFilterFinished(int,QPixmap)));
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
    if (!filter)
    {
        return 0;
    }

    d->wrapper->registerFilter(id, filter);

    PreviewListItem* item = new PreviewListItem(this);
    item->setText(txt);
    //  in case text is mangled by textelide, it is displayed by hovering.
    item->setToolTip(txt);
    item->setId(id);
    return item;
}

PreviewListItem* PreviewList::findItem(int id) const
{
    int it = 0;

    while (it <= this->count())
    {
        PreviewListItem* item = dynamic_cast<PreviewListItem*>(this->item(it));

        if (item && item->id() == id)
        {
            return item;
        }

        ++it;
    }

    return 0;
}

void PreviewList::setCurrentId(int id)
{
    int it = 0;

    while (it <= this->count())
    {

        PreviewListItem* item = dynamic_cast<PreviewListItem*>(this->item(it));

        if (item && item->id() == id)
        {
            setCurrentItem(item);
            item->setSelected(true);
            return;
        }

        ++it;
    }
}

int PreviewList::currentId() const
{
    PreviewListItem* item = dynamic_cast<PreviewListItem*>(currentItem());

    if (item )
    {
        return item->id();
    }

    return 0;
}

void PreviewList::slotProgressTimerDone()
{
    QPixmap ppix(d->progressPix.frameAt(d->progressCount));
    QPixmap pixmap(128, 128);
    pixmap.fill(Qt::transparent);
    QPainter p(&pixmap);
    p.drawPixmap((pixmap.width()/2) - (ppix.width()/2), (pixmap.height()/2) - (ppix.height()/2), ppix);

    int busy                      = 0;
    int it                        = 0;
    PreviewListItem* selectedItem = 0;

    while (it <= this->count())
    {
        PreviewListItem* item = dynamic_cast<PreviewListItem*>(this->item(it));

        if (item && item->isSelected())
        {
            selectedItem = item;
        }

        if (item && item->isBusy())
        {
            item->setPixmap(pixmap);
            ++busy;
        }

        ++it;
    }

    d->progressCount++;

    if (d->progressCount >= d->progressPix.frameCount())
    {
        d->progressCount = 0;
    }

    if (!busy)
    {
        d->progressTimer->stop();
        // Qt 4.5 doesn't display icons correctly centred over i18n(text),
        // Qt 4.6 doesn't even reset the previous selection correctly.
        this->reset();

        if (selectedItem)
        {
            setCurrentItem(selectedItem);
        }
    }
}

void PreviewList::slotFilterStarted(int id)
{
    PreviewListItem* item = findItem(id);
    item->setBusy(true);
}

void PreviewList::slotFilterFinished(int id, const QPixmap& pix)
{
    PreviewListItem* item = findItem(id);

    if (item)
    {
        item->setBusy(false);
        item->setPixmap(pix);
    }
}

} // namespace Digikam
