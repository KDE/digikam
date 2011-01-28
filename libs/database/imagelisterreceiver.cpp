/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-03-20
 * Description : Simple virtual interface for ImageLister
 *
 * Copyright (C) 2007-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

QDataStream& operator<<(QDataStream& os, const ImageListerRecord& record)
{
    os << record.imageID;
    os << record.albumID;
    os << record.albumRootID;
    os << record.name;

    os << record.rating;
    os << (int)record.category;
    os << record.format;
    os << record.creationDate;
    os << record.modificationDate;
    os << record.fileSize;
    os << record.imageSize;

    if (record.binaryFormat == ImageListerRecord::ExtraValueFormat)
    {
        os << record.extraValues;
    }

    return os;
}

QDataStream& operator>>(QDataStream& ds, ImageListerRecord& record)
{
    int category;
    ds >> record.imageID;
    ds >> record.albumID;
    ds >> record.albumRootID;
    ds >> record.name;

    ds >> record.rating;
    ds >> category;
    record.category = (DatabaseItem::Category)category;
    ds >> record.format;
    ds >> record.creationDate;
    ds >> record.modificationDate;
    ds >> record.fileSize;
    ds >> record.imageSize;

    if (record.binaryFormat == ImageListerRecord::ExtraValueFormat)
    {
        ds >> record.extraValues;
    }

    return ds;
}

void ImageListerRecord::initializeStream(ImageListerRecord::BinaryFormat format, QDataStream& ds)
{
    switch (format)
    {
        case ImageListerRecord::TraditionalFormat:
            break;
        case ImageListerRecord::ExtraValueFormat:
            ds << (quint32)MagicValue;
            ds << (quint32)format;
            break;
    }
}

bool ImageListerRecord::checkStream(ImageListerRecord::BinaryFormat format, QDataStream& ds)
{
    switch (format)
    {
        case ImageListerRecord::TraditionalFormat:
            return true;
        case ImageListerRecord::ExtraValueFormat:
            quint32 magicValue   = 0;
            quint32 streamFormat = 0;
            ds >> magicValue;
            ds >> streamFormat;
            return (magicValue == MagicValue && streamFormat == (quint32)format);
    }
    return false;
}

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

ImageListerSlaveBaseReceiver::ImageListerSlaveBaseReceiver(KIO::SlaveBase* slave)
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

    if (!records.isEmpty())
    {
        ImageListerRecord::initializeStream(records.first().binaryFormat, os);
    }

    for (QList<ImageListerRecord>::const_iterator it = records.constBegin(); it != records.constEnd(); ++it)
    {
        os << *it;
    }

    m_slave->data(ba);

    records.clear();
}

ImageListerSlaveBasePartsSendingReceiver::ImageListerSlaveBasePartsSendingReceiver(KIO::SlaveBase* slave, int limit)
    : ImageListerSlaveBaseReceiver(slave), m_limit(limit), m_count(0)
{
}

void ImageListerSlaveBasePartsSendingReceiver::receive(const ImageListerRecord& record)
{
    ImageListerSlaveBaseReceiver::receive(record);

    if (++m_count > m_limit)
    {
        sendData();
        m_count = 0;
    }
}

ImageListerSlaveBaseGrowingPartsSendingReceiver::
    ImageListerSlaveBaseGrowingPartsSendingReceiver(KIO::SlaveBase* slave, int start, int end, int increment)
    : ImageListerSlaveBasePartsSendingReceiver(slave, start),
      m_maxLimit(end), m_increment(increment)
{
}

void ImageListerSlaveBaseGrowingPartsSendingReceiver::receive(const ImageListerRecord& record)
{
    ImageListerSlaveBasePartsSendingReceiver::receive(record);
    // limit was reached?
    if (m_count == 0)
    {
        m_limit = qMin(m_limit + m_increment, m_maxLimit);
    }
}

}  // namespace Digikam
