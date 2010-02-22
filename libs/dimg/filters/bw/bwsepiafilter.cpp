/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-03-06
 * Description : black and white image filter.
 *
 * Copyright (C) 2005-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// KDE includes

#include <kdebug.h>

// Local includes

#include "dimg.h"
#include "tonalityfilter.h"
#include "mixerfilter.h"
#include "infraredfilter.h"
#include "imagehistogram.h"

namespace Digikam
{

class BWSepiaFilterPriv
{
public:

    BWSepiaFilterPriv()
    {
        redAttn   = 0.0;
        greenAttn = 0.0;
        blueAttn  = 0.0;
        redMult   = 0.0;
        greenMult = 0.0;
        blueMult  = 0.0;
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

BWSepiaFilter::BWSepiaFilter(DImg* orgImage, QObject* parent, const BWSepiaContainer& settings)
             : DImgThreadedFilter(orgImage, parent, "BWSepiaFilter"),
               d(new BWSepiaFilterPriv)
{
    d->settings = settings;
    initFilter();
}

BWSepiaFilter::BWSepiaFilter(uchar* bits, uint width, uint height, bool sixteenBits, const BWSepiaContainer& settings)
             : DImgThreadedFilter(),
               d(new BWSepiaFilterPriv)
{
    d->settings = settings;
    m_orgImage  = DImg(width, height, sixteenBits, true, bits, true);
    initFilter();
    filterImage();
}

BWSepiaFilter::~BWSepiaFilter()
{
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
        postProgress(10);
        
        // Apply black and white filter.

        blackAndWhiteConversion(m_orgImage.bits(), m_orgImage.width(), m_orgImage.height(), 
                                m_orgImage.sixteenBit(), d->settings.filterType);
        postProgress(20);
        
        // Apply black and white film type.

        blackAndWhiteConversion(m_orgImage.bits(), m_orgImage.width(), m_orgImage.height(), 
                                m_orgImage.sixteenBit(), d->settings.filmType + BWSepiaContainer::BWGeneric);
        postProgress(30);

        // Apply color tone filter.

        blackAndWhiteConversion(m_orgImage.bits(), m_orgImage.width(), m_orgImage.height(), 
                                m_orgImage.sixteenBit(), d->settings.toneType + BWSepiaContainer::BWNoTone);
        postProgress(40);

        // Calculate and apply the curve on image.

        uchar* targetData = new uchar[m_orgImage.numBytes()];
        postProgress(50);

        d->settings.curves->curvesLutSetup(AlphaChannel);
        postProgress(60);
        
        d->settings.curves->curvesLutProcess(m_orgImage.bits(), targetData, m_orgImage.width(), m_orgImage.height());
        postProgress(70);

        // Adjust contrast.

        m_destImage.putImageData(targetData);
        postProgress(80);

        BCGFilter bcg(&m_destImage, 0L, d->settings.bcgPrm);
        bcg.startFilterDirectly();
        m_destImage.putImageData(bcg.getTargetImage().bits());
        postProgress(90);
    }
}

DImg BWSepiaFilter::getThumbnailForEffect(const DImg& img)
{    
    return getThumbnailForEffect(img.bits(), img.width(), img.height(), img.sixteenBit());
}

DImg BWSepiaFilter::getThumbnailForEffect(uchar* data, int w, int h, bool sb)
{
    DImg thumb(w, h, sb, true, data);

    postProgress(10);

    if (d->settings.previewType < BWSepiaContainer::BWGeneric)
    {
        // In Filter view, we will render a preview of the B&W filter with the generic B&W film.
        blackAndWhiteConversion(thumb.bits(), w, h, sb, d->settings.previewType);
        postProgress(25);

        blackAndWhiteConversion(thumb.bits(), w, h, sb, BWSepiaContainer::BWGeneric);
        postProgress(50);
    }
    else
    {
        // In Film and Tone view, we will render the preview without to use the B&W Filter
        blackAndWhiteConversion(thumb.bits(), w, h, sb, d->settings.previewType);
        postProgress(50);
    }

    if (d->settings.curves)   // in case we're called before the creator is done
    {
        uchar* targetData = new uchar[w*h*(sb ? 8 : 4)];
        postProgress(60);

        d->settings.curves->curvesLutSetup(AlphaChannel);
        postProgress(70);
        
        d->settings.curves->curvesLutProcess(thumb.bits(), targetData, w, h);
        postProgress(80);

        DImg preview(w, h, sb, true, targetData);
        thumb.putImageData(preview.bits());
        postProgress(90);

        delete [] targetData;
    }
    
    return (thumb);
}

void BWSepiaFilter::blackAndWhiteConversion(int type)
{    
    return blackAndWhiteConversion(m_orgImage.bits(), m_orgImage.width(), m_orgImage.height(),
                                   m_orgImage.sixteenBit(), type);
}

void BWSepiaFilter::blackAndWhiteConversion(uchar* data, int w, int h, bool sb, int type)
{
    // Value to multiply RGB 8 bits component of mask used by TonalityFilter.
    int mul = sb ? 255 : 1;

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
            d->redMult   = 0.24;
            d->greenMult = 0.68;
            d->blueMult  = 0.08;
            applyChannelMixer(data, w, h, sb);
            break;
        }

        case BWSepiaContainer::BWAgfa200X:
        {
            d->redMult   = 0.18;
            d->greenMult = 0.41;
            d->blueMult  = 0.41;
            applyChannelMixer(data, w, h, sb);
            break;
        }

        case BWSepiaContainer::BWAgfapan25:
        {
            d->redMult   = 0.25;
            d->greenMult = 0.39;
            d->blueMult  = 0.36;
            applyChannelMixer(data, w, h, sb);
            break;
        }

        case BWSepiaContainer::BWAgfapan100:
        {
            d->redMult   = 0.21;
            d->greenMult = 0.40;
            d->blueMult  = 0.39;
            applyChannelMixer(data, w, h, sb);
            break;
        }

        case BWSepiaContainer::BWAgfapan400:
        {
            d->redMult   = 0.20;
            d->greenMult = 0.41;
            d->blueMult  = 0.39;
            applyChannelMixer(data, w, h, sb);
            break;
        }

        case BWSepiaContainer::BWIlfordDelta100:
        {
            d->redMult   = 0.21;
            d->greenMult = 0.42;
            d->blueMult  = 0.37;
            applyChannelMixer(data, w, h, sb);
            break;
        }

        case BWSepiaContainer::BWIlfordDelta400:
        {
            d->redMult   = 0.22;
            d->greenMult = 0.42;
            d->blueMult  = 0.36;
            applyChannelMixer(data, w, h, sb);
            break;
        }

        case BWSepiaContainer::BWIlfordDelta400Pro3200:
        {
            d->redMult   = 0.31;
            d->greenMult = 0.36;
            d->blueMult  = 0.33;
            applyChannelMixer(data, w, h, sb);
            break;
        }

        case BWSepiaContainer::BWIlfordFP4:
        {
            d->redMult   = 0.28;
            d->greenMult = 0.41;
            d->blueMult  = 0.31;
            applyChannelMixer(data, w, h, sb);
            break;
        }

        case BWSepiaContainer::BWIlfordHP5:
        {
            d->redMult   = 0.23;
            d->greenMult = 0.37;
            d->blueMult  = 0.40;
            applyChannelMixer(data, w, h, sb);
            break;
        }

        case BWSepiaContainer::BWIlfordPanF:
        {
            d->redMult   = 0.33;
            d->greenMult = 0.36;
            d->blueMult  = 0.31;
            applyChannelMixer(data, w, h, sb);
            break;
        }

        case BWSepiaContainer::BWIlfordXP2Super:
        {
            d->redMult   = 0.21;
            d->greenMult = 0.42;
            d->blueMult  = 0.37;
            applyChannelMixer(data, w, h, sb);
            break;
        }

        case BWSepiaContainer::BWKodakTmax100:
        {
            d->redMult   = 0.24;
            d->greenMult = 0.37;
            d->blueMult  = 0.39;
            applyChannelMixer(data, w, h, sb);
            break;
        }

        case BWSepiaContainer::BWKodakTmax400:
        {
            d->redMult   = 0.27;
            d->greenMult = 0.36;
            d->blueMult  = 0.37;
            applyChannelMixer(data, w, h, sb);
            break;
        }

        case BWSepiaContainer::BWKodakTriX:
        {
            d->redMult   = 0.25;
            d->greenMult = 0.35;
            d->blueMult  = 0.40;
            applyChannelMixer(data, w, h, sb);
            break;
        }

        // --------------------------------------------------------------------------------

        case BWSepiaContainer::BWIlfordSFX200:
        {
            InfraredFilter infra(data, w, h, sb, 200);
            break;
        }

        case BWSepiaContainer::BWIlfordSFX400:
        {
            InfraredFilter infra(data, w, h, sb, 400);
            break;
        }

        case BWSepiaContainer::BWIlfordSFX800:
        {
            InfraredFilter infra(data, w, h, sb, 800);
            break;
        }

        // --------------------------------------------------------------------------------

        case BWSepiaContainer::BWSepiaTone:
        {
            toneSettings.redMask   = 162*mul;
            toneSettings.greenMask = 132*mul;
            toneSettings.blueMask  = 101*mul;
            TonalityFilter tone(data, w, h, sb, toneSettings);
            break;
        }

        case BWSepiaContainer::BWBrownTone:
        {
            toneSettings.redMask   = 129*mul;
            toneSettings.greenMask = 115*mul;
            toneSettings.blueMask  = 104*mul;
            TonalityFilter tone(data, w, h, sb, toneSettings);
            break;
        }

        case BWSepiaContainer::BWColdTone:
        {
            toneSettings.redMask   = 102*mul;
            toneSettings.greenMask = 109*mul;
            toneSettings.blueMask  = 128*mul;
            TonalityFilter tone(data, w, h, sb, toneSettings);
            break;
        }

        case BWSepiaContainer::BWSeleniumTone:
        {
            toneSettings.redMask   = 122*mul;
            toneSettings.greenMask = 115*mul;
            toneSettings.blueMask  = 122*mul;
            TonalityFilter tone(data, w, h, sb, toneSettings);
            break;
        }

        case BWSepiaContainer::BWPlatinumTone:
        {
            toneSettings.redMask   = 115*mul;
            toneSettings.greenMask = 110*mul;
            toneSettings.blueMask  = 106*mul;
            TonalityFilter tone(data, w, h, sb, toneSettings);
            break;
        }

        case BWSepiaContainer::BWGreenTone:
        {
            toneSettings.redMask   = 125*mul;
            toneSettings.greenMask = 125*mul;
            toneSettings.blueMask  = 105*mul;
            TonalityFilter tone(data, w, h, sb, toneSettings);
            break;
        }
    }
}

void BWSepiaFilter::applyChannelMixer(uchar* data, int w, int h, bool sb)
{
    MixerContainer settings;
    settings.bMonochrome    = true;
    settings.blackRedGain   = d->redMult   + d->redMult*d->redAttn;
    settings.blackGreenGain = d->greenMult + d->greenMult*d->greenAttn;
    settings.blackBlueGain  = d->blueMult  + d->blueMult*d->blueAttn;
    MixerFilter mixer(data, w, h, sb, settings);
}

}  // namespace Digikam
