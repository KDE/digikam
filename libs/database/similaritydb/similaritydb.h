/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-07-07
 * Description : Similarity database interface.
 *
 * Copyright (C)      2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2009-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef SIMILARITY_DATABASE_H
#define SIMILARITY_DATABASE_H

// Qt includes

#include <QString>
#include <QList>
#include <QStringList>
#include <QDateTime>
#include <QPair>
#include <QMap>
#include <QHash>

// Local includes

#include "dbenginesqlquery.h"
#include "similaritydbbackend.h"
#include "similaritydbaccess.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT SimilarityDb
{

public:

    /**
     * Returns if there are valid entries in the ImageHaarMatrix table.
     * Returns false if the table is empty.
     */
    bool hasHaarFingerprints() const;
    
    /**
     * Returns a list of all images where the Haar fingerprint has either not been generated
     * yet, or is outdated because the file is identified as changed since
     * the generation of the fingerprint.
     * Return image ids or item URLs.
     */
    QList<qlonglong> getDirtyOrMissingFingerprints();
    QStringList getDirtyOrMissingFingerprintURLs();
    
    /**
     * Copies all similarity-specific information, from image srcId to destId.
     */
    void copySimilarityAttributes(qlonglong srcId, qlonglong destId);

    // ----------- Database shrinking methods ----------

    /**
     * Returns true if the integrity of the database is preserved.
     */
    bool integrityCheck();

    /**
     * Shrinks the database.
     */
    void vacuum();

private:

    explicit SimilarityDb(SimilarityDbBackend* const backend);
    ~SimilarityDb();

private:

    class Private;
    Private* const d;

    friend class SimilarityDbAccess;
};

}  // namespace Digikam

#endif /* SIMILARITY_DATABASE_H */
