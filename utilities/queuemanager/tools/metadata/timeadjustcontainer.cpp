/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-04-19
 * Description : time adjust settings container.
 *
 * Copyright (C) 2012-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "timeadjustcontainer.h"

namespace Digikam
{

TimeAdjustContainer::TimeAdjustContainer()
{
    customDate     = QDateTime::currentDateTime();
    customTime     = QDateTime::currentDateTime();
    adjustmentTime = QDateTime();

    updEXIFModDate = false;
    updEXIFOriDate = false;
    updEXIFDigDate = false;
    updEXIFThmDate = false;
    updIPTCDate    = false;
    updXMPDate     = false;
    updFileModDate = false;

    dateSource     = APPDATE;
    metadataSource = EXIFIPTCXMP;
    fileDateSource = FILELASTMOD;
    adjustmentType = COPYVALUE;
    adjustmentDays = 0;
}

TimeAdjustContainer::~TimeAdjustContainer()
{
}

bool TimeAdjustContainer::atLeastOneUpdateToProcess() const
{
    return (updFileModDate ||
            updEXIFModDate ||
            updEXIFOriDate ||
            updEXIFDigDate ||
            updEXIFThmDate ||
            updIPTCDate    ||
            updXMPDate);
}

QDateTime TimeAdjustContainer::calculateAdjustedDate(const QDateTime& originalTime) const
{
    int sign = 0;

    switch (adjustmentType)
    {
        case ADDVALUE:
            sign = 1;
            break;
        case SUBVALUE:
            sign = -1;
            break;
        default: // COPYVALUE
            return originalTime;
    }

    int seconds  = adjustmentTime.time().second();
    seconds     += 60*adjustmentTime.time().minute();
    seconds     += 60*60*adjustmentTime.time().hour();
    seconds     += 24*60*60*adjustmentDays;

    return originalTime.addSecs(sign * seconds);
}

// -------------------------------------------------------------------

DeltaTime::DeltaTime()
{
    deltaNegative = false;
    deltaDays     = 0;
    deltaHours    = 0;
    deltaMinutes  = 0;
    deltaSeconds  = 0;
}

DeltaTime::~DeltaTime()
{
}

bool DeltaTime::isNull() const
{
    return (deltaDays    == 0 &&
            deltaHours   == 0 &&
            deltaMinutes == 0 &&
            deltaSeconds == 0);
}

}  // namespace Digikam
