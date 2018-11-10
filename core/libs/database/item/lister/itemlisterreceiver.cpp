/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-03-20
 * Description : Simple virtual interface for ItemLister
 *
 * Copyright (C) 2007-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2007-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2015      by Mohamed_Anwer <m_dot_anwer at gmx dot com>
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

#include "itemlisterreceiver.h"

// Qt includes

#include <QList>

namespace Digikam
{

ItemListerValueListReceiver::ItemListerValueListReceiver()
    : hasError(false)
{
}

void ItemListerValueListReceiver::error(const QString&)
{
    hasError = true;
}

void ItemListerValueListReceiver::receive(const ItemListerRecord& record)
{
    records << record;
}

// ----------------------------------------------

ItemListerJobReceiver::ItemListerJobReceiver(DBJob* const job)
    : m_job(job)
{
}

void ItemListerJobReceiver::sendData()
{
    emit m_job->data(records);

    records.clear();
}

void ItemListerJobReceiver::error(const QString& errMsg)
{
    m_job->error(errMsg);
    ItemListerValueListReceiver::error(errMsg);
}

// ----------------------------------------------

ItemListerJobPartsSendingReceiver::ItemListerJobPartsSendingReceiver(DBJob* const job, int limit)
    : ItemListerJobReceiver(job), m_limit(limit), m_count(0)
{

}

void ItemListerJobPartsSendingReceiver::receive(const ItemListerRecord &record)
{

    ItemListerJobReceiver::receive(record);

    if (++m_count > m_limit)
    {
        sendData();
        m_count = 0;
    }
}

// ----------------------------------------------

ItemListerJobGrowingPartsSendingReceiver::
    ItemListerJobGrowingPartsSendingReceiver(DBJob* const job, int start, int end, int increment)
    : ItemListerJobPartsSendingReceiver(job, start),
      m_maxLimit(end), m_increment(increment)
{
}

void ItemListerJobGrowingPartsSendingReceiver::receive(const ItemListerRecord& record)
{
    ItemListerJobPartsSendingReceiver::receive(record);
    // limit was reached?
    if (m_count == 0)
    {
        m_limit = qMin(m_limit + m_increment, m_maxLimit);
    }
}

} // namespace Digikam
