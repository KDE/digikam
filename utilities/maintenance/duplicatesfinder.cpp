/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-01-20
 * Description : Duplicates items finder.
 *
 * Copyright (C) 2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "duplicatesfinder.moc"

// KDE includes

#include <kapplication.h>
#include <klocale.h>
#include <kicon.h>

// Local includes

#include "databaseaccess.h"
#include "databasebackend.h"
#include "imagelister.h"
#include "knotificationwrapper.h"

namespace Digikam
{

DuplicatesFinder::DuplicatesFinder(const QStringList& albumsIdList, const QStringList& tagsIdList, int similarity)
    : ProgressItem(0, "DuplicatesFinder", QString(), QString(), true, true)
{
    ProgressManager::addProgressItem(this);

    m_duration.start();
    double thresh = similarity / 100.0;

    m_job = ImageLister::startListJob(DatabaseUrl::searchUrl(-1));
    m_job->addMetaData("duplicates", "normal");
    m_job->addMetaData("albumids",   albumsIdList.join(","));
    m_job->addMetaData("tagids",     tagsIdList.join(","));
    m_job->addMetaData("threshold",  QString::number(thresh));

    connect(m_job, SIGNAL(result(KJob*)),
            this, SLOT(slotDuplicatesSearchResult()));

    connect(m_job, SIGNAL(totalAmount(KJob*, KJob::Unit, qulonglong)),
            this, SLOT(slotDuplicatesSearchTotalAmount(KJob*, KJob::Unit, qulonglong)));

    connect(m_job, SIGNAL(processedAmount(KJob*, KJob::Unit, qulonglong)),
            this, SLOT(slotDuplicatesSearchProcessedAmount(KJob*, KJob::Unit, qulonglong)));

    connect(this, SIGNAL(progressItemCanceled(ProgressItem*)),
            this, SLOT(slotCancelButtonPressed()));

    setLabel(i18n("Find duplicates items"));
    setThumbnail(KIcon("tools-wizard").pixmap(22));
}

DuplicatesFinder::~DuplicatesFinder()
{
}

void DuplicatesFinder::slotDuplicatesSearchTotalAmount(KJob*, KJob::Unit, qulonglong amount)
{
    setTotalItems(amount);
}

void DuplicatesFinder::slotDuplicatesSearchProcessedAmount(KJob*, KJob::Unit, qulonglong amount)
{
    setCompletedItems(amount);
    updateProgress();
}

void DuplicatesFinder::slotDuplicatesSearchResult()
{
    QTime now, t = now.addMSecs(m_duration.elapsed());
    // Pop-up a message to bring user when all is done.
    KNotificationWrapper(id(),
                         i18n("Find duplicates is done.\nDuration: %1", t.toString()),
                         kapp->activeWindow(), label());
    m_job = NULL;

    emit signalComplete();

    setComplete();
}

void DuplicatesFinder::slotCancelButtonPressed()
{
    if (m_job)
    {
        m_job->kill();
        m_job = NULL;
    }

    emit signalComplete();

    setComplete();
}

}  // namespace Digikam
