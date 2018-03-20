/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-03-06
 * Description : black and white image filter.
 *
 * Copyright (C) 2005-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2010 by Martin Klapetek <martin dot klapetek at gmail dot com>
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

#include "bwsepiafilter.h"

// C++ includes

#include <cstdio>
#include <cmath>

// Local includes

#include "dimg.h"
#include "mixerfilter.h"
#include "infraredfilter.h"

namespace Digikam
{

class BWSepiaFilter::Private
{
public:

    Private() :
        redAttn(0.0),
        greenAttn(0.0),
        blueAttn(0.0),
        redMult(0.0),
        greenMult(0.0),
        blueMult(0.0),
        settings(false)
    {
    }

    // Color filter attenuation in percents.
    double           redAttn;
    double           greenAttn;
    double           blueAttn;

    // Channel mixer color multiplier.
    double           redMult;
    double           greenMult;
    double           blueMult;

    BWSepiaContainer settings;
};

BWSepiaFilter::BWSepiaFilter(QObject* const parent)
    : DImgThreadedFilter(parent),
      d(new Private)
{
    initFilter();
}

BWSepiaFilter::BWSepiaFilter(DImg* const orgImage, QObject* const parent, const BWSepiaContainer& settings)
    : DImgThreadedFilter(orgImage, parent, QLatin1String("BWSepiaFilter")),
      d(new Private)
{
    d->settings = settings;
    initFilter();
}

BWSepiaFilter::~BWSepiaFilter()
{
    cancelFilter();
    delete d;
}

void BWSepiaFilter::filterImage()
{
    if (d->settings.preview)
    {
        m_destImage = getThumbnailForEffect(m_orgImage);
    }
    else
    {
        m_destImage = m_orgImage;
        postProgress(10);

        // Apply black and white filter.

        blackAndWhiteConversion(m_destImage, d->settings.filterType);
        postProgress(20);

        // Apply black and white film type.

        blackAndWhiteConversion(m_destImage, d->settings.filmType);
        postProgress(30);

        // Apply color tone filter.

        blackAndWhiteConversion(m_destImage, d->settings.toneType);
        postProgress(40);

        // Calculate and apply the luminosity curve on image.

        CurvesFilter curves(&m_destImage, 0L, d->settings.curvesPrm);
        postProgress(50);
        curves.startFilterDirectly();
        postProgress(60);
        m_destImage.putImageData(curves.getTargetImage().bits());
        postProgress(70);

        // Adjust contrast.

        BCGFilter bcg(&m_destImage, 0L, d->settings.bcgPrm);
        postProgress(80);
        bcg.startFilterDirectly();
        postProgress(90);
        m_destImage.putImageData(bcg.getTargetImage().bits());
        postProgress(100);
    }
}

DImg BWSepiaFilter::getThumbnailForEffect(DImg& img)
{
    postProgress(10);
    DImg thumb = img.copy();

    postProgress(25);

    if (d->settings.previewType < BWSepiaContainer::BWGeneric)
    {
        // In Filter view, we will render a preview of the B&W filter with the generic B&W film.

        blackAndWhiteConversion(thumb, d->settings.previewType);
        postProgress(50);

        blackAndWhiteConversion(thumb, BWSepiaContainer::BWGeneric);
        postProgress(75);
    }
    else
    {
        // In Film and Tone view, we will render the preview without to use the B&W Filter.

        postProgress(50);

        blackAndWhiteConversion(thumb, d->settings.previewType);
        postProgress(75);
    }

    postProgress(90);

    return (thumb);
}

void BWSepiaFilter::blackAndWhiteConversion(DImg& img, int type)
{
    TonalityContainer toneSettings;

    switch (type)
    {
        case BWSepiaContainer::BWNoFilter:
        {
            d->redAttn   = 0.0;
            d->greenAttn = 0.0;
            d->blueAttn  = 0.0;
            break;
        }

        case BWSepiaContainer::BWGreenFilter:
        {
            d->redAttn   = -0.20 * d->settings.strength;
            d->greenAttn = +0.11 * d->settings.strength;
            d->blueAttn  = +0.09 * d->settings.strength;
            break;
        }

        case BWSepiaContainer::BWOrangeFilter:
        {
            d->redAttn   = +0.48 * d->settings.strength;
            d->greenAttn = -0.37 * d->settings.strength;
            d->blueAttn  = -0.11 * d->settings.strength;
            break;
        }

        case BWSepiaContainer::BWRedFilter:
        {
            d->redAttn   = +0.60 * d->settings.strength;
            d->greenAttn = -0.49 * d->settings.strength;
            d->blueAttn  = -0.11 * d->settings.strength;
            break;
        }

        case BWSepiaContainer::BWYellowFilter:
        {
            d->redAttn   = +0.30 * d->settings.strength;
            d->greenAttn = -0.31 * d->settings.strength;
            d->blueAttn  = +0.01 * d->settings.strength;
            break;
        }

        case BWSepiaContainer::BWYellowGreenFilter:
        {
            d->redAttn   = +0.25 * d->settings.strength;
            d->greenAttn = +0.65 * d->settings.strength;
            d->blueAttn  = +0.15 * d->settings.strength;
            break;
        }

        case BWSepiaContainer::BWBlueFilter:
        {
            d->redAttn   = +0.15 * d->settings.strength;
            d->greenAttn = +0.15 * d->settings.strength;
            d->blueAttn  = +0.80 * d->settings.strength;
            break;
        }

        // --------------------------------------------------------------------------------

        case BWSepiaContainer::BWGeneric:
        case BWSepiaContainer::BWNoTone:
        {
            d->redMult   = +0.24;
            d->greenMult = +0.68;
            d->blueMult  = +0.08;
            applyChannelMixer(img);
            break;
        }

        case BWSepiaContainer::BWAgfa200X:
        {
            d->redMult   = +0.18;
            d->greenMult = +0.41;
            d->blueMult  = +0.41;
            applyChannelMixer(img);
            break;
        }

        case BWSepiaContainer::BWAgfapan25:
        {
            d->redMult   = +0.25;
            d->greenMult = +0.39;
            d->blueMult  = +0.36;
            applyChannelMixer(img);
            break;
        }

        case BWSepiaContainer::BWAgfapan100:
        {
            d->redMult   = +0.21;
            d->greenMult = +0.40;
            d->blueMult  = +0.39;
            applyChannelMixer(img);
            break;
        }

        case BWSepiaContainer::BWAgfapan400:
        {
            d->redMult   = +0.20;
            d->greenMult = +0.41;
            d->blueMult  = +0.39;
            applyChannelMixer(img);
            break;
        }

        case BWSepiaContainer::BWIlfordDelta100:
        {
            d->redMult   = +0.21;
            d->greenMult = +0.42;
            d->blueMult  = +0.37;
            applyChannelMixer(img);
            break;
        }

        case BWSepiaContainer::BWIlfordDelta400:
        {
            d->redMult   = +0.22;
            d->greenMult = +0.42;
            d->blueMult  = +0.36;
            applyChannelMixer(img);
            break;
        }

        case BWSepiaContainer::BWIlfordDelta400Pro3200:
        {
            d->redMult   = +0.31;
            d->greenMult = +0.36;
            d->blueMult  = +0.33;
            applyChannelMixer(img);
            break;
        }

        case BWSepiaContainer::BWIlfordFP4:
        {
            d->redMult   = +0.28;
            d->greenMult = +0.41;
            d->blueMult  = +0.31;
            applyChannelMixer(img);
            break;
        }

        case BWSepiaContainer::BWIlfordHP5:
        {
            d->redMult   = +0.23;
            d->greenMult = +0.37;
            d->blueMult  = +0.40;
            applyChannelMixer(img);
            break;
        }

        case BWSepiaContainer::BWIlfordPanF:
        {
            d->redMult   = +0.33;
            d->greenMult = +0.36;
            d->blueMult  = +0.31;
            applyChannelMixer(img);
            break;
        }

        case BWSepiaContainer::BWIlfordXP2Super:
        {
            d->redMult   = +0.21;
            d->greenMult = +0.42;
            d->blueMult  = +0.37;
            applyChannelMixer(img);
            break;
        }

        case BWSepiaContainer::BWKodakTmax100:
        {
            d->redMult   = +0.24;
            d->greenMult = +0.37;
            d->blueMult  = +0.39;
            applyChannelMixer(img);
            break;
        }

        case BWSepiaContainer::BWKodakTmax400:
        {
            d->redMult   = +0.27;
            d->greenMult = +0.36;
            d->blueMult  = +0.37;
            applyChannelMixer(img);
            break;
        }

        case BWSepiaContainer::BWKodakTriX:
        {
            d->redMult   = +0.25;
            d->greenMult = +0.35;
            d->blueMult  = +0.40;
            applyChannelMixer(img);
            break;
        }

        // --------------------------------------------------------------------------------

        case BWSepiaContainer::BWIlfordSFX200:
        {
            d->redMult   = +0.4;
            d->greenMult = +2.1;
            d->blueMult  = -0.8;
            applyInfraredFilter(img, 200);
            break;
        }

        case BWSepiaContainer::BWIlfordSFX400:
        {
            d->redMult   = +0.4;
            d->greenMult = +2.1;
            d->blueMult  = -0.8;
            applyInfraredFilter(img, 400);
            break;
        }

        case BWSepiaContainer::BWIlfordSFX800:
        {
            d->redMult   = +0.4;
            d->greenMult = +2.1;
            d->blueMult  = -0.8;
            applyInfraredFilter(img, 800);
            break;
        }

        case BWSepiaContainer::BWKodakHIE:
        {
            d->redMult   = +1.0;
            d->greenMult = +1.0;
            d->blueMult  = -1.0;
            applyInfraredFilter(img, 100);
            break;
        }

        // --------------------------------------------------------------------------------

        case BWSepiaContainer::BWSepiaTone:
        {
            toneSettings.redMask   = 162;
            toneSettings.greenMask = 132;
            toneSettings.blueMask  = 101;
            applyToneFilter(img, toneSettings);
            break;
        }

        case BWSepiaContainer::BWBrownTone:
        {
            toneSettings.redMask   = 129;
            toneSettings.greenMask = 115;
            toneSettings.blueMask  = 104;
            applyToneFilter(img, toneSettings);
            break;
        }

        case BWSepiaContainer::BWColdTone:
        {
            toneSettings.redMask   = 102;
            toneSettings.greenMask = 109;
            toneSettings.blueMask  = 128;
            applyToneFilter(img, toneSettings);
            break;
        }

        case BWSepiaContainer::BWSeleniumTone:
        {
            toneSettings.redMask   = 122;
            toneSettings.greenMask = 115;
            toneSettings.blueMask  = 122;
            applyToneFilter(img, toneSettings);
            break;
        }

        case BWSepiaContainer::BWPlatinumTone:
        {
            toneSettings.redMask   = 115;
            toneSettings.greenMask = 110;
            toneSettings.blueMask  = 106;
            applyToneFilter(img, toneSettings);
            break;
        }

        case BWSepiaContainer::BWGreenTone:
        {
            toneSettings.redMask   = 125;
            toneSettings.greenMask = 125;
            toneSettings.blueMask  = 105;
            applyToneFilter(img, toneSettings);
            break;
        }
    }
}

void BWSepiaFilter::applyChannelMixer(DImg& img)
{
    MixerContainer settings;
    settings.bMonochrome    = true;
    settings.blackRedGain   = d->redMult   + d->redMult * d->redAttn;
    settings.blackGreenGain = d->greenMult + d->greenMult * d->greenAttn;
    settings.blackBlueGain  = d->blueMult  + d->blueMult * d->blueAttn;
    MixerFilter mixer(&img, 0L, settings);
    mixer.startFilterDirectly();
    img.putImageData(mixer.getTargetImage().bits());
}

void BWSepiaFilter::applyInfraredFilter(DImg& img, int sensibility)
{
    InfraredContainer settings;
    settings.sensibility = sensibility;
    settings.redGain     = d->redMult   + d->redMult * d->redAttn;
    settings.greenGain   = d->greenMult + d->greenMult * d->greenAttn;
    settings.blueGain    = d->blueMult  + d->blueMult * d->blueAttn;
    InfraredFilter infra(&img, 0L, settings);
    infra.startFilterDirectly();
    img.putImageData(infra.getTargetImage().bits());
}

void BWSepiaFilter::applyToneFilter(DImg& img, TonalityContainer& settings)
{
    // Value to multiply RGB 8 bits component of mask used by TonalityFilter.
    int mul            = img.sixteenBit() ? 255 : 1;
    settings.redMask   = settings.redMask   * mul;
    settings.greenMask = settings.greenMask * mul;
    settings.blueMask  = settings.blueMask  * mul;
    TonalityFilter tone(&img, 0L, settings);
    tone.startFilterDirectly();
    img.putImageData(tone.getTargetImage().bits());
}

FilterAction BWSepiaFilter::filterAction()
{
    DefaultFilterAction<BWSepiaFilter> action(d->settings.curvesPrm.isStoredLosslessly());
    action.setDisplayableName(DisplayableName());

    action.addParameter(QLatin1String("filmType"),    d->settings.filmType);
    action.addParameter(QLatin1String("filterType"),  d->settings.filterType);
    action.addParameter(QLatin1String("preview"),     d->settings.preview);
    action.addParameter(QLatin1String("previewType"), d->settings.previewType);
    action.addParameter(QLatin1String("strength"),    d->settings.strength);
    action.addParameter(QLatin1String("toneType"),    d->settings.toneType);

    // Version 2: BWKodakHIE added
    action.supportOlderVersionIf(1, d->settings.filmType < BWSepiaContainer::BWKodakHIE);

    d->settings.curvesPrm.writeToFilterAction(action);
    d->settings.bcgPrm.writeToFilterAction(action);

    return action;
}

void BWSepiaFilter::readParameters(const FilterAction& action)
{
    d->settings.filmType    = action.parameter(QLatin1String("filmType")).toInt();
    d->settings.filterType  = action.parameter(QLatin1String("filterType")).toInt();
    d->settings.preview     = action.parameter(QLatin1String("preview")).toBool();
    d->settings.previewType = action.parameter(QLatin1String("previewType")).toInt();
    d->settings.strength    = action.parameter(QLatin1String("strength")).toDouble();
    d->settings.toneType    = action.parameter(QLatin1String("toneType")).toInt();

    d->settings.curvesPrm   = CurvesContainer::fromFilterAction(action);
    d->settings.bcgPrm      = BCGContainer::fromFilterAction(action);
}


}  // namespace Digikam
