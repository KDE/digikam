/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2006-09-19
 * @brief  Track file reader
 *
 * @author Copyright (C) 2006-2017 by Gilles Caulier
 *         <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a>
 * @author Copyright (C) 2010-2014 by Michael G. Hansen
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

#ifndef TRACK_READER_H
#define TRACK_READER_H

// Qt includes

#include <QXmlDefaultHandler>

// local includes

#include "tracks.h"
#include "digikam_export.h"

class TestTracks;

namespace GeoIface
{

class DIGIKAM_EXPORT TrackReader : public QXmlDefaultHandler
{
public:

    class TrackReadResult
    {
    public:

        TrackReadResult()
          : track(),
            isValid(false),
            loadError()
        {
        }

        TrackManager::Track            track;
        bool                           isValid;
        QString                        loadError;

        typedef QList<TrackReadResult> List;
    };

    explicit TrackReader(TrackReadResult* const dataTarget);
    virtual ~TrackReader();

    virtual bool characters(const QString& ch);
    virtual bool endElement(const QString& namespaceURI, const QString& localName, const QString& qName);
    virtual bool startElement(const QString& namespaceURI, const QString& localName, const QString& qName, const QXmlAttributes& atts);

    static TrackReadResult loadTrackFile(const QUrl& url);
    static QDateTime ParseTime(QString timeString);

private:

    void rebuildElementPath();

    static QString myQName(const QString& namespaceURI, const QString& localName);

private:

    class Private;
    const QScopedPointer<Private> d;

    friend class ::TestTracks;
};

} // namespace GeoIface

#endif // TRACK_READER_H
