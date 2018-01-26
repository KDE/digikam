/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-06-27
 * Description : Similarity database backend
 *
 * Copyright (C) 2007-2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2010-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C)      2017 by Swati  Lodha   <swatilodha27 at gmail dot com>
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

// Local includes

#include "similaritydbbackend.h"
#include "dbenginebackend_p.h"
#include "similaritydbschemaupdater.h"

namespace Digikam
{

SimilarityDbBackend::SimilarityDbBackend(DbEngineLocking* const locking, const QString& backendName)
    : BdEngineBackend(backendName, locking, *new BdEngineBackendPrivate(this))
{
}

SimilarityDbBackend::~SimilarityDbBackend()
{
}

bool SimilarityDbBackend::initSchema(SimilarityDbSchemaUpdater* const updater)
{
    Q_D(BdEngineBackend);

    if (d->status == OpenSchemaChecked)
    {
        return true;
    }

    if (d->status == Unavailable)
    {
        return false;
    }

    if (updater->update())
    {
        d->status = OpenSchemaChecked;
        return true;
    }

    return false;
}

}  // namespace Digikam
