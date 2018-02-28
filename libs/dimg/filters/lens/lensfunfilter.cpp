/* ============================================================
 *
 * Date        : 2008-02-10
 * Description : a tool to fix automatically camera lens aberrations
 *
 * Copyright (C) 2008      by Adrian Schroeter <adrian at suse dot de>
 * Copyright (C) 2008-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include <QtConcurrent>

// Local includes

#include "digikam_debug.h"
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
        loop     = 0;
    }

    DImg          tempImage;

    LensFunIface* iface;

    lfModifier*   modifier;

    int           loop;
};

LensFunFilter::LensFunFilter(QObject* const parent)
    : DImgThreadedFilter(parent),
      d(new Private)
{
    d->iface = new LensFunIface;
    initFilter();
}

LensFunFilter::LensFunFilter(DImg* const orgImage, QObject* const parent,  const LensFunContainer& settings)
    : DImgThreadedFilter(orgImage, parent, QLatin1String("LensCorrection")),
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

void LensFunFilter::filterCCAMultithreaded(uint start, uint stop)
{
    QScopedArrayPointer<float> pos(new float[m_orgImage.width() * 2 * 3]);

    for (unsigned int y = start; runningFlag() && (y < stop); ++y)
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
        }
    }
}

void LensFunFilter::filterVIGMultithreaded(uint start, uint stop)
{
    uchar* data = m_destImage.bits();
    data += m_destImage.width() * m_destImage.bytesDepth() * start;

    for (unsigned int y = start; runningFlag() && (y < stop); ++y)
    {
        if (d->modifier->ApplyColorModification(data,
                                                0.0,
                                                y,
                                                m_destImage.width(),
                                                1,
                                                LF_CR_4(RED, GREEN, BLUE, UNKNOWN),
                                                0))
        {
            data += m_destImage.width() * m_destImage.bytesDepth();
        }
    }
}

void LensFunFilter::filterDSTMultithreaded(uint start, uint stop)
{
    QScopedArrayPointer<float> pos(new float[m_orgImage.width() * 2 * 3]);

    for (unsigned int y = start; runningFlag() && (y < stop); ++y)
    {
        if (d->modifier->ApplyGeometryDistortion(0.0, y, d->tempImage.width(), 1, pos.data()))
        {
            float* src = pos.data();

            for (unsigned int x = 0; runningFlag() && (x < d->tempImage.width()); ++x, ++d->loop)
            {
                d->tempImage.setPixelColor(x, y, m_destImage.getSubPixelColor(src[0], src[1]));
                src += 2;
            }
        }
    }
}

void LensFunFilter::filterImage()
{
    m_destImage.bitBltImage(&m_orgImage, 0, 0);

    if (!d->iface)
    {
        qCDebug(DIGIKAM_DIMG_LOG) << "ERROR: LensFun Interface is null.";
        return;
    }

    if (!d->iface->usedLens())
    {
        qCDebug(DIGIKAM_DIMG_LOG) << "ERROR: LensFun Interface Lens device is null.";
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
        qCDebug(DIGIKAM_DIMG_LOG) << "ERROR: cannot initialize LensFun Modifier.";
        return;
    }

    // Calc necessary steps for progress bar

    int steps = ((d->iface->settings().filterCCA)                                   ? 1 : 0) +
                ((d->iface->settings().filterVIG)                                   ? 1 : 0) +
                ((d->iface->settings().filterDST || d->iface->settings().filterGEO) ? 1 : 0);

    qCDebug(DIGIKAM_DIMG_LOG) << "LensFun Modifier Flags: " << modflags << "  Steps:" << steps;

    if (steps < 1)
    {
        qCDebug(DIGIKAM_DIMG_LOG) << "No LensFun Modifier steps. There is nothing to process...";
        return;
    }

    qCDebug(DIGIKAM_DIMG_LOG) << "Image size to process: ("
                              << m_orgImage.width()  << ", "
                              << m_orgImage.height() << ")";

    QList<int> vals = multithreadedSteps(m_destImage.height());

    // Stage 1: Chromatic Aberation Corrections

    if (d->iface->settings().filterCCA)
    {
        m_orgImage.prepareSubPixelAccess(); // init lanczos kernel

        QList <QFuture<void> > tasks;

        for (int j = 0 ; runningFlag() && (j < vals.count()-1) ; ++j)
        {
            tasks.append(QtConcurrent::run(this,
                                           &LensFunFilter::filterCCAMultithreaded,
                                           vals[j],
                                           vals[j+1]));
        }

        foreach(QFuture<void> t, tasks)
            t.waitForFinished();

        qCDebug(DIGIKAM_DIMG_LOG) << "Chromatic Aberation Corrections applied.";
    }

    postProgress(30);

    // Stage 2: Color Corrections: Vignetting and Color Contribution Index

    if (d->iface->settings().filterVIG)
    {
        QList <QFuture<void> > tasks;

        for (int j = 0 ; runningFlag() && (j < vals.count()-1) ; ++j)
        {
            tasks.append(QtConcurrent::run(this,
                                           &LensFunFilter::filterVIGMultithreaded,
                                           vals[j],
                                           vals[j+1]));
        }

        foreach(QFuture<void> t, tasks)
            t.waitForFinished();

        qCDebug(DIGIKAM_DIMG_LOG) << "Vignetting and Color Corrections applied.";
    }

    postProgress(60);

    // Stage 3: Distortion and Geometry Corrections

    if (d->iface->settings().filterDST || d->iface->settings().filterGEO)
    {
        d->loop = 0;

        // we need a deep copy first
        d->tempImage = DImg(m_destImage.width(),
                            m_destImage.height(),
                            m_destImage.sixteenBit(),
                            m_destImage.hasAlpha());

        m_destImage.prepareSubPixelAccess(); // init lanczos kernel

        QList <QFuture<void> > tasks;

        for (int j = 0 ; runningFlag() && (j < vals.count()-1) ; ++j)
        {
            tasks.append(QtConcurrent::run(this,
                                           &LensFunFilter::filterDSTMultithreaded,
                                           vals[j],
                                           vals[j+1]));
        }

        foreach(QFuture<void> t, tasks)
            t.waitForFinished();

        qCDebug(DIGIKAM_DIMG_LOG) << "Distortion and Geometry Corrections applied.";

        if (d->loop)
        {
            m_destImage = d->tempImage;
        }
    }

    postProgress(90);
}

bool LensFunFilter::registerSettingsToXmp(MetaEngineData& data) const
{
    // Register in digiKam Xmp namespace all information about Lens corrections.

    QString str;
    LensFunContainer prm = d->iface->settings();

    str.append(i18n("Camera: %1-%2",        prm.cameraMake, prm.cameraModel));
    str.append(QLatin1String("\n"));
    str.append(i18n("Lens: %1",             prm.lensModel));
    str.append(QLatin1String("\n"));
    str.append(i18n("Subject Distance: %1", QString::number(prm.subjectDistance)));
    str.append(QLatin1String("\n"));
    str.append(i18n("Aperture: %1",         QString::number(prm.aperture)));
    str.append(QLatin1String("\n"));
    str.append(i18n("Focal Length: %1",     QString::number(prm.focalLength)));
    str.append(QLatin1String("\n"));
    str.append(i18n("Crop Factor: %1",      QString::number(prm.cropFactor)));
    str.append(QLatin1String("\n"));
    str.append(i18n("CCA Correction: %1",   prm.filterCCA  && d->iface->supportsCCA()       ? i18n("enabled") : i18n("disabled")));
    str.append(QLatin1String("\n"));
    str.append(i18n("VIG Correction: %1",   prm.filterVIG  && d->iface->supportsVig()       ? i18n("enabled") : i18n("disabled")));
    str.append(QLatin1String("\n"));
    str.append(i18n("DST Correction: %1",   prm.filterDST && d->iface->supportsDistortion() ? i18n("enabled") : i18n("disabled")));
    str.append(QLatin1String("\n"));
    str.append(i18n("GEO Correction: %1",   prm.filterGEO && d->iface->supportsGeometry()   ? i18n("enabled") : i18n("disabled")));

    DMetadata meta(data);
    bool ret = meta.setXmpTagString("Xmp.digiKam.LensCorrectionSettings",
                                    str.replace(QLatin1Char('\n'), QLatin1String(" ; ")));
    data     = meta.data();

    return ret;
}

FilterAction LensFunFilter::filterAction()
{
    FilterAction action(FilterIdentifier(), CurrentVersion());
    action.setDisplayableName(DisplayableName());

    LensFunContainer prm = d->iface->settings();
    action.addParameter(QLatin1String("ccaCorrection"),   prm.filterCCA);
    action.addParameter(QLatin1String("vigCorrection"),   prm.filterVIG);
    action.addParameter(QLatin1String("dstCorrection"),   prm.filterDST);
    action.addParameter(QLatin1String("geoCorrection"),   prm.filterGEO);
    action.addParameter(QLatin1String("cropFactor"),      prm.cropFactor);
    action.addParameter(QLatin1String("focalLength"),     prm.focalLength);
    action.addParameter(QLatin1String("aperture"),        prm.aperture);
    action.addParameter(QLatin1String("subjectDistance"), prm.subjectDistance);
    action.addParameter(QLatin1String("cameraMake"),      prm.cameraMake);
    action.addParameter(QLatin1String("cameraModel"),     prm.cameraModel);
    action.addParameter(QLatin1String("lensModel"),       prm.lensModel);

    return action;
}

void LensFunFilter::readParameters(const Digikam::FilterAction& action)
{
    LensFunContainer prm = d->iface->settings();
    prm.filterCCA        = action.parameter(QLatin1String("ccaCorrection")).toBool();
    prm.filterVIG        = action.parameter(QLatin1String("vigCorrection")).toBool();
    prm.filterDST        = action.parameter(QLatin1String("dstCorrection")).toBool();
    prm.filterGEO        = action.parameter(QLatin1String("geoCorrection")).toBool();
    prm.cropFactor       = action.parameter(QLatin1String("cropFactor")).toDouble();
    prm.focalLength      = action.parameter(QLatin1String("focalLength")).toDouble();
    prm.aperture         = action.parameter(QLatin1String("aperture")).toDouble();
    prm.subjectDistance  = action.parameter(QLatin1String("subjectDistance")).toDouble();
    prm.cameraMake       = action.parameter(QLatin1String("cameraMake")).toString();
    prm.cameraModel      = action.parameter(QLatin1String("cameraModel")).toString();
    prm.lensModel        = action.parameter(QLatin1String("lensModel")).toString();
    d->iface->setSettings(prm);
}

}  // namespace Digikam
