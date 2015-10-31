/* ============================================================
 *
 * This file is a part of digiKam
 *
 * Date        : 02.02.2012
 *
 * Copyright (C) 2012-2013 by Marcel Wiesweg <marcel dot wiesweg at uk-essen dot de>
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

#ifndef TRAININGDB_H
#define TRAININGDB_H

// Qt includes

#include <QString>

// Local includes

#include "identity.h"
#include "databasefacecorebackend.h"

using namespace Digikam;

namespace FacesEngine
{

class LBPHFaceModel;

class TrainingDB
{
public:

    TrainingDB(DatabaseFaceCoreBackend* const db);
    ~TrainingDB();

    void setSetting(const QString& keyword, const QString& value);
    QString setting(const QString& keyword) const;

    int  addIdentity() const;
    void updateIdentity(const Identity& p);
    void deleteIdentity(int id);
    QList<Identity> identities()  const;
    QList<int>      identityIds() const;

    /// OpenCV LBPH

    void updateLBPHFaceModel(LBPHFaceModel& model);
    LBPHFaceModel lbphFaceModel() const;
    void clearLBPHTraining(const QString& context = QString());
    void clearLBPHTraining(const QList<int>& identities, const QString& context = QString());

private:

    class Private;
    Private* const d;
};

} // namespace FacesEngine

#endif // PATIENTDB_H
