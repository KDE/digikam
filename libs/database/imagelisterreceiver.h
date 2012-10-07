/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-03-20
 * Description : Simple virtual interface for ImageLister
 *
 * Copyright (C) 2005 by Renchi Raju <renchi dot raju at gmail dot com>
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

#ifndef IMAGELISTERRECEIVER_H
#define IMAGELISTERRECEIVER_H

// Qt includes

#include <QString>
#include <QList>

// KDE includes

#include <kio/slavebase.h>

// Local includes

#include "digikam_export.h"
#include "imagelisterrecord.h"

class QDataStream;

namespace Digikam
{

//TODO: Docs
class DIGIKAM_DATABASE_EXPORT ImageListerReceiver
{

public:

    virtual ~ImageListerReceiver() {};
    virtual void receive(const ImageListerRecord& record) = 0;
    virtual void error(const QString& /*errMsg*/) {};
};

class DIGIKAM_DATABASE_EXPORT ImageListerValueListReceiver
    : public ImageListerReceiver
{

public:

    ImageListerValueListReceiver();

    QList<ImageListerRecord> records;
    bool                     hasError;

    virtual void receive(const ImageListerRecord& record);
    virtual void error(const QString& errMsg);
};

// ------------------------------------------------------------------------------------------------

class DIGIKAM_DATABASE_EXPORT ImageListerSlaveBaseReceiver : public ImageListerValueListReceiver
{

public:

    explicit ImageListerSlaveBaseReceiver(KIO::SlaveBase* slave);
    virtual void error(const QString& errMsg);
    void sendData();

protected:

    KIO::SlaveBase* m_slave;
};

class DIGIKAM_DATABASE_EXPORT ImageListerSlaveBasePartsSendingReceiver
    : public ImageListerSlaveBaseReceiver
{

public:

    ImageListerSlaveBasePartsSendingReceiver(KIO::SlaveBase* slave, int limit);
    virtual void receive(const ImageListerRecord& record);

protected:

    int m_limit;
    int m_count;
};

// ------------------------------------------------------------------------------------------------

class DIGIKAM_DATABASE_EXPORT ImageListerSlaveBaseGrowingPartsSendingReceiver
    : public ImageListerSlaveBasePartsSendingReceiver
{

public:

    ImageListerSlaveBaseGrowingPartsSendingReceiver(KIO::SlaveBase* slave, int start, int end, int increment);
    virtual void receive(const ImageListerRecord& record);

protected:

    int m_maxLimit;
    int m_increment;
};

}  // namespace Digikam

#endif  // IMAGELISTERRECEIVER_H
