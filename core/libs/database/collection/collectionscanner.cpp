/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-03-21
 * Description : Collection scanning to database.
 *
 * Copyright (C) 2005      by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2005-2006 by Tom Albers <tomalbers at kde dot nl>
 * Copyright (C) 2007-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2009-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "collectionscanner_p.h"

namespace Digikam
{

CollectionScanner::CollectionScanner()
    : d(new Private)
{
}

CollectionScanner::~CollectionScanner()
{
    delete d;
}

void CollectionScanner::setSignalsEnabled(bool on)
{
    d->wantSignals = on;
}

void CollectionScanner::setNeedFileCount(bool on)
{
    d->needTotalFiles = on;
}

CollectionScannerHintContainer* CollectionScanner::createHintContainer()
{
    return new CollectionScannerHintContainerImplementation;
}

void CollectionScanner::setHintContainer(CollectionScannerHintContainer* const container)
{
    // the API specs require the object given here to be created by createContainer, so we can cast.
    d->hints = static_cast<CollectionScannerHintContainerImplementation*>(container);
}

void CollectionScanner::setUpdateHashHint(bool hint)
{
    d->updatingHashHint = hint;
}

void CollectionScanner::setObserver(CollectionScannerObserver* const observer)
{
    d->observer = observer;
}

void CollectionScanner::setDeferredFileScanning(bool defer)
{
    d->deferredFileScanning = defer;
}

QStringList CollectionScanner::deferredAlbumPaths() const
{
    return d->deferredAlbumPaths.toList();
}

} // namespace Digikam
