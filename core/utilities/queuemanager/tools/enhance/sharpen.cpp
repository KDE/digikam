/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-05-02
 * Description : sharpen image batch tool.
 *
 * Copyright (C) 2009      by Matthias Welwarsky <matze at welwarsky dot de>
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "sharpen.h"

// Qt includes

#include <QLabel>
#include <QWidget>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_config.h"
#include "dimg.h"
#include "sharpenfilter.h"
#include "unsharpmaskfilter.h"
#include "sharpsettings.h"

#ifdef HAVE_EIGEN3
#   include "refocusfilter.h"
#endif // HAVE_EIGEN3

namespace Digikam
{

Sharpen::Sharpen(QObject* const parent)
    : BatchTool(QLatin1String("Sharpen"), EnhanceTool, parent)
{
    m_settingsView = 0;
    setToolTitle(i18n("Sharpen Image"));
    setToolDescription(i18n("Sharpen images"));
    setToolIconName(QLatin1String("sharpenimage"));
}

Sharpen::~Sharpen()
{
}

void Sharpen::registerSettingsWidget()
{
    m_settingsWidget = new QWidget;
    m_settingsView   = new SharpSettings(m_settingsWidget);

    connect(m_settingsView, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

    BatchTool::registerSettingsWidget();
}

BatchToolSettings Sharpen::defaultSettings()
{
    BatchToolSettings settings;
    SharpContainer defaultPrm = m_settingsView->defaultSettings();

    // sharpen method
    settings.insert(QLatin1String("SharpenFilterType"),    (int)defaultPrm.method);

    // simple sharp
    settings.insert(QLatin1String("SimpleSharpRadius"),    (int)defaultPrm.ssRadius);

    // unsharp mask
    settings.insert(QLatin1String("UnsharpMaskRadius"),    (double)defaultPrm.umRadius);
    settings.insert(QLatin1String("UnsharpMaskAmount"),    (double)defaultPrm.umAmount);
    settings.insert(QLatin1String("UnsharpMaskThreshold"), (double)defaultPrm.umThreshold);
    settings.insert(QLatin1String("UnsharpMaskLuma"),      (bool)defaultPrm.umLumaOnly);

#ifdef HAVE_EIGEN3
    // refocus
    settings.insert(QLatin1String("RefocusRadius"),        (double)defaultPrm.rfRadius);
    settings.insert(QLatin1String("RefocusCorrelation"),   (double)defaultPrm.rfCorrelation);
    settings.insert(QLatin1String("RefocusNoise"),         (double)defaultPrm.rfNoise);
    settings.insert(QLatin1String("RefocusGauss"),         (double)defaultPrm.rfGauss);
    settings.insert(QLatin1String("RefocusMatrixSize"),    (int)defaultPrm.rfMatrix);
#endif // HAVE_EIGEN3

    return settings;
}

void Sharpen::slotAssignSettings2Widget()
{
    SharpContainer prm;

    // sharpen method
    prm.method        = settings()[QLatin1String("SharpenFilterType")].toInt();

    // simple sharp
    prm.ssRadius      = settings()[QLatin1String("SimpleSharpRadius")].toInt();

    // unsharp mask
    prm.umRadius      = settings()[QLatin1String("UnsharpMaskRadius")].toDouble();
    prm.umAmount      = settings()[QLatin1String("UnsharpMaskAmount")].toDouble();
    prm.umThreshold   = settings()[QLatin1String("UnsharpMaskThreshold")].toDouble();
    prm.umLumaOnly    = settings()[QLatin1String("UnsharpMaskLuma")].toBool();

#ifdef HAVE_EIGEN3
    // refocus
    prm.rfRadius      = settings()[QLatin1String("RefocusRadius")].toDouble();
    prm.rfCorrelation = settings()[QLatin1String("RefocusCorrelation")].toDouble();
    prm.rfNoise       = settings()[QLatin1String("RefocusNoise")].toDouble();
    prm.rfGauss       = settings()[QLatin1String("RefocusGauss")].toDouble();
    prm.rfMatrix      = settings()[QLatin1String("RefocusMatrixSize")].toInt();
#endif // HAVE_EIGEN3

    m_settingsView->setSettings(prm);
}

void Sharpen::slotSettingsChanged()
{
    BatchToolSettings settings;
    SharpContainer prm = m_settingsView->settings();

    // sharpen method
    settings.insert(QLatin1String("SharpenFilterType"),    (int)prm.method);

    // simple sharp
    settings.insert(QLatin1String("SimpleSharpRadius"),    (int)prm.ssRadius);

    // unsharp mask
    settings.insert(QLatin1String("UnsharpMaskRadius"),    (double)prm.umRadius);
    settings.insert(QLatin1String("UnsharpMaskAmount"),    (double)prm.umAmount);
    settings.insert(QLatin1String("UnsharpMaskThreshold"), (double)prm.umThreshold);
    settings.insert(QLatin1String("UnsharpMaskLuma"),      (bool)prm.umLumaOnly);

#ifdef HAVE_EIGEN3
    // refocus
    settings.insert(QLatin1String("RefocusRadius"),        (double)prm.rfRadius);
    settings.insert(QLatin1String("RefocusCorrelation"),   (double)prm.rfCorrelation);
    settings.insert(QLatin1String("RefocusNoise"),         (double)prm.rfNoise);
    settings.insert(QLatin1String("RefocusGauss"),         (double)prm.rfGauss);
    settings.insert(QLatin1String("RefocusMatrixSize"),    (int)prm.rfMatrix);
#endif // HAVE_EIGEN3

    BatchTool::slotSettingsChanged(settings);
}

bool Sharpen::toolOperations()
{
    if (!loadToDImg())
    {
        return false;
    }

    int filterType  = settings()[QLatin1String("SharpenFilterType")].toInt();

    switch (filterType)
    {
        case SharpContainer::SimpleSharp:
        {
            double radius = settings()[QLatin1String("SimpleSharpRadius")].toInt() / 10.0;
            double sigma;

            if (radius < 1.0)
            {
                sigma = radius;
            }
            else
            {
                sigma = sqrt(radius);
            }

            SharpenFilter filter(&image(), 0L, radius, sigma);
            applyFilter(&filter);
            break;
        }

        case SharpContainer::UnsharpMask:
        {
            double r     = settings()[QLatin1String("UnsharpMaskRadius")].toDouble();
            double a  = settings()[QLatin1String("UnsharpMaskAmount")].toDouble();
            double th = settings()[QLatin1String("UnsharpMaskThreshold")].toDouble();
            bool    l = settings()[QLatin1String("UnsharpMaskLuma")].toBool();

            UnsharpMaskFilter filter(&image(), 0L, r, a, th, l);
            applyFilter(&filter);
            break;
        }

        case SharpContainer::Refocus:
        {
#ifdef HAVE_EIGEN3
            double radius      = settings()[QLatin1String("RefocusRadius")].toDouble();
            double correlation = settings()[QLatin1String("RefocusCorrelation")].toDouble();
            double noise       = settings()[QLatin1String("RefocusNoise")].toDouble();
            double gauss       = settings()[QLatin1String("RefocusGauss")].toDouble();
            int matrixSize     = settings()[QLatin1String("RefocusMatrixSize")].toInt();

            RefocusFilter filter(&image(), 0L, matrixSize, radius, gauss, correlation, noise);
            applyFilter(&filter);
#endif // HAVE_EIGEN3
            break;
        }
    }

    return savefromDImg();
}

} // namespace Digikam
