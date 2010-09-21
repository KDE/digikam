/* ============================================================
 *
 * Date        : 2008-02-10
 * Description : a tool to fix automatically camera lens aberrations
 *
 * Copyright (C) 2008 by Adrian Schroeter <adrian at suse dot de>
 * Copyright (C) 2008-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "lensfuniface.h"

// KDE includes

#include <kdebug.h>

namespace Digikam
{

LensFunIface::LensFunIface()
{
    m_init = false;
    init();
}

LensFunIface::~LensFunIface()
{
    if (m_init)
    {
    }
}

bool LensFunIface::init()
{
    m_lfDb              = lf_db_new();
    m_lfDb->Load();
    m_lfCameras         = m_lfDb->GetCameras();
    m_init              = true;
    m_settings.usedLens = 0;

    return true;
}

void LensFunIface::setSettings(const LensFunContainer& other)
{
    m_settings = other;
}

LensFunContainer LensFunIface::settings() const
{
    return m_settings;
}

void LensFunIface::setFilterSettings(const LensFunContainer& other)
{
    m_settings.filterCCA  = other.filterCCA;
    m_settings.filterVig  = other.filterVig;
    m_settings.filterCCI  = other.filterCCI;
    m_settings.filterDist = other.filterDist;
    m_settings.filterGeom = other.filterGeom;
}

LensFunContainer::DevicePtr LensFunIface::findCamera(const QString& make, const QString& model) const
{
    const lfCamera* const* lfCamera = m_lfDb->GetCameras();
    if (lfCamera && *lfCamera)
    {
        LensFunContainer::DevicePtr cam = *lfCamera;
        if (QString(cam->Maker) == make &&
            QString(cam->Model) == model)
        {
            kDebug() << "Search for camera " << make << "-" << model << " ==> true";
            return cam;
        }
        ++lfCamera;
    }
    kDebug() << "Search for camera " << make << "-" << model << " ==> false";
    return 0;
}

LensFunContainer::LensPtr LensFunIface::findLens(const QString& model) const
{
    const lfLens* const* lfLens = m_lfDb->GetLenses();
    while (lfLens && *lfLens)
    {
        LensFunContainer::LensPtr lens = *lfLens;
        if (QString(lens->Model) == model)
        {
            kDebug() << "Search for lens " << model << " ==> true";
            return lens;
        }
        ++lfLens;
    }
    kDebug() << "Search for lens " << model << " ==> false";
    return 0;
}

LensFunContainer::LensList LensFunIface::findLenses(const lfCamera* lfCamera, const QString& lensDesc,
                                                    const QString& lensMaker) const
{
    LensFunContainer::LensList lensList;
    const lfLens**             lfLens = 0;

    if (lfCamera)
    {
        if (!lensMaker.isEmpty())
            lfLens = m_lfDb->FindLenses(lfCamera, lensMaker.toAscii().constData(), lensDesc.toAscii().constData());
        else
            lfLens = m_lfDb->FindLenses(lfCamera, NULL, lensDesc.toAscii().constData());

        while (lfLens && *lfLens)
        {
            lensList << (*lfLens);
            ++lfLens;
        }
    }
    return lensList;
}

bool LensFunIface::findFromMetadata(const DMetadata& meta, LensFunContainer& settings) const
{
    if (meta.isEmpty())
        return false;

    PhotoInfoContainer photoInfo = meta.getPhotographInformation();
    QString make                 = photoInfo.make;
    QString model                = photoInfo.model;
    QString lens                 = photoInfo.lens;
    bool ret                     = false;

    // ------------------------------------------------------------------------------------------------

    const lfCamera** lfCamera = m_lfDb->FindCameras( make.toAscii().constData(), model.toAscii().constData() );

    if (lfCamera && *lfCamera)
    {
        settings.usedCamera = *lfCamera;
        ret                 = true;

        kDebug() << "Camera maker : " << settings.usedCamera->Maker;
        kDebug() << "Camera model : " << settings.usedCamera->Model;

        // ------------------------------------------------------------------------------------------------

        if (!lens.isEmpty())
        {
            // Performing lens searches.

            kDebug() << "Lens desc.   : " << lens;
            QMap<int, LensFunContainer::LensPtr> bestMatches;
            QString                              lensCutted;
            LensFunContainer::LensList           lensList;

            // In first, search in DB as well.
            lensList = findLenses(settings.usedCamera, lens);
            if (!lensList.isEmpty()) bestMatches.insert(lensList.count(), lensList[0]);

            // Adapt exiv2 strings to lensfun strings for Nikon.
            lensCutted = lens;
            lensCutted.replace("Nikon ", "");
            lensCutted.replace("Zoom-", "");
            lensCutted.replace("IF-ID", "ED-IF");
            lensList = findLenses(settings.usedCamera, lensCutted);
            kDebug() << "* Check for Nikon lens (" << lensCutted << " : " << lensList.count() << ")";
            if (!lensList.isEmpty()) bestMatches.insert(lensList.count(), lensList[0]);

            // Adapt exiv2 strings to lensfun strings. Some lens description use something like that :
            // "10.0 - 20.0 mm". This must be adapted like this : "10-20mm"
            lensCutted = lens;
            lensCutted.replace(QRegExp("\\.[0-9]"), "");
            lensCutted.replace(" - ", "-");
            lensCutted.replace(" mm", "mn");
            lensList = findLenses(settings.usedCamera, lensCutted);
            kDebug() << "* Check for no maker lens (" << lensCutted << " : " << lensList.count() << ")";
            if (!lensList.isEmpty()) bestMatches.insert(lensList.count(), lensList[0]);

            // Display the results.

            if (bestMatches.isEmpty())
            {
                kDebug() << "lens matches : NOT FOUND";
                ret &= false;
            }
            else
            {
                settings.usedLens = bestMatches[bestMatches.keys()[0]];
                kDebug() << "Lens found   : " << settings.usedLens->Model;
            }

            // ------------------------------------------------------------------------------------------------

            settings.cropFactor = settings.usedLens->CropFactor;
            kDebug() << "Crop Factor  : " << settings.cropFactor;

            // ------------------------------------------------------------------------------------------------

            QString temp = photoInfo.focalLength;
            if (temp.isEmpty())
            {
                kDebug() << "Focal Length : NOT FOUND";
                ret &= false;
            }
            settings.focalLength = temp.mid(0, temp.length() -3).toDouble(); // HACK: strip the " mm" at the end ...
            kDebug() << "Focal Length : " << settings.focalLength;

            // ------------------------------------------------------------------------------------------------

            temp = photoInfo.aperture;
            if (temp.isEmpty())
            {
                kDebug() << "Aperture     : NOT FOUND";
                ret &= false;
            }
            settings.aperture = temp.mid(1).toDouble();
            kDebug() << "Aperture     : " << settings.aperture;

            // ------------------------------------------------------------------------------------------------
            // Try to get subject distance value.

            // From standard Exif.
            temp = meta.getExifTagString("Exif.Photo.SubjectDistance");
            if (temp.isEmpty())
            {
                // From standard XMP.
                temp = meta.getXmpTagString("Xmp.exif.SubjectDistance");
            }
            if (temp.isEmpty())
            {
                // From Canon Makernote.
                temp = meta.getExifTagString("Exif.CanonSi.SubjectDistance");
            }
            if (temp.isEmpty())
            {
                // From Nikon Makernote.
                temp = meta.getExifTagString("Exif.NikonLd2.FocusDistance");
            }
            if (temp.isEmpty())
            {
                // From Nikon Makernote.
                temp = meta.getExifTagString("Exif.NikonLd3.FocusDistance");
            }
            // TODO: Add here others Makernotes tags.

            if (temp.isEmpty())
            {
                kDebug() << "Subject dist.: NOT FOUND";
                ret &= false;
            }

            temp                     = temp.replace(" m", "");
            settings.subjectDistance = temp.toDouble();
            kDebug() << "Subject dist.: " << settings.subjectDistance;
        }
        else
        {
            ret &= false;
        }
    }

    kDebug() << "Return val.  : " << ret;

    return ret;
}

#if 0
LensFunIface::correctionData LensFunIface::getCorrectionData()
{
}
#endif

}  // namespace Digikam
