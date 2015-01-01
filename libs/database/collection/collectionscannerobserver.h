/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-04-16
 * Description : Schema update
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

#ifndef COLLECTIONSCANNEROBSERVER_H
#define COLLECTIONSCANNEROBSERVER_H

// Qt includes

#include <QString>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class CollectionScanner;

class DIGIKAM_DATABASE_EXPORT CollectionScannerObserver
{
public:

    virtual ~CollectionScannerObserver()
    {
    }

    virtual bool continueQuery() = 0;
};

// ------------------------------------------------------------------------------------------

class DIGIKAM_DATABASE_EXPORT InitializationObserver : public CollectionScannerObserver
{
public:

    enum UpdateResult
    {
        UpdateSuccess,
        UpdateError,
        UpdateErrorMustAbort
    };

public:

    virtual ~InitializationObserver()
    {
    };

    virtual void moreSchemaUpdateSteps(int numberOfSteps) = 0;
    virtual void schemaUpdateProgress(const QString& message, int numberOfSteps = 1) = 0;
    virtual void finishedSchemaUpdate(UpdateResult result) = 0;
    virtual void connectCollectionScanner(CollectionScanner* const scanner) = 0;
    virtual void error(const QString& errorMessage) = 0;
};

}  // namespace Digikam

#endif // COLLECTIONSCANNEROBSERVER_H
