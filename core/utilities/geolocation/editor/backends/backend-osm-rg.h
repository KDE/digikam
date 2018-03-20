/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-05-12
 * Description : OSM Nominatim backend for Reverse Geocoding
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

#ifndef BACKEND_OSM_RG_H
#define BACKEND_OSM_RG_H

// Qt includes

#include <QNetworkReply>
#include <QList>
#include <QUrl>
#include <QMap>

// Local includes

#include "backend-rg.h"

namespace Digikam
{

class BackendOsmRG : public RGBackend
{
    Q_OBJECT

public:

    explicit BackendOsmRG(QObject* const parent);
    virtual ~BackendOsmRG();

    QMap<QString,QString> makeQMapFromXML(const QString& xmlData);

    virtual void callRGBackend(const QList<RGInfo>& rgList,const QString& language);
    virtual QString getErrorMessage();
    virtual QString backendName();
    virtual void cancelRequests();

private Q_SLOTS:

    void nextPhoto();
    void slotFinished(QNetworkReply* reply);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // BACKEND_OSM_RG_H
