/* ============================================================
 *
 * Date        : 2008-02-10
 * Description : a tool to fix automatically camera lens aberrations
 *
 * Copyright (C) 2008      by Adrian Schroeter <adrian at suse dot de>
 * Copyright (C) 2008-2014 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "lensfunfilter.h"

// Qt includes

#include <QByteArray>
#include <QCheckBox>
#include <QString>

// KDE includes

#include <kdebug.h>

// Local includes

#include "lensfuniface.h"
#include "dmetadata.h"

namespace Digikam
{

class LensFunFilter::Private
{
public:

    Private()
    {
        iface    = 0;
        modifier = 0;
    }

    LensFunIface* iface;

    lfModifier*   modifier;
};

LensFunFilter::LensFunFilter(QObject* const parent)
    : DImgThreadedFilter(parent),
      d(new Private)
{
    d->iface = new LensFunIface;
    initFilter();
}

LensFunFilter::LensFunFilter(DImg* const orgImage, QObject* const parent,  const LensFunContainer& settings)
    : DImgThreadedFilter(orgImage, parent, "LensCorrection"),
      d(new Private)
{
    d->iface = new LensFunIface;
    d->iface->setSettings(settings);
    initFilter();
}

LensFunFilter::~LensFunFilter()
{
    cancelFilter();

    if (d->modifier)
    {
        d->modifier->Destroy();
    }

    delete d->iface;
    delete d;
}

void LensFunFilter::filterImage()
{
    m_destImage.bitBltImage(&m_orgImage, 0, 0);

    if (!d->iface)
    {
        kError() << "ERROR: LensFun Interface is null.";
        return;
    }

    if (!d->iface->usedLens())
    {
        kError() << "ERROR: LensFun Interface Lens device is null.";
        return;
    }

    // Lensfun Modifier flags to process

    int modifyFlags = 0;

    if (d->iface->settings().filterDST)
    {
        modifyFlags |= LF_MODIFY_DISTORTION;
    }

    if (d->iface->settings().filterGEO)
    {
        modifyFlags |= LF_MODIFY_GEOMETRY;
    }

    if (d->iface->settings().filterCCA)
    {
        modifyFlags |= LF_MODIFY_TCA;
    }

    if (d->iface->settings().filterVIG)
    {
        modifyFlags |= LF_MODIFY_VIGNETTING;
    }

    // Init lensfun lib, we are working on the full image.

    lfPixelFormat colorDepth = m_orgImage.bytesDepth() == 4 ? LF_PF_U8 : LF_PF_U16;

    d->modifier              = lfModifier::Create(d->iface->usedLens(),
                                                  d->iface->settings().cropFactor,
                                                  m_orgImage.width(),
                                                  m_orgImage.height());

    int modflags             = d->modifier->Initialize(d->iface->usedLens(),
                                                       colorDepth,
                                                       d->iface->settings().focalLength,
                                                       d->iface->settings().aperture,
                                                       d->iface->settings().subjectDistance,
                                                       1.0, /* no scaling */
                                                       LF_RECTILINEAR,
                                                       modifyFlags,
                                                       0 /*no inverse*/);

    if (!d->modifier)
    {
        kError() << "ERROR: cannot initialize LensFun Modifier.";
        return;
    }

    // Calc necessary steps for progress bar

    int steps = ((d->iface->settings().filterCCA)                                   ? 1 : 0) +
                ((d->iface->settings().filterVIG)                                   ? 1 : 0) +
                ((d->iface->settings().filterDST || d->iface->settings().filterGEO) ? 1 : 0);

    kDebug() << "LensFun Modifier Flags: " << modflags << "  Steps:" << steps;

    if (steps < 1)
    {
        kDebug() << "No LensFun Modifier steps. There is nothing to process...";
        return;
    }

    // The real correction to do

    int loop   = 0;
    int lwidth = m_orgImage.width() * 2 * 3;
    QScopedArrayPointer<float> pos(new float[lwidth]);

    kDebug() << "Image size to process: (" << m_orgImage.width() << ", " << m_orgImage.height() << ")";

    // Stage 1: Chromatic Aberation Corrections

    if (d->iface->settings().filterCCA)
    {
        m_orgImage.prepareSubPixelAccess(); // init lanczos kernel

        for (unsigned int y = 0; runningFlag() && (y < m_orgImage.height()); ++y)
        {
            if (d->modifier->ApplySubpixelDistortion(0.0, y, m_orgImage.width(), 1, pos.data()))
            {
                float* src = pos.data();

                for (unsigned x = 0; runningFlag() && (x < m_destImage.width()); ++x)
                {
                    DColor destPixel(0, 0, 0, 0xFFFF, m_destImage.sixteenBit());

                    destPixel.setRed(m_orgImage.getSubPixelColorFast(src[0],   src[1]).red());
                    destPixel.setGreen(m_orgImage.getSubPixelColorFast(src[2], src[3]).green());
                    destPixel.setBlue(m_orgImage.getSubPixelColorFast(src[4],  src[5]).blue());

                    m_destImage.setPixelColor(x, y, destPixel);
                    src += 2 * 3;
                }

                ++loop;
            }

            // Update progress bar in dialog.
            int progress = (int)(((double)y * 100.0) / m_orgImage.height());

            if (progress % 5 == 0)
            {
                postProgress(progress / steps);
            }
        }

        kDebug() << "Chromatic Aberation Corrections applied. (loop: " << loop << ")";
    }

    // Stage 2: Color Corrections: Vignetting and Color Contribution Index

    if (d->iface->settings().filterVIG)
    {
        uchar* data   = m_destImage.bits();
        loop          = 0;
        double offset = 0.0;

        if (steps == 3)
        {
            offset = 33.3;
        }
        else if (steps == 2 && d->iface->settings().filterCCA)
        {
            offset = 50.0;
        }

        for (unsigned int y = 0; runningFlag() && (y < m_destImage.height()); ++y)
        {
            if (d->modifier->ApplyColorModification(data, 0.0, y, m_destImage.width(), 1,
                                                    LF_CR_4(RED, GREEN, BLUE, UNKNOWN), 0))
            {
                data += m_destImage.width() * m_destImage.bytesDepth();
                ++loop;
            }

            // Update progress bar in dialog.
            int progress = (int)(((double)y * 100.0) / m_destImage.height());

            if (progress % 5 == 0)
            {
                postProgress(progress / steps + offset);
            }
        }

        kDebug() << "Vignetting and Color Corrections applied. (loop: " << loop << ")";
    }

    // Stage 3: Distortion and Geometry Corrections

    if (d->iface->settings().filterDST || d->iface->settings().filterGEO)
    {
        loop = 0;

        // we need a deep copy first
        DImg tempImage(m_destImage.width(), m_destImage.height(), m_destImage.sixteenBit(), m_destImage.hasAlpha());
        m_destImage.prepareSubPixelAccess(); // init lanczos kernel

        for (unsigned long y = 0; runningFlag() && (y < tempImage.height()); ++y)
        {
            if (d->modifier->ApplyGeometryDistortion(0.0, y, tempImage.width(), 1, pos.data()))
            {
                float* src = pos.data();

                for (unsigned long x = 0; runningFlag() && (x < tempImage.width()); ++x, ++loop)
                {
                    //kDebug() << " ZZ " << src[0] << " " << src[1] << " " << (int)src[0] << " " << (int)src[1];

                    tempImage.setPixelColor(x, y, m_destImage.getSubPixelColor(src[0], src[1]));
                    src += 2;
                }
            }

            // Update progress bar in dialog.
            int progress = (int)(((double)y * 100.0) / tempImage.height());

            if (progress % 5 == 0)
            {
                postProgress(progress / steps + 33.3 * (steps - 1));
            }
        }

        kDebug() << "Distortion and Geometry Corrections applied. (loop: " << loop << ")";

        if (loop != 0)
        {
            m_destImage = tempImage;
        }
    }
}

bool LensFunFilter::registerSettingsToXmp(KExiv2Data& data) const
{
    // Register in digiKam Xmp namespace all information about Lens corrections.

    QString str;
    LensFunContainer prm = d->iface->settings();

    str.append(i18n("Camera: %1-%2",        prm.cameraMake, prm.cameraModel));
    str.append("\n");
    str.append(i18n("Lens: %1",             prm.lensModel));
    str.append("\n");
    str.append(i18n("Subject Distance: %1", QString::number(prm.subjectDistance)));
    str.append("\n");
    str.append(i18n("Aperture: %1",         QString::number(prm.aperture)));
    str.append("\n");
    str.append(i18n("Focal Length: %1",     QString::number(prm.focalLength)));
    str.append("\n");
    str.append(i18n("Crop Factor: %1",      QString::number(prm.cropFactor)));
    str.append("\n");
    str.append(i18n("CCA Correction: %1",   prm.filterCCA  && d->iface->supportsCCA()       ? i18n("enabled") : i18n("disabled")));
    str.append("\n");
    str.append(i18n("VIG Correction: %1",   prm.filterVIG  && d->iface->supportsVig()       ? i18n("enabled") : i18n("disabled")));
    str.append("\n");
    str.append(i18n("DST Correction: %1",   prm.filterDST && d->iface->supportsDistortion() ? i18n("enabled") : i18n("disabled")));
    str.append("\n");
    str.append(i18n("GEO Correction: %1",   prm.filterGEO && d->iface->supportsGeometry()   ? i18n("enabled") : i18n("disabled")));

    DMetadata meta(data);
    bool ret = meta.setXmpTagString("Xmp.digiKam.LensCorrectionSettings",
                                    str.replace('\n', " ; "), false);
    data     = meta.data();

    return ret;
}

FilterAction LensFunFilter::filterAction()
{
    FilterAction action(FilterIdentifier(), CurrentVersion());
    action.setDisplayableName(DisplayableName());

    LensFunContainer prm = d->iface->settings();
    action.addParameter("ccaCorrection",   prm.filterCCA);
    action.addParameter("vigCorrection",   prm.filterVIG);
    action.addParameter("dstCorrection",   prm.filterDST);
    action.addParameter("geoCorrection",   prm.filterGEO);
    action.addParameter("cropFactor",      prm.cropFactor);
    action.addParameter("focalLength",     prm.focalLength);
    action.addParameter("aperture",        prm.aperture);
    action.addParameter("subjectDistance", prm.subjectDistance);
    action.addParameter("cameraMake",      prm.cameraMake);
    action.addParameter("cameraModel",     prm.cameraModel);
    action.addParameter("lensModel",       prm.lensModel);

    return action;
}

void LensFunFilter::readParameters(const Digikam::FilterAction& action)
{
    LensFunContainer prm = d->iface->settings();
    prm.filterCCA        = action.parameter("ccaCorrection").toBool();
    prm.filterVIG        = action.parameter("vigCorrection").toBool();
    prm.filterDST        = action.parameter("dstCorrection").toBool();
    prm.filterGEO        = action.parameter("geoCorrection").toBool();
    prm.cropFactor       = action.parameter("cropFactor").toDouble();
    prm.focalLength      = action.parameter("focalLength").toDouble();
    prm.aperture         = action.parameter("aperture").toDouble();
    prm.subjectDistance  = action.parameter("subjectDistance").toDouble();
    prm.cameraMake       = action.parameter("cameraMake").toString();
    prm.cameraModel      = action.parameter("cameraModel").toString();
    prm.lensModel        = action.parameter("lensModel").toString();
    d->iface->setSettings(prm);
}

}  // namespace Digikam
