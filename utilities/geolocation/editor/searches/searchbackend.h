/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2010-06-01
 * @brief  A simple backend to search OSM and Geonames.org.
 *
 * @author Copyright (C) 2010, 2011 by Michael G. Hansen
 *         <a href="mailto:mike at mghansen dot de">mike at mghansen dot de</a>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef SEARCHBACKEND_H
#define SEARCHBACKEND_H

// Qt includes

#include <QObject>
#include <QNetworkReply>

// Local includes

#include "geoiface_types.h"

// Local includes

#include "rgwidget.h"

namespace Digikam
{

class SearchBackend : public QObject
{
    Q_OBJECT

public:

    class SearchResult
    {
    public:

        SearchResult()
        {
        }

        typedef QList<SearchResult>    List;
        GeoIface::GeoCoordinates       coordinates;
        QString                        name;
        GeoIface::GeoCoordinates::Pair boundingBox;
        QString                        internalId;
    };

    explicit SearchBackend(QObject* const parent = 0);
    ~SearchBackend();

    bool search(const QString& backendName, const QString& searchTerm) const;
    SearchResult::List getResults() const;
    QString getErrorMessage() const;
    QList<QPair<QString, QString> >  getBackends() const;

Q_SIGNALS:

    void signalSearchCompleted();

private Q_SLOTS:

    void slotFinished(QNetworkReply* reply);

private:

    class Private;
    Private* const d;
};

} /* namespace Digikam */

#endif /* SEARCHBACKEND_H */
