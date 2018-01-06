/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-01-26
 * Description : a progress bar with information dispatched to progress manager
 *
 * Copyright (C) 2012-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "dprogresswdg.h"

// Qt includes

#include <QString>
#include <QIcon>

// Local includes

#include "progressmanager.h"

namespace Digikam
{

class DProgressWdg::Private
{
public:

    Private()
    {
    }

    ProgressItem* findProgressItem() const
    {
        return ProgressManager::instance()->findItembyId(progressId);
    }

    QString    progressId;
};

DProgressWdg::DProgressWdg(QWidget* const parent)
    : QProgressBar(parent),
      d(new Private)
{
    connect(this, &DProgressWdg::valueChanged,
            this, &DProgressWdg::slotValueChanged);
}

DProgressWdg::~DProgressWdg()
{
    delete d;
}

void DProgressWdg::slotValueChanged(int)
{
    float percents           = ((float)value() / (float)maximum()) * 100.0;
    ProgressItem* const item = d->findProgressItem();

    if (item)
    {
        item->setProgress(percents);
    }
}

void DProgressWdg::progressCompleted()
{
    ProgressItem* const item = d->findProgressItem();

    if (item)
    {
        item->setComplete();
    }
}

void DProgressWdg::progressThumbnailChanged(const QPixmap& thumb)
{
    ProgressItem* const item = d->findProgressItem();

    if (item)
    {
        item->setThumbnail(thumb);
    }
}

void DProgressWdg::progressStatusChanged(const QString& status)
{
    ProgressItem* const item = d->findProgressItem();

    if (item)
    {
        item->setStatus(status);
    }
}

void DProgressWdg::progressScheduled(const QString& title, bool canBeCanceled, bool hasThumb)
{
    ProgressItem* const item = ProgressManager::createProgressItem(title,
                                                                   QString(),
                                                                   canBeCanceled,
                                                                   hasThumb);

    if (canBeCanceled)
    {
        connect(item, SIGNAL(progressItemCanceled(QString)),
                this, SLOT(slotProgressCanceled(QString)));
    }

    d->progressId = item->id();
}

void DProgressWdg::slotProgressCanceled(const QString& id)
{
    if (d->progressId == id)
    {
        emit signalProgressCanceled();
    }
}

} // namespace Digikam
