/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-05-12
 * Description : Backend for reverse geocoding using geonames.org (US-only)
 *
 * Copyright (C) 2010 by Michael G. Hansen <mike at mghansen dot de>
 * Copyright (C) 2010 by Gabriel Voicu <ping dot gabi at gmail dot com>
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

#ifndef DIGIKAM_BACKEND_GEONAMESUS_RG_H
#define DIGIKAM_BACKEND_GEONAMESUS_RG_H

// Qt includes

#include <QNetworkReply>
#include <QString>
#include <QList>
#include <QUrl>
#include <QMap>

// Local includes

#include "digikam_export.h"
#include "backend-rg.h"

namespace Digikam
{

class DIGIKAM_EXPORT BackendGeonamesUSRG : public RGBackend
{
    Q_OBJECT

public:

    explicit BackendGeonamesUSRG(QObject* const parent);
    virtual ~BackendGeonamesUSRG();

    QMap<QString, QString> makeQMapFromXML(const QString& xmlData);

    virtual void callRGBackend(const QList<RGInfo>& rgList, const QString& language) override;
    virtual QString getErrorMessage() override;
    virtual QString backendName() override;
    virtual void cancelRequests() override;

private Q_SLOTS:

    void nextPhoto();
    void slotFinished(QNetworkReply* reply);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_BACKEND_GEONAMESUS_RG_H
