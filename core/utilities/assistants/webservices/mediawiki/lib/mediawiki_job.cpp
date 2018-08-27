/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-03-22
 * Description : a Iface C++ interface
 *
 * Copyright (C) 2011-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2011      by Paolo de Vathaire <paolo dot devathaire at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "mediawiki_job.h"

// Qt includes

#include <QNetworkReply>

// Local include

#include "mediawiki_iface.h"
#include "mediawiki_job_p.h"

namespace MediaWiki
{

Job::Job(JobPrivate& dd, QObject* const parent)
    : KJob(parent),
      d_ptr(&dd)
{
    setCapabilities(Job::Killable);
}

Job::~Job()
{
    delete d_ptr;
}

bool Job::doKill()
{
    Q_D(Job);

    if (d->reply != 0)
    {
        d->reply->abort();
    }

    return true;
}

void Job::connectReply()
{
    Q_D(Job);

    connect(d->reply, SIGNAL(uploadProgress(qint64,qint64)),
            this, SLOT(processUploadProgress(qint64,qint64)));
}

void Job::processUploadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    setTotalAmount(Job::Bytes, bytesTotal);
    setProcessedAmount(Job::Bytes, bytesReceived);
}

} // namespace MediaWiki
