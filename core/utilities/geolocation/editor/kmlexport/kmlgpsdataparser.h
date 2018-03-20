/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-05-16
 * Description : a tool to export GPS data to KML file.
 *
 * Copyright (C) 2006-2007 by Stephane Pontier <shadow dot walker at free dot fr>
 * Copyright (C) 2008-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef KML_GPS_DATA_PARSER_H
#define KML_GPS_DATA_PARSER_H

// Local includes

#include "geodataparser.h"

// Qt includes

#include <QDomDocument>

namespace Digikam
{

/*! a classe derivated from GeoDataParser mainly to transform GPS data to KML
 *  @author Stéphane Pontier shadow.walker@free.fr
 */
class KMLGeoDataParser : public GeoDataParser
{

public:

    KMLGeoDataParser();
    ~KMLGeoDataParser();

    /*! KMLGeoDataParser::KMLGeoDataParser::lineString()
     *  @return the string containing the time ordered point (lon,lat,alt)
     */
    QString lineString();

    /*! Create a KML Element that will contain the linetrace of the GPS
     *  @param parent the QDomElement to which the track will be added
     *  @param root the QDomDocument used to create all elements
     *  @param altitudeMode altitude mode of the line and points
     */
    void CreateTrackLine(QDomElement& parent, QDomDocument& root, int altitudeMode);

    /*! Create a KML Element that will contain the points and of the GPS
     *  @param parent the QDomElement to which the track will be added
     *  @param root the QDomDocument used to create all elements
     *  @param timeZone the Timezone of the pictures
     *  @param altitudeMode altitude mode of the line and points
     */
    void CreateTrackPoints(QDomElement& parent, QDomDocument& root, int timeZone, int altitudeMode);

private:

    /*!
     *  @brief Add a new element
     *  @param target the parent element to which add the element
     *  @param tag the new element name
     *  @return the New element
     */
    QDomElement addKmlElement(QDomElement& target, const QString& tag)
    {
        QDomElement kmlElement = kmlDocument->createElement( tag );
        target.appendChild( kmlElement );
        return kmlElement;
    }

    /**
     *  @brief Add a new element with a text
     *  @param target the parent element to which add the element
     *  @param tag the new element name
     *  @param text the text content of the new element
     *  @return the New element
     */
    QDomElement addKmlTextElement(QDomElement& target, const QString& tag, const QString& text)
    {
        QDomElement kmlElement  = kmlDocument->createElement( tag );
        target.appendChild( kmlElement );
        QDomText kmlTextElement = kmlDocument->createTextNode( text );
        kmlElement.appendChild( kmlTextElement );
        return kmlElement;
    }

private:

    /*! @todo maybe initialize it in the constructor */
    /*! the root document, used to create all QDomElements */
    QDomDocument* kmlDocument;
};

} // namespace Digikam

#endif // KML_GPS_DATA_PARSER_H
