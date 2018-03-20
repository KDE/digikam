/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-05-12
 * Description : Parses retrieved data into tag string.
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

    while ((indexFBracket = returnedAddress.indexOf(QLatin1String("{"))) >= 0)
    {
        int indexLBracket       = returnedAddress.indexOf(QLatin1String("}"));
        QString humanTag        = returnedAddress.mid(indexFBracket + 1, indexLBracket-indexFBracket-1);
        int indexFormatFBracket = auxReturnedFormat.indexOf(QLatin1String("{"));
        auxReturnedFormat.replace(indexFormatFBracket - 1, humanTag.length() + 3, QLatin1String(""));
        bool dataAdded          = false;
        QString result;

        if (backendName == QLatin1String("OSM"))
        {
            if (humanTag == QLatin1String("Country"))
            {
                if (!info.rgData[QLatin1String("country")].isEmpty())
                {
                    result    = info.rgData[QLatin1String("country")];
                    returnedFormat.append(QLatin1String("/{Country}"));
                    dataAdded = true;
                }
            }
            else if (humanTag == QLatin1String("State district"))
            {
                if (!info.rgData[QLatin1String("state_district")].isEmpty())
                {
                    result    = info.rgData[QLatin1String("state_district")];
                    returnedFormat.append(QLatin1String("/{State district}"));
                    dataAdded = true;
                }
            }
            else if (humanTag == QLatin1String("County"))
            {
                if (!info.rgData[QLatin1String("county")].isEmpty())
                {
                    result    = info.rgData[QLatin1String("county")];
                    returnedFormat.append(QLatin1String("/{County}"));
                    dataAdded = true;
                }
            }
            else if (humanTag == QLatin1String("City"))
            {
                if (!info.rgData[QLatin1String("city")].isEmpty())
                {
                    result    = info.rgData[QLatin1String("city")];
                    returnedFormat.append(QLatin1String("/{City}"));
                    dataAdded = true;
                }
            }
            else if (humanTag == QLatin1String("City district"))
            {
                if (!info.rgData[QLatin1String("city_district")].isEmpty())
                {
                    result    = info.rgData[QLatin1String("city_district")];
                    returnedFormat.append(QLatin1String("/{City district}"));
                    dataAdded = true;
                }
            }
            else if (humanTag == QLatin1String("Suburb"))
            {
                if (!info.rgData[QLatin1String("suburb")].isEmpty())
                {
                    result    = info.rgData[QLatin1String("suburb")];
                    returnedFormat.append(QLatin1String("/{Suburb}"));
                    dataAdded = true;
                }
            }
            else if (humanTag == QLatin1String("Street"))
            {
                if (!info.rgData[QLatin1String("road")].isEmpty())
                {
                    result    = info.rgData[QLatin1String("road")];
                    returnedFormat.append(QLatin1String("/{Street}"));
                    dataAdded = true;
                }
            }
            else if (humanTag == QLatin1String("State"))
            {
                if (!info.rgData[QLatin1String("state")].isEmpty())
                {
                    result    = info.rgData[QLatin1String("state")];
                    returnedFormat.append(QLatin1String("/{State}"));
                    dataAdded = true;
                }
            }
            else if (humanTag == QLatin1String("Town"))
            {
                if (!info.rgData[QLatin1String("town")].isEmpty())
                {
                    result    = info.rgData[QLatin1String("town")];
                    returnedFormat.append(QLatin1String("/{Town}"));
                    dataAdded = true;
                }
            }
            else if (humanTag == QLatin1String("Village"))
            {
                if (!info.rgData[QLatin1String("village")].isEmpty())
                {
                    result    = info.rgData[QLatin1String("village")];
                    returnedFormat.append(QLatin1String("/{Village}"));
                    dataAdded = true;
                }
            }
            else if (humanTag == QLatin1String("Hamlet"))
            {
                if (!info.rgData[QLatin1String("hamlet")].isEmpty())
                {
                    result    = info.rgData[QLatin1String("hamlet")];
                    returnedFormat.append(QLatin1String("/{Hamlet}"));
                    dataAdded = true;
                }
            }
            else if (humanTag == QLatin1String("House number"))
            {
                if (!info.rgData[QLatin1String("house_number")].isEmpty())
                {
                    result    = info.rgData[QLatin1String("house_number")];
                    returnedFormat.append(QLatin1String("/{House number}"));
                    dataAdded = true;
                }
            }
            else
            {
                returnedAddress.replace(indexFBracket - 1, indexLBracket - indexFBracket + 2, QLatin1String(""));

                int indexFormatFBracket = auxReturnedFormat.indexOf(QLatin1String("{"));
                int indexFormatLBracket = auxReturnedFormat.indexOf(QLatin1String("}"));
                auxReturnedFormat.replace(indexFormatFBracket - 1,
                                          indexFormatLBracket - indexFormatFBracket + 2,
                                          QLatin1String(""));
                dataAdded               = true;
            }
        }

        else if (backendName == QLatin1String("GeonamesUS"))
        {

            if (humanTag.compare(QLatin1String("LAU2")) == 0)
            {
                if (!info.rgData[QLatin1String("adminName2")].isEmpty())
                {
                    result    = info.rgData[QLatin1String("adminName2")];
                    returnedFormat.append(QLatin1String("/{LAU2}"));
                    dataAdded = true;
                }
            }

           else if (humanTag == QLatin1String("LAU1"))
            {
                if (!info.rgData[QLatin1String("adminName1")].isEmpty())
                {
                    result    = info.rgData[QLatin1String("adminName1")];
                    returnedFormat.append(QLatin1String("/{LAU1}"));
                    dataAdded = true;
                }
            }

            else if (humanTag == QLatin1String("City"))
            {
                if (!info.rgData[QLatin1String("placename")].isEmpty())
                {
                    result    = info.rgData[QLatin1String("placename")];
                    returnedFormat.append(QLatin1String("/{City}"));
                    dataAdded = true;
                }
            }
            else
            {
                returnedAddress.replace(indexFBracket - 1,
                                        indexLBracket - indexFBracket + 2,
                                        QLatin1String(""));

                int indexFormatFBracket = auxReturnedFormat.indexOf(QLatin1String("{"));
                int indexFormatLBracket = auxReturnedFormat.indexOf(QLatin1String("}"));
                auxReturnedFormat.replace(indexFormatFBracket - 1,
                                          indexFormatLBracket - indexFormatFBracket + 2,
                                          QLatin1String(""));
                dataAdded               = true;
            }
        }

        else if (backendName == QLatin1String("Geonames"))
        {
            if (humanTag.compare(QLatin1String("Country")) == 0)
            {
                if (!info.rgData[QLatin1String("countryName")].isEmpty())
                {
                    result    = info.rgData[QLatin1String("countryName")];
                    returnedFormat.append(QLatin1String("/{Country}"));
                    dataAdded = true;
                }
            }

            else if (humanTag == QLatin1String("Place"))
            {
                if (!info.rgData[QLatin1String("name")].isEmpty())
                {
                    result    = info.rgData[QLatin1String("name")];
                    returnedFormat.append(QLatin1String("/{Place}"));
                    dataAdded = true;
                }
            }
            else
            {
                returnedAddress.replace(indexFBracket - 1,
                                        indexLBracket - indexFBracket + 2,
                                        QLatin1String(""));

                int indexFormatFBracket = auxReturnedFormat.indexOf(QLatin1String("{"));
                int indexFormatLBracket = auxReturnedFormat.indexOf(QLatin1String("}"));
                auxReturnedFormat.replace(indexFormatFBracket - 1,
                                          indexFormatLBracket - indexFormatFBracket + 2,
                                          QLatin1String(""));
                dataAdded               = true;
            }
        }

        if (!dataAdded)
        {
            returnedAddress.replace(indexFBracket - 1, humanTag.length() + 3, QLatin1String(""));
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

#endif // PARSETAGSTRING_H
