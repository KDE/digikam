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

bool LensFunIface::findFromMetadata(const DMetadata& meta)
{
    if (meta.isEmpty())
        return false;

    PhotoInfoContainer photoInfo = meta.getPhotographInformation();
    QString make                 = photoInfo.make;
    QString model                = photoInfo.model;
    QString lens                 = photoInfo.lens;

    kDebug() << "make from metadata:  " << make;
    kDebug() << "model from metadata: " << model;
    kDebug() << "lens from metadata:  " << lens;

    // ------------------------------------------------------------------------------------------------

    const lfCamera** makes = m_lfDb->FindCameras( make.toAscii(), model.toAscii() );
    QString makeLF;
    QString modelLF;

    if (makes && *makes)
    {
        makeLF  = (*makes)->Maker;
        modelLF = (*makes)->Model;
    }
    else
    {
        return false;
    }

    kDebug() << "make from LensFun:  " << makeLF;
    kDebug() << "model from LensFun: " << modelLF;

    const lfCamera* const* it = m_lfCameras;
    const lfCamera* dev       = 0;

    while ( *it )
    {
       if ( ((*it)->Model == modelLF) && ((*it)->Maker == makeLF) )
       {
            dev = *it;
            break;
       }

       ++it;
    }

    if (!dev)
    {
         kDebug() << "Device is null! ";
         return false;
    }

    const lfLens** lenses = m_lfDb->FindLenses( dev, NULL, NULL );
    QStringList lensList;
//    d->iface->m_cropFactor = dev->CropFactor;

    // Load all lens name fromLensFun to a list.
    while (lenses && *lenses)
    {
        lensList << (*lenses)->Model;
        ++lenses;
    }

    // The LensFun DB has the Maker before the Lens model name.
    // We use here the Camera Maker, because the Lens Maker seems not to be
    // part of the Exif data. This is of course bad for 3rd party lenses, but
    // they seem anyway not to have Exif entries usually :/
    int lensIdx = lensList.indexOf(lens);

    if (lensIdx < 0)
       lensIdx = lensList.indexOf(make + ' ' + lens);

    if (lensIdx < 0)
    {
        QString lensCutted = lens;

        if (lensCutted.contains("Nikon"))
        {
            // adapt exiv2 strings to lensfun strings
            lensCutted.replace("Nikon ", "");
            lensCutted.replace("Zoom-", "");
            lensCutted.replace("IF-ID", "ED-IF");
        }

        const lfLens** lenses = m_lfDb->FindLenses( dev, NULL, lensCutted.toAscii().data() );
        int count             = 0;
        QString lensLF;

        while (lenses && *lenses)
        {
            lensLF = (*lenses)->Model;
            ++lenses;
            ++count;
        }

        if (count == 1)
        {
            lensIdx = lensList.indexOf(lensLF);
        }
    }

    if (lensIdx >= 0)
    {
        // found lens model directly, best case :)
        kDebug() << "lens from LensFun: " << lensList[lensIdx];
    }
    else
    {
        kDebug() << "lens from LensFun: NOT FOUND";
    }


/*
    slotUpdateCombos();

    int modelIdx = d->model->combo()->findText(model);

    if (modelIdx < 0)
    {
        const lfCamera** makes = d->iface->m_lfDb->FindCameras( make.toAscii(), model.toAscii() );
        QString modelLF        = "";
        int count              = 0;

        while (makes && *makes)
        {
            modelLF = (*makes)->Model;
            ++makes;
            ++count;
        }
        if (count == 1)
        {
            modelIdx = d->model->combo()->findText(modelLF);
        }
    }

    if (modelIdx >= 0)
    {
        d->model->setCurrentIndex(modelIdx);
        d->model->setEnabled(false);
        slotUpdateLensCombo();
    }

    // The LensFun DB has the Maker before the Lens model name.
    // We use here the Camera Maker, because the Lens Maker seems not to be
    // part of the Exif data. This is of course bad for 3rd party lenses, but
    // they seem anyway not to have Exif entries usually :/
    int lensIdx = d->lens->combo()->findText(lens);

    if (lensIdx < 0)
       lensIdx = d->lens->combo()->findText(make + ' ' + lens);

    if (lensIdx < 0)
    {
        QString lensCutted = lens;

        if (lensCutted.contains("Nikon"))
        {
            // adapt exiv2 strings to lensfun strings
            lensCutted.replace("Nikon ", "");
            lensCutted.replace("Zoom-", "");
            lensCutted.replace("IF-ID", "ED-IF");
        }

        QVariant v            = d->model->combo()->itemData( d->model->currentIndex() );
        DevicePtr dev         = v.value<LensFunCameraSelector::DevicePtr>();
        const lfLens** lenses = d->iface->m_lfDb->FindLenses( dev, NULL, lensCutted.toAscii().data() );
        QString lensLF        = "";
        int count             = 0;

        while (lenses && *lenses)
        {
            lensLF = (*lenses)->Model;
            ++lenses;
            ++count;
        }
        if (count == 1)
        {
            lensIdx = d->lens->combo()->findText(lensLF);
        }
    }

    if (lensIdx >= 0)
    {
        // found lens model directly, best case :)
        d->lens->setCurrentIndex(lensIdx);
        d->lens->setEnabled(false);
    }
    else
    {
        // Lens not found, try to reduce the list according to the values we have
        // FIXME: Implement removal of not matching lenses ...
        d->lens->setEnabled(true);
    }

    kDebug() << "Search for Lens: " << make << " :: " << lens
             << "< and found: >" << d->lens->combo()->itemText(0) + " <";

    QString temp = photoInfo.focalLength;
    if (!temp.isEmpty())
    {
        double focal = temp.mid(0, temp.length() -3).toDouble(); // HACK: strip the " mm" at the end ...
        kDebug() << "Focal Length: " << focal;
        d->focal->setValue(focal);
        d->focal->setEnabled(false);
    }

    temp = photoInfo.aperture;
    if (!temp.isEmpty())
    {
        double aperture = temp.mid(1).toDouble();
        kDebug() << "Aperture: " << aperture;
        d->aperture->setValue(aperture);
        d->aperture->setEnabled(false);
    }

    // ------------------------------------------------------------------------------------------------
    // Try to get subject distance value.

    // From standard Exif.
    temp = d->metadata.getExifTagString("Exif.Photo.SubjectDistance");
    if (temp.isEmpty())
    {
        // From standard XMP.
        temp = d->metadata.getXmpTagString("Xmp.exif.SubjectDistance");
    }
    if (temp.isEmpty())
    {
        // From Canon Makernote.
        temp = d->metadata.getExifTagString("Exif.CanonSi.SubjectDistance");
    }
    if (temp.isEmpty())
    {
        // From Nikon Makernote.
        temp = d->metadata.getExifTagString("Exif.NikonLd2.FocusDistance");
    }
    if (temp.isEmpty())
    {
        // From Nikon Makernote.
        temp = d->metadata.getExifTagString("Exif.NikonLd3.FocusDistance");
    }
    // TODO: Add here others Makernotes tags.

    if (!temp.isEmpty())
    {
        temp            = temp.replace(" m", "");
        double distance = temp.toDouble();
        kDebug() << "Subject Distance: " << distance;
        d->distance->setValue(distance);
        d->distance->setEnabled(false);
    }
*/
}

#if 0
LensFunIface::correctionData LensFunIface::getCorrectionData()
{
}
#endif

}  // namespace Digikam
