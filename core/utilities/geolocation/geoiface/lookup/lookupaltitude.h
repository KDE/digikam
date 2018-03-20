/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-04-30
 * Description : Base class for altitude lookup jobs
 *
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2010-2011 by Michael G. Hansen <mike at mghansen dot de>
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

#ifndef LOOKUP_ALTITUDE_H
#define LOOKUP_ALTITUDE_H

// Qt includes

#include <QObject>

// local includes

#include "digikam_export.h"
#include "geocoordinates.h"

namespace Digikam
{

class DIGIKAM_EXPORT LookupAltitude : public QObject
{
    Q_OBJECT

public:

    class  Request
    {
    public:

        Request()
            : coordinates(),
              success(false),
              data()
        {
        }

        GeoCoordinates         coordinates;
        bool                   success;
        QVariant               data;

        typedef QList<Request> List;
    };

public:

    enum StatusEnum
    {
        StatusInProgress = 0,
        StatusSuccess    = 1,
        StatusCanceled   = 2,
        StatusError      = 3
    };
    Q_DECLARE_FLAGS(StatusAltitude, StatusEnum)

public:

    explicit LookupAltitude(QObject* const parent);
    virtual ~LookupAltitude();

    virtual QString backendName() const = 0;
    virtual QString backendHumanName() const = 0;

    virtual void addRequests(const Request::List& requests) = 0;
    virtual Request::List getRequests() const = 0;
    virtual Request getRequest(const int index) const = 0;

    virtual void startLookup() = 0;
    virtual StatusAltitude getStatus() const = 0;
    virtual QString errorMessage() const = 0;
    virtual void cancel() = 0;

Q_SIGNALS:

    void signalRequestsReady(const QList<int>& readyRequests);
    void signalDone();
};

} // namespace Digikam

Q_DECLARE_OPERATORS_FOR_FLAGS(Digikam::LookupAltitude::StatusAltitude)

#endif // LOOKUP_ALTITUDE_H
