/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-03-20
 * Description : Simple virtual interface for ImageLister
 *
 * Copyright (C) 2007-2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

// Qt includes.

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

ImageListerSlaveBaseReceiver::ImageListerSlaveBaseReceiver(KIO::SlaveBase *slave)
                            : m_slave(slave)
{
}

void ImageListerSlaveBaseReceiver::error(const QString& errMsg)
{
    m_slave->error(KIO::ERR_INTERNAL, errMsg);
    ImageListerValueListReceiver::error(errMsg);
}

void ImageListerSlaveBaseReceiver::sendData()
{
    QByteArray  ba;
    QDataStream os(&ba, QIODevice::WriteOnly);

    for (QList<ImageListerRecord>::const_iterator it = records.constBegin(); it != records.constEnd(); ++it)
    {
        os << *it;
    }

    m_slave->data(ba);

    records.clear();
}

ImageListerSlaveBasePartsSendingReceiver::ImageListerSlaveBasePartsSendingReceiver(KIO::SlaveBase *slave, int limit)
                                        : ImageListerSlaveBaseReceiver(slave), m_limit(limit), m_count(0)
{
}

void ImageListerSlaveBasePartsSendingReceiver::receive(const ImageListerRecord &record)
{
    ImageListerSlaveBaseReceiver::receive(record);
    if (++m_count > m_limit)
    {
        sendData();
        m_count = 0;
    }
}

}  // namespace Digikam
