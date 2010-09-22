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
    m_lfDb          = lf_db_new();
    m_lfDb->Load();
    m_lfCameras     = m_lfDb->GetCameras();
    m_init          = true;
    m_usedLens      = 0;
    m_usedCamera    = 0;

    return true;
}

void LensFunIface::setSettings(const LensFunContainer& other)
{
    m_settings   = other;
    m_usedCamera = findCamera(m_settings.cameraMake, m_settings.cameraModel);
    m_usedLens   = findLens(m_settings.lensModel);
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

LensFunIface::DevicePtr LensFunIface::findCamera(const QString& make, const QString& model) const
{
    const lfCamera* const* lfCamera = m_lfDb->FindCameras( make.toAscii().constData(), model.toAscii().constData() );
    if (lfCamera && *lfCamera)
    {
        DevicePtr cam = *lfCamera;

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

LensFunIface::LensPtr LensFunIface::findLens(const QString& model) const
{
    const lfLens* const* lfLens = m_lfDb->GetLenses();
    while (lfLens && *lfLens)
    {
        LensPtr lens = *lfLens;
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

LensFunIface::LensList LensFunIface::findLenses(const lfCamera* lfCamera, const QString& lensDesc,
                                                const QString& lensMaker) const
{
    LensList lensList;
    const lfLens** lfLens = 0;

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

bool LensFunIface::findFromMetadata(const DMetadata& meta)
{
    if (meta.isEmpty())
        return false;

    PhotoInfoContainer photoInfo = meta.getPhotographInformation();
    QString make                 = photoInfo.make;
    QString model                = photoInfo.model;
    QString lens                 = photoInfo.lens;
    bool ret                     = false;
    m_settings                   = LensFunContainer();

    // ------------------------------------------------------------------------------------------------

    const lfCamera** lfCamera = m_lfDb->FindCameras( make.toAscii().constData(), model.toAscii().constData() );

    if (lfCamera && *lfCamera)
    {
        m_usedCamera           = *lfCamera;
        ret                    = true;
        m_settings.cameraMake  = m_usedCamera->Maker;
        m_settings.cameraModel = m_usedCamera->Model;

        kDebug() << "Camera maker : " << m_settings.cameraMake;
        kDebug() << "Camera model : " << m_settings.cameraModel;

        // ------------------------------------------------------------------------------------------------

        if (!lens.isEmpty())
        {
            // Performing lens searches.

            kDebug() << "Lens desc.   : " << lens;
            QMap<int, LensPtr> bestMatches;
            QString            lensCutted;
            LensList           lensList;

            // STAGE 1, search in DB as well.
            lensList = findLenses(m_usedCamera, lens);
            if (!lensList.isEmpty()) bestMatches.insert(lensList.count(), lensList[0]);

            // STAGE 1, Adapt exiv2 strings to lensfun strings for Nikon.
            lensCutted = lens;
            if (lensCutted.contains("Nikon"))
            {
                lensCutted.replace("Nikon ", "");
                lensCutted.replace("Zoom-", "");
                lensCutted.replace("IF-ID", "ED-IF");
                lensList = findLenses(m_usedCamera, lensCutted);
                kDebug() << "* Check for Nikon lens (" << lensCutted << " : " << lensList.count() << ")";
                if (!lensList.isEmpty()) bestMatches.insert(lensList.count(), lensList[0]);
            }

            // TODO : Add here more specific lens maker rules.

            // LAST STAGE, Adapt exiv2 strings to lensfun strings. Some lens description use something like that :
            // "10.0 - 20.0 mm". This must be adapted like this : "10-20mm"
            lensCutted = lens;
            lensCutted.replace(QRegExp("\\.[0-9]"), "");
            lensCutted.replace(" - ", "-");
            lensCutted.replace(" mm", "mn");
            lensList = findLenses(m_usedCamera, lensCutted);
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
                m_usedLens           = bestMatches[bestMatches.keys()[0]];
                m_settings.lensModel = m_usedLens->Model;
                kDebug() << "Lens found   : " << m_settings.lensModel;
            }

            // ------------------------------------------------------------------------------------------------

            m_settings.cropFactor = m_usedLens->CropFactor;
            kDebug() << "Crop Factor  : " << m_settings.cropFactor;

            // ------------------------------------------------------------------------------------------------

            QString temp = photoInfo.focalLength;
            if (temp.isEmpty())
            {
                kDebug() << "Focal Length : NOT FOUND";
                ret &= false;
            }
            m_settings.focalLength = temp.mid(0, temp.length() -3).toDouble(); // HACK: strip the " mm" at the end ...
            kDebug() << "Focal Length : " << m_settings.focalLength;

            // ------------------------------------------------------------------------------------------------

            temp = photoInfo.aperture;
            if (temp.isEmpty())
            {
                kDebug() << "Aperture     : NOT FOUND";
                ret &= false;
            }
            m_settings.aperture = temp.mid(1).toDouble();
            kDebug() << "Aperture     : " << m_settings.aperture;

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

            temp                       = temp.replace(" m", "");
            m_settings.subjectDistance = temp.toDouble();
            kDebug() << "Subject dist.: " << m_settings.subjectDistance;
        }
        else
        {
            ret &= false;
        }
    }

    kDebug() << "Return val.  : " << ret;

    return ret;
}

bool LensFunIface::supportsDistortion() const
{
    if (!m_usedLens) return false;

    lfLensCalibDistortion res;
    return m_usedLens->InterpolateDistortion(m_settings.focalLength, res);
};

bool LensFunIface::supportsCCA() const
{
    if (!m_usedLens) return false;

    lfLensCalibTCA res;
    return m_usedLens->InterpolateTCA(m_settings.focalLength, res);
};

bool LensFunIface::supportsVig() const
{
    if (!m_usedLens) return false;

    lfLensCalibVignetting res;
    return m_usedLens->InterpolateVignetting(m_settings.focalLength,
                                             m_settings.aperture,
                                             m_settings.subjectDistance, res);
};

bool LensFunIface::supportsGeometry() const
{
    return supportsDistortion();
};

bool LensFunIface::supportsCCI() const
{
    return supportsVig();
};

#if 0
LensFunIface::correctionData LensFunIface::getCorrectionData()
{
}
#endif

}  // namespace Digikam
