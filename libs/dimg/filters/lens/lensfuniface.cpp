/* ============================================================
 *
 * Date        : 2008-02-10
 * Description : a plugin to fix automatically camera lens aberrations
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
    m_lfDb       = lf_db_new();
    m_lfDb->Load();
    m_lfCameras  = m_lfDb->GetCameras();
    m_init       = true;
    m_usedLens   = NULL;

    return true;
}

void LensFunIface::setSettings(const LensFunContainer& settings)
{
    m_settings = settings;
}

bool LensFunIface::supportsDistortion()
{
    if (m_usedLens == NULL) return false;

    lfLensCalibDistortion res;
    return m_usedLens->InterpolateDistortion(m_focalLength, res);
}

bool LensFunIface::supportsCCA()
{
    if (m_usedLens == NULL) return false;

    lfLensCalibTCA res;
    return m_usedLens->InterpolateTCA(m_focalLength, res);
}

bool LensFunIface::supportsVig()
{
    if (m_usedLens == NULL) return false;

    lfLensCalibVignetting res;
    return m_usedLens->InterpolateVignetting(m_focalLength, m_aperture, m_subjectDistance, res);
}

int LensFunIface::findTextFromList(const QStringList& list, const QString& text, Qt::CaseSensitivity cs) const
{
    int i = 0;
    foreach(const QString s, list)
    {
        if (s.contains(text, cs))
            return i;

        i++;
    }
    return -1;
}

bool LensFunIface::findFromMetadata(const DMetadata& meta)
{
    if (meta.isEmpty())
        return false;

    PhotoInfoContainer photoInfo = meta.getPhotographInformation();
    QString make                 = photoInfo.make;
    QString model                = photoInfo.model;
    QString lens                 = photoInfo.lens;

    // Data to field
    QString     makeLF;
    QString     modelLF;
    QStringList lensList;
    double      focal, aperture, distance;
    bool        ret = false;

    // ------------------------------------------------------------------------------------------------

    const lfCamera** lfCamera = m_lfDb->FindCameras( make.toAscii(), model.toAscii() );

    if (lfCamera && *lfCamera)
    {
        makeLF  = (*lfCamera)->Maker;
        modelLF = (*lfCamera)->Model;

        kDebug() << "Camera maker : " << makeLF;
        kDebug() << "Camera model : " << modelLF;

        ret = true;

        // ------------------------------------------------------------------------------------------------

        if (!lens.isEmpty())
        {
            kDebug() << "Lens desc.   : " << lens;

            const lfLens** lfLens = m_lfDb->FindLenses(*lfCamera, NULL, lens.toAscii());

            while (lfLens && *lfLens)
            {
                lensList << (*lfLens)->Model;
                ++lfLens;
            }

            if (lensList.isEmpty())
            {
                kDebug() << "lens matches : NOT FOUND";
                ret &= false;
            }
            else if (lensList.count() > 1)
            {
                int i=0;
                foreach(const QString s, lensList)
                {
                    if (i == 0) kDebug() << "Lens matches : " << s;
                    else        kDebug() << "             : " << s;
                    i++;
                }
                ret &= false;
            }
            else
            {
                kDebug() << "Lens found   : " << lensList[0];
            }

            // ------------------------------------------------------------------------------------------------

            QString temp = photoInfo.focalLength;
            if (temp.isEmpty())
            {
                kDebug() << "Focal Length : NOT FOUND";
                ret &= false;
            }
            focal = temp.mid(0, temp.length() -3).toDouble(); // HACK: strip the " mm" at the end ...
            kDebug() << "Focal Length : " << focal;

            // ------------------------------------------------------------------------------------------------

            temp = photoInfo.aperture;
            if (temp.isEmpty())
            {
                kDebug() << "Aperture     : NOT FOUND";
                ret &= false;
            }
            aperture = temp.mid(1).toDouble();
            kDebug() << "Aperture     : " << aperture;

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

            temp     = temp.replace(" m", "");
            distance = temp.toDouble();
            kDebug() << "Subject dist.: " << distance;
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
