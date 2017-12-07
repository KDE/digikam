/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-02-13
 * Description : a list of selectable options with preview
 *               effects as thumbnails.
 *
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "previewlist.h"

// Qt includes

#include <QPixmap>
#include <QTimer>
#include <QPainter>
#include <QMap>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dimg.h"
#include "dimgthreadedfilter.h"
#include "imageiface.h"
#include "digikam_debug.h"
#include "dlayoutbox.h"
#include "dworkingpixmap.h"

namespace Digikam
{

class PreviewThreadWrapper::Private
{

public:

    Private()
    {
    }

    QMap<int, DImgThreadedFilter*> map;
};

PreviewThreadWrapper::PreviewThreadWrapper(QObject* const parent)
    : QObject(parent),
      d(new Private)
{
}

PreviewThreadWrapper::~PreviewThreadWrapper()
{
    qDeleteAll(d->map.values());
    d->map.clear();

    delete d;
}

void PreviewThreadWrapper::registerFilter(int id, DImgThreadedFilter* const filter)
{
    if(d->map.contains(id))
        return;

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
    DImgThreadedFilter* const filter = dynamic_cast<DImgThreadedFilter*>(sender());

    if (!filter)
    {
        return;
    }

    emit signalFilterStarted(d->map.key(filter));
}

void PreviewThreadWrapper::slotFilterFinished(bool success)
{
    DImgThreadedFilter* const filter = dynamic_cast<DImgThreadedFilter*>(sender());

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
    DImgThreadedFilter* const filter = dynamic_cast<DImgThreadedFilter*>(sender());

    if (!filter)
    {
        return;
    }

    //qCDebug(DIGIKAM_GENERAL_LOG) << filter->filterName() << " : " << progress << " %";
}

void PreviewThreadWrapper::startFilters()
{
    foreach(DImgThreadedFilter* const filter, d->map)
    {
        filter->startFilter();
    }
}

void PreviewThreadWrapper::stopFilters()
{
    foreach(DImgThreadedFilter* const filter, d->map)
    {
        filter->cancelFilter();
        filter->deleteLater();
    }
}

// ---------------------------------------------------------------------

class PreviewListItem::Private
{

public:

    Private() :
        busy(false),
        id(0)
    {
    }

    bool busy;
    int  id;
};

PreviewListItem::PreviewListItem(QListWidget* const parent)
    : QListWidgetItem(parent),
      d(new Private)
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
    icon.addPixmap(pix, QIcon::Active,   QIcon::On);
    icon.addPixmap(pix, QIcon::Active,   QIcon::Off);
    icon.addPixmap(pix, QIcon::Normal,   QIcon::On);
    icon.addPixmap(pix, QIcon::Normal,   QIcon::Off);
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

class PreviewList::Private
{

public:

    Private() :
        progressCount(0),
        progressTimer(0),
        wrapper(0)
    {
        progressPix = DWorkingPixmap();
    }

    int                        progressCount;

    QTimer*                    progressTimer;

    DWorkingPixmap progressPix;

    PreviewThreadWrapper*      wrapper;
};

PreviewList::PreviewList(QObject* const /*parent*/)
    : QListWidget(),
      d(new Private)
{
    d->wrapper = new PreviewThreadWrapper(this);

    setSelectionMode(QAbstractItemView::SingleSelection);
    setDropIndicatorShown(true);
    setSortingEnabled(false);
    setIconSize(QSize(96, 96));
    setViewMode(QListView::IconMode);
    setWrapping(true);
    setWordWrap(false);
    setMovement(QListView::Static);
    setSpacing(5);
    setGridSize(QSize(125, 100 + fontMetrics().height()));
    setResizeMode(QListView::Adjust);
    setTextElideMode(Qt::ElideRight);
    setCursor(Qt::PointingHandCursor);
    setStyleSheet(QLatin1String("QListWidget::item:selected:!active {show-decoration-selected: 0}"));

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

PreviewListItem* PreviewList::addItem(DImgThreadedFilter* const filter, const QString& txt, int id)
{
    if (!filter)
    {
        return 0;
    }

    d->wrapper->registerFilter(id, filter);

    PreviewListItem* const item = new PreviewListItem(this);
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
        PreviewListItem* const item = dynamic_cast<PreviewListItem*>(this->item(it));

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

        PreviewListItem* const item = dynamic_cast<PreviewListItem*>(this->item(it));

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
    PreviewListItem* const item = dynamic_cast<PreviewListItem*>(currentItem());

    if (item)
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
    p.drawPixmap((pixmap.width() / 2) - (ppix.width() / 2), (pixmap.height() / 2) - (ppix.height() / 2), ppix);

    int busy                      = 0;
    int it                        = 0;
    PreviewListItem* selectedItem = 0;

    while (it <= this->count())
    {
        PreviewListItem* const item = dynamic_cast<PreviewListItem*>(this->item(it));

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
    PreviewListItem* const item = findItem(id);
    item->setBusy(true);
}

void PreviewList::slotFilterFinished(int id, const QPixmap& pix)
{
    PreviewListItem* const item = findItem(id);

    if (item)
    {
        item->setBusy(false);
        item->setPixmap(pix);
        update();
    }
}

} // namespace Digikam
