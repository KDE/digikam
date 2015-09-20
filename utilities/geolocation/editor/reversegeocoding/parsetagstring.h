/** ===========================================================
 * @file
 *
 * This file is a part of kipi-plugins project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2010-05-12
 * @brief  Parses retrieved data into tag string.
 *
 * @author Copyright (C) 2010 by Michael G. Hansen
 *         <a href="mailto:mike at mghansen dot de">mike at mghansen dot de</a>
 * @author Copyright (C) 2010 by Gabriel Voicu
 *         <a href="mailto:ping dot gabi at gmail dot com">ping dot gabi at gmail dot com</a>
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

#ifndef PARSETAGSTRING_H
#define PARSETAGSTRING_H

// local includes

#include "digikam_debug.h"
#include "backend-rg.h"

namespace Digikam
{

QStringList makeTagString(const RGInfo& info,const QString& inputFormat,const QString& backendName)
{
    QString auxReturnedFormat = inputFormat;
    QString returnedAddress   = inputFormat;
    QString returnedFormat;

    QStringList returnedAddressElements;

    int indexFBracket = -1;

    while ((indexFBracket = returnedAddress.indexOf(QStringLiteral("{"))) >= 0)
    {
        int indexLBracket       = returnedAddress.indexOf(QStringLiteral("}"));
        QString humanTag        = returnedAddress.mid(indexFBracket + 1, indexLBracket-indexFBracket-1);
        int indexFormatFBracket = auxReturnedFormat.indexOf(QStringLiteral("{"));
        auxReturnedFormat.replace(indexFormatFBracket - 1, humanTag.length() + 3, QStringLiteral(""));
        bool dataAdded          = false;
        QString result;

        if (backendName == QStringLiteral("OSM"))
        {
            if (humanTag == QStringLiteral("Country"))
            {
                if (!info.rgData[QStringLiteral("country")].isEmpty())
                {
                    result    = info.rgData[QStringLiteral("country")];
                    returnedFormat.append(QStringLiteral("/{Country}"));
                    dataAdded = true;
                }
            }
            else if (humanTag == QStringLiteral("State district"))
            {
                if (!info.rgData[QStringLiteral("state_district")].isEmpty())
                {
                    result    = info.rgData[QStringLiteral("state_district")];
                    returnedFormat.append(QStringLiteral("/{State district}"));
                    dataAdded = true;
                }
            }
            else if (humanTag == QStringLiteral("County"))
            {
                if (!info.rgData[QStringLiteral("county")].isEmpty())
                {
                    result    = info.rgData[QStringLiteral("county")];
                    returnedFormat.append(QStringLiteral("/{County}"));
                    dataAdded = true;
                }
            }
            else if (humanTag == QStringLiteral("City"))
            {
                if (!info.rgData[QStringLiteral("city")].isEmpty())
                {
                    result    = info.rgData[QStringLiteral("city")];
                    returnedFormat.append(QStringLiteral("/{City}"));
                    dataAdded = true;
                }
            }
            else if (humanTag == QStringLiteral("City district"))
            {
                if (!info.rgData[QStringLiteral("city_district")].isEmpty())
                {
                    result    = info.rgData[QStringLiteral("city_district")];
                    returnedFormat.append(QStringLiteral("/{City district}"));
                    dataAdded = true;
                }
            }
            else if (humanTag == QStringLiteral("Suburb"))
            {
                if (!info.rgData[QStringLiteral("suburb")].isEmpty())
                {
                    result    = info.rgData[QStringLiteral("suburb")];
                    returnedFormat.append(QStringLiteral("/{Suburb}"));
                    dataAdded = true;
                }
            }
            else if (humanTag == QStringLiteral("Street"))
            {
                if (!info.rgData[QStringLiteral("road")].isEmpty())
                {
                    result    = info.rgData[QStringLiteral("road")];
                    returnedFormat.append(QStringLiteral("/{Street}"));
                    dataAdded = true;
                }
            }
            else if (humanTag == QStringLiteral("State"))
            {
                if (!info.rgData[QStringLiteral("state")].isEmpty())
                {
                    result    = info.rgData[QStringLiteral("state")];
                    returnedFormat.append(QStringLiteral("/{State}"));
                    dataAdded = true;
                }
            }
            else if (humanTag == QStringLiteral("Town"))
            {
                if (!info.rgData[QStringLiteral("town")].isEmpty())
                {
                    result    = info.rgData[QStringLiteral("town")];
                    returnedFormat.append(QStringLiteral("/{Town}"));
                    dataAdded = true;
                }
            }
            else if (humanTag == QStringLiteral("Village"))
            {
                if (!info.rgData[QStringLiteral("village")].isEmpty())
                {
                    result    = info.rgData[QStringLiteral("village")];
                    returnedFormat.append(QStringLiteral("/{Village}"));
                    dataAdded = true;
                }
            }
            else if (humanTag == QStringLiteral("Hamlet"))
            {
                if (!info.rgData[QStringLiteral("hamlet")].isEmpty())
                {
                    result    = info.rgData[QStringLiteral("hamlet")];
                    returnedFormat.append(QStringLiteral("/{Hamlet}"));
                    dataAdded = true;
                }
            }
            else if (humanTag == QStringLiteral("House number"))
            {
                if (!info.rgData[QStringLiteral("house_number")].isEmpty())
                {
                    result    = info.rgData[QStringLiteral("house_number")];
                    returnedFormat.append(QStringLiteral("/{House number}"));
                    dataAdded = true;
                }
            }
            else
            {
                returnedAddress.replace(indexFBracket - 1, indexLBracket - indexFBracket + 2, QStringLiteral(""));

                int indexFormatFBracket = auxReturnedFormat.indexOf(QStringLiteral("{"));
                int indexFormatLBracket = auxReturnedFormat.indexOf(QStringLiteral("}"));
                auxReturnedFormat.replace(indexFormatFBracket - 1,
                                          indexFormatLBracket - indexFormatFBracket + 2,
                                          QStringLiteral(""));
                dataAdded               = true;
            }
        }

        else if (backendName == QStringLiteral("GeonamesUS"))
        {

            if (humanTag.compare(QStringLiteral("LAU2")) == 0)
            {
                if (!info.rgData[QStringLiteral("adminName2")].isEmpty())
                {
                    result    = info.rgData[QStringLiteral("adminName2")];
                    returnedFormat.append(QStringLiteral("/{LAU2}"));
                    dataAdded = true;
                }
            }

           else if (humanTag == QStringLiteral("LAU1"))
            {
                if (!info.rgData[QStringLiteral("adminName1")].isEmpty())
                {
                    result    = info.rgData[QStringLiteral("adminName1")];
                    returnedFormat.append(QStringLiteral("/{LAU1}"));
                    dataAdded = true;
                }
            }

            else if (humanTag == QStringLiteral("City"))
            {
                if (!info.rgData[QStringLiteral("placeName")].isEmpty())
                {
                    result    = info.rgData[QStringLiteral("placeName")];
                    returnedFormat.append(QStringLiteral("/{City}"));
                    dataAdded = true;
                }
            }
            else
            {
                returnedAddress.replace(indexFBracket - 1,
                                        indexLBracket - indexFBracket + 2,
                                        QStringLiteral(""));

                int indexFormatFBracket = auxReturnedFormat.indexOf(QStringLiteral("{"));
                int indexFormatLBracket = auxReturnedFormat.indexOf(QStringLiteral("}"));
                auxReturnedFormat.replace(indexFormatFBracket - 1,
                                          indexFormatLBracket - indexFormatFBracket + 2,
                                          QStringLiteral(""));
                dataAdded               = true;
            }
        }

        else if (backendName == QStringLiteral("Geonames"))
        {
            if (humanTag.compare(QStringLiteral("Country")) == 0)
            {
                if (!info.rgData[QStringLiteral("countryName")].isEmpty())
                {
                    result    = info.rgData[QStringLiteral("countryName")];
                    returnedFormat.append(QStringLiteral("/{Country}"));
                    dataAdded = true;
                }
            }

            else if (humanTag == QStringLiteral("Place"))
            {
                if (!info.rgData[QStringLiteral("name")].isEmpty())
                {
                    result    = info.rgData[QStringLiteral("name")];
                    returnedFormat.append(QStringLiteral("/{Place}"));
                    dataAdded = true;
                }
            }
            else
            {
                returnedAddress.replace(indexFBracket - 1,
                                        indexLBracket - indexFBracket + 2,
                                        QStringLiteral(""));

                int indexFormatFBracket = auxReturnedFormat.indexOf(QStringLiteral("{"));
                int indexFormatLBracket = auxReturnedFormat.indexOf(QStringLiteral("}"));
                auxReturnedFormat.replace(indexFormatFBracket - 1,
                                          indexFormatLBracket - indexFormatFBracket + 2,
                                          QStringLiteral(""));
                dataAdded               = true;
            }
        }

        if (!dataAdded)
        {
            returnedAddress.replace(indexFBracket - 1, humanTag.length() + 3, QStringLiteral(""));
        }
        else
        {
            returnedAddress.replace(indexFBracket, humanTag.length() + 2, result);
        }
    }

    returnedAddressElements.append(returnedFormat);
    returnedAddressElements.append(returnedAddress);

    return returnedAddressElements;
}

} // namespace Digikam

#endif /* PARSETAGSTRING_H */
