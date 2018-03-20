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

#ifndef IMAGELISTERRECEIVER_H
#define IMAGELISTERRECEIVER_H

// Qt includes

#include <QString>
#include <QList>

// Local includes

#include "digikam_export.h"
#include "imagelisterrecord.h"
#include "dbjob.h"

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

class DIGIKAM_DATABASE_EXPORT ImageListerJobReceiver : public ImageListerValueListReceiver
{

public:

    explicit ImageListerJobReceiver(DBJob *const job);
    virtual void error(const QString& errMsg);
    void sendData();

protected:

    DBJob *const m_job;
};

// ------------------------------------------------------------------------------------------------

class DIGIKAM_DATABASE_EXPORT ImageListerJobPartsSendingReceiver : public ImageListerJobReceiver
{

public:

    ImageListerJobPartsSendingReceiver(DBJob *const job, int limit);
    virtual void receive(const ImageListerRecord &record);

protected:

    int m_limit;
    int m_count;
};

// ------------------------------------------------------------------------------------------------

class DIGIKAM_DATABASE_EXPORT ImageListerJobGrowingPartsSendingReceiver
    : public ImageListerJobPartsSendingReceiver
{

public:

    ImageListerJobGrowingPartsSendingReceiver(DBJob* job, int start, int end, int increment);
    virtual void receive(const ImageListerRecord& record);

protected:

    int m_maxLimit;
    int m_increment;
};

}  // namespace Digikam

#endif  // IMAGELISTERRECEIVER_H
