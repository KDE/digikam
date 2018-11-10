/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-02-23
 * Description : image metadata interface - photo info helpers.
 *
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2011      by Leif Huhn <leif at dkstat dot com>
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

#include "dmetadata.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QLocale>

// Local includes

#include "metaenginesettings.h"
#include "digikam_version.h"
#include "digikam_globals.h"
#include "digikam_debug.h"

namespace Digikam
{

PhotoInfoContainer DMetadata::getPhotographInformation() const
{
    PhotoInfoContainer photoInfo;

    if (hasExif() || hasXmp())
    {
        photoInfo.dateTime = getItemDateTime();

        // -----------------------------------------------------------------------------------

        photoInfo.make     = getExifTagString("Exif.Image.Make");

        if (photoInfo.make.isEmpty())
        {
            photoInfo.make = getXmpTagString("Xmp.tiff.Make");
        }

        // -----------------------------------------------------------------------------------

        photoInfo.model    = getExifTagString("Exif.Image.Model");

        if (photoInfo.model.isEmpty())
        {
            photoInfo.model = getXmpTagString("Xmp.tiff.Model");
        }

        // -----------------------------------------------------------------------------------

        photoInfo.lens     = getLensDescription();

        // -----------------------------------------------------------------------------------

        photoInfo.aperture = getExifTagString("Exif.Photo.FNumber");

        if (photoInfo.aperture.isEmpty())
        {
            photoInfo.aperture = getExifTagString("Exif.Photo.ApertureValue");
        }

        if (photoInfo.aperture.isEmpty())
        {
            photoInfo.aperture = getXmpTagString("Xmp.exif.FNumber");
        }

        if (photoInfo.aperture.isEmpty())
        {
            photoInfo.aperture = getXmpTagString("Xmp.exif.ApertureValue");
        }

        // -----------------------------------------------------------------------------------

        photoInfo.exposureTime = getExifTagString("Exif.Photo.ExposureTime");

        if (photoInfo.exposureTime.isEmpty())
        {
            photoInfo.exposureTime = getExifTagString("Exif.Photo.ShutterSpeedValue");
        }

        if (photoInfo.exposureTime.isEmpty())
        {
            photoInfo.exposureTime = getXmpTagString("Xmp.exif.ExposureTime");
        }

        if (photoInfo.exposureTime.isEmpty())
        {
            photoInfo.exposureTime = getXmpTagString("Xmp.exif.ShutterSpeedValue");
        }

        // -----------------------------------------------------------------------------------

        photoInfo.exposureMode    = getExifTagString("Exif.Photo.ExposureMode");

        if (photoInfo.exposureMode.isEmpty())
        {
            photoInfo.exposureMode = getXmpTagString("Xmp.exif.ExposureMode");
        }

        if (photoInfo.exposureMode.isEmpty())
        {
            photoInfo.exposureMode = getExifTagString("Exif.CanonCs.MeteringMode");
        }

        // -----------------------------------------------------------------------------------

        photoInfo.exposureProgram = getExifTagString("Exif.Photo.ExposureProgram");

        if (photoInfo.exposureProgram.isEmpty())
        {
            photoInfo.exposureProgram = getXmpTagString("Xmp.exif.ExposureProgram");
        }

        if (photoInfo.exposureProgram.isEmpty())
        {
            photoInfo.exposureProgram = getExifTagString("Exif.CanonCs.ExposureProgram");
        }

        // -----------------------------------------------------------------------------------

        photoInfo.focalLength     = getExifTagString("Exif.Photo.FocalLength");

        if (photoInfo.focalLength.isEmpty())
        {
            photoInfo.focalLength = getXmpTagString("Xmp.exif.FocalLength");
        }

        if (photoInfo.focalLength.isEmpty())
        {
            photoInfo.focalLength = getExifTagString("Exif.Canon.FocalLength");
        }

        // -----------------------------------------------------------------------------------

        photoInfo.focalLength35mm = getExifTagString("Exif.Photo.FocalLengthIn35mmFilm");

        if (photoInfo.focalLength35mm.isEmpty())
        {
            photoInfo.focalLength35mm = getXmpTagString("Xmp.exif.FocalLengthIn35mmFilm");
        }

        // -----------------------------------------------------------------------------------

        QStringList ISOSpeedTags;

        ISOSpeedTags << QLatin1String("Exif.Photo.ISOSpeedRatings");
        ISOSpeedTags << QLatin1String("Exif.Photo.ExposureIndex");
        ISOSpeedTags << QLatin1String("Exif.Image.ISOSpeedRatings");
        ISOSpeedTags << QLatin1String("Xmp.exif.ISOSpeedRatings");
        ISOSpeedTags << QLatin1String("Xmp.exif.ExposureIndex");
        ISOSpeedTags << QLatin1String("Exif.CanonSi.ISOSpeed");
        ISOSpeedTags << QLatin1String("Exif.CanonCs.ISOSpeed");
        ISOSpeedTags << QLatin1String("Exif.Nikon1.ISOSpeed");
        ISOSpeedTags << QLatin1String("Exif.Nikon2.ISOSpeed");
        ISOSpeedTags << QLatin1String("Exif.Nikon3.ISOSpeed");
        ISOSpeedTags << QLatin1String("Exif.NikonIi.ISO");
        ISOSpeedTags << QLatin1String("Exif.NikonIi.ISO2");
        ISOSpeedTags << QLatin1String("Exif.MinoltaCsNew.ISOSetting");
        ISOSpeedTags << QLatin1String("Exif.MinoltaCsOld.ISOSetting");
        ISOSpeedTags << QLatin1String("Exif.MinoltaCs5D.ISOSpeed");
        ISOSpeedTags << QLatin1String("Exif.MinoltaCs7D.ISOSpeed");
        ISOSpeedTags << QLatin1String("Exif.Sony1Cs.ISOSetting");
        ISOSpeedTags << QLatin1String("Exif.Sony2Cs.ISOSetting");
        ISOSpeedTags << QLatin1String("Exif.Sony1Cs2.ISOSetting");
        ISOSpeedTags << QLatin1String("Exif.Sony2Cs2.ISOSetting");
        ISOSpeedTags << QLatin1String("Exif.Sony1MltCsA100.ISOSetting");
        ISOSpeedTags << QLatin1String("Exif.Pentax.ISO");
        ISOSpeedTags << QLatin1String("Exif.Olympus.ISOSpeed");
        ISOSpeedTags << QLatin1String("Exif.Samsung2.ISO");

        photoInfo.sensitivity = getExifTagStringFromTagsList(ISOSpeedTags);

        // -----------------------------------------------------------------------------------

        photoInfo.flash = getExifTagString("Exif.Photo.Flash");

        if (photoInfo.flash.isEmpty())
        {
            photoInfo.flash = getXmpTagString("Xmp.exif.Flash");
        }

        if (photoInfo.flash.isEmpty())
        {
            photoInfo.flash = getExifTagString("Exif.CanonCs.FlashActivity");
        }

        // -----------------------------------------------------------------------------------

        photoInfo.whiteBalance = getExifTagString("Exif.Photo.WhiteBalance");

        if (photoInfo.whiteBalance.isEmpty())
        {
            photoInfo.whiteBalance = getXmpTagString("Xmp.exif.WhiteBalance");
        }

        // -----------------------------------------------------------------------------------

        double l, L, a;
        photoInfo.hasCoordinates = getGPSInfo(a, l, L);
    }

    return photoInfo;
}

QString DMetadata::getLensDescription() const
{
    QString     lens;
    QStringList lensExifTags;

    // In first, try to get Lens information from makernotes.

    lensExifTags.append(QLatin1String("Exif.CanonCs.LensType"));      // Canon Cameras Makernote.
    lensExifTags.append(QLatin1String("Exif.CanonCs.Lens"));          // Canon Cameras Makernote.
    lensExifTags.append(QLatin1String("Exif.Canon.0x0095"));          // Alternative Canon Cameras Makernote.
    lensExifTags.append(QLatin1String("Exif.NikonLd1.LensIDNumber")); // Nikon Cameras Makernote.
    lensExifTags.append(QLatin1String("Exif.NikonLd2.LensIDNumber")); // Nikon Cameras Makernote.
    lensExifTags.append(QLatin1String("Exif.NikonLd3.LensIDNumber")); // Nikon Cameras Makernote.
    lensExifTags.append(QLatin1String("Exif.Minolta.LensID"));        // Minolta Cameras Makernote.
    lensExifTags.append(QLatin1String("Exif.Photo.LensModel"));       // Sony Cameras Makernote (and others?).
    lensExifTags.append(QLatin1String("Exif.Sony1.LensID"));          // Sony Cameras Makernote.
    lensExifTags.append(QLatin1String("Exif.Sony2.LensID"));          // Sony Cameras Makernote.
    lensExifTags.append(QLatin1String("Exif.SonyMinolta.LensID"));    // Sony Cameras Makernote.
    lensExifTags.append(QLatin1String("Exif.Pentax.LensType"));       // Pentax Cameras Makernote.
    lensExifTags.append(QLatin1String("Exif.PentaxDng.LensType"));    // Pentax Cameras Makernote.
    lensExifTags.append(QLatin1String("Exif.Panasonic.0x0051"));      // Panasonic Cameras Makernote.
    lensExifTags.append(QLatin1String("Exif.Panasonic.0x0310"));      // Panasonic Cameras Makernote.
    lensExifTags.append(QLatin1String("Exif.Sigma.LensRange"));       // Sigma Cameras Makernote.
    lensExifTags.append(QLatin1String("Exif.Samsung2.LensType"));     // Samsung Cameras Makernote.
    lensExifTags.append(QLatin1String("Exif.Photo.0xFDEA"));          // Non-standard Exif tag set by Camera Raw.
    lensExifTags.append(QLatin1String("Exif.OlympusEq.LensModel"));   // Olympus Cameras Makernote.

    // Olympus Cameras Makernote. FIXME is this necessary? exiv2 returns complete name, which doesn't match with lensfun information, see bug #311295
    //lensExifTags.append("Exif.OlympusEq.LensType");

    // TODO : add Fuji camera Makernotes.

    // -------------------------------------------------------------------
    // Try to get Lens Data information from Exif.

    for (QStringList::const_iterator it = lensExifTags.constBegin(); it != lensExifTags.constEnd(); ++it)
    {
        lens = getExifTagString((*it).toLatin1().constData());

        if ( !lens.isEmpty() &&
             !(lens.startsWith(QLatin1Char('(')) &&
               lens.endsWith(QLatin1Char(')'))
              )
           )   // To prevent undecoded tag values from Exiv2 as "(65535)".
        {
            return lens;
        }
    }

    // -------------------------------------------------------------------
    // Try to get Lens Data information from XMP.
    // XMP aux tags.
    lens = getXmpTagString("Xmp.aux.Lens");

    if (lens.isEmpty())
    {
        // XMP M$ tags (Lens Maker + Lens Model).
        lens = getXmpTagString("Xmp.MicrosoftPhoto.LensManufacturer");

        if (!lens.isEmpty())
        {
            lens.append(QLatin1Char(' '));
        }

        lens.append(getXmpTagString("Xmp.MicrosoftPhoto.LensModel"));
    }

    return lens;
}

double DMetadata::apexApertureToFNumber(double aperture)
{
    // convert from APEX. See Exif spec, Annex C.
    if (aperture == 0.0)
    {
        return 1;
    }
    else if (aperture == 1.0)
    {
        return 1.4;
    }
    else if (aperture == 2.0)
    {
        return 2;
    }
    else if (aperture == 3.0)
    {
        return 2.8;
    }
    else if (aperture == 4.0)
    {
        return 4;
    }
    else if (aperture == 5.0)
    {
        return 5.6;
    }
    else if (aperture == 6.0)
    {
        return 8;
    }
    else if (aperture == 7.0)
    {
        return 11;
    }
    else if (aperture == 8.0)
    {
        return 16;
    }
    else if (aperture == 9.0)
    {
        return 22;
    }
    else if (aperture == 10.0)
    {
        return 32;
    }

    return exp(log(2) * aperture / 2.0);
}

double DMetadata::apexShutterSpeedToExposureTime(double shutterSpeed)
{
    // convert from APEX. See Exif spec, Annex C.
    if (shutterSpeed == -5.0)
    {
        return 30;
    }
    else if (shutterSpeed == -4.0)
    {
        return 15;
    }
    else if (shutterSpeed == -3.0)
    {
        return 8;
    }
    else if (shutterSpeed == -2.0)
    {
        return 4;
    }
    else if (shutterSpeed == -1.0)
    {
        return 2;
    }
    else if (shutterSpeed == 0.0)
    {
        return 1;
    }
    else if (shutterSpeed == 1.0)
    {
        return 0.5;
    }
    else if (shutterSpeed == 2.0)
    {
        return 0.25;
    }
    else if (shutterSpeed == 3.0)
    {
        return 0.125;
    }
    else if (shutterSpeed == 4.0)
    {
        return 1.0 / 15.0;
    }
    else if (shutterSpeed == 5.0)
    {
        return 1.0 / 30.0;
    }
    else if (shutterSpeed == 6.0)
    {
        return 1.0 / 60.0;
    }
    else if (shutterSpeed == 7.0)
    {
        return 0.008;    // 1/125
    }
    else if (shutterSpeed == 8.0)
    {
        return 0.004;    // 1/250
    }
    else if (shutterSpeed == 9.0)
    {
        return 0.002;    // 1/500
    }
    else if (shutterSpeed == 10.0)
    {
        return 0.001;    // 1/1000
    }
    else if (shutterSpeed == 11.0)
    {
        return 0.0005;    // 1/2000
    }
    // additions by me
    else if (shutterSpeed == 12.0)
    {
        return 0.00025;    // 1/4000
    }
    else if (shutterSpeed == 13.0)
    {
        return 0.000125;    // 1/8000
    }

    return exp( - log(2) * shutterSpeed);
}

} // namespace Digikam
