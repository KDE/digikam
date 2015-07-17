/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-03-20
 * Description : Simple virtual interface for ImageLister
 *
 * Copyright (C) 2007-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2015      by Mohamed Anwer <m dot anwer at gmx dot com>
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

#include "imagelisterreceiver.h"

// Qt includes

#include <QList>

namespace Digikam
{

ImageListerValueListReceiver::ImageListerValueListReceiver()
    : hasError(false)
{
}

void ImageListerValueListReceiver::error(const QString&)
{
    hasError = true;
}

void ImageListerValueListReceiver::receive(const ImageListerRecord& record)
{
    records << record;
}

// ----------------------------------------------

ImageListerJobReceiver::ImageListerJobReceiver(DBJob *const job)
    : m_job(job)
{
}

void ImageListerJobReceiver::sendData()
{
    emit m_job->data(records);

    records.clear();
}

void ImageListerJobReceiver::error(const QString& errMsg)
{
    m_job->error(errMsg);
    ImageListerValueListReceiver::error(errMsg);
}

// ----------------------------------------------

ImageListerJobPartsSendingReceiver::ImageListerJobPartsSendingReceiver(DBJob *const job, int limit)
    : ImageListerJobReceiver(job), m_limit(limit), m_count(0)
{

}

void ImageListerJobPartsSendingReceiver::receive(const ImageListerRecord &record)
{

    ImageListerJobReceiver::receive(record);

    if (++m_count > m_limit)
    {
        sendData();
        m_count = 0;
    }
}

// ----------------------------------------------

ImageListerJobGrowingPartsSendingReceiver::
    ImageListerJobGrowingPartsSendingReceiver(DBJob* job, int start, int end, int increment)
    : ImageListerJobPartsSendingReceiver(job, start),
      m_maxLimit(end), m_increment(increment)
{
}

void ImageListerJobGrowingPartsSendingReceiver::receive(const ImageListerRecord& record)
{
    ImageListerJobPartsSendingReceiver::receive(record);
    // limit was reached?
    if (m_count == 0)
    {
        m_limit = qMin(m_limit + m_increment, m_maxLimit);
    }
}

}  // namespace Digikam
