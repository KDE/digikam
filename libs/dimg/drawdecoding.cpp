/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-08-06
 * Description : Raw decoding settings for digiKam:
 *               standard libkdcraw parameters plus
 *               few customized for post processing.
 *
 * Copyright (C) 2008-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "drawdecoding.h"

namespace Digikam
{

DRawDecoding::DRawDecoding()
{
    resetPostProcessingSettings();
}

DRawDecoding::DRawDecoding(const RawDecodingSettings& prm)
{
    sixteenBitsImage        = prm.sixteenBitsImage;
    whiteBalance            = prm.whiteBalance;
    customWhiteBalance      = prm.customWhiteBalance;
    customWhiteBalanceGreen = prm.customWhiteBalanceGreen;
    RGBInterpolate4Colors   = prm.RGBInterpolate4Colors;
    unclipColors            = prm.unclipColors;
    DontStretchPixels       = prm.DontStretchPixels;
    enableNoiseReduction    = prm.enableNoiseReduction;
    medianFilterPasses      = prm.medianFilterPasses;
    NRThreshold             = prm.NRThreshold;
    enableCACorrection      = prm.enableCACorrection;
    caMultiplier[0]         = prm.caMultiplier[0];
    caMultiplier[1]         = prm.caMultiplier[1];
    RAWQuality              = prm.RAWQuality;
    inputColorSpace         = prm.inputColorSpace;
    outputColorSpace        = prm.outputColorSpace;
    inputProfile            = prm.inputProfile;
    outputProfile           = prm.outputProfile;
    autoBrightness          = prm.autoBrightness;    

    resetPostProcessingSettings();
}

DRawDecoding::~DRawDecoding()
{
}

void DRawDecoding::optimizeTimeLoading()
{
    KDcrawIface::RawDecodingSettings::optimizeTimeLoading();
    resetPostProcessingSettings();
}

void DRawDecoding::resetPostProcessingSettings()
{
    lightness    = 0.0;
    contrast     = 1.0;
    gamma        = 1.0;
    saturation   = 1.0;
    exposureComp = 0.0;
    curveAdjust  = QPolygon();
}

bool DRawDecoding::postProcessingSettingsIsDirty()
{
    return (lightness    != 0.0    ||
            contrast     != 1.0    ||
            gamma        != 1.0    ||
            saturation   != 1.0    ||
            exposureComp != 0.0    ||
            !curveAdjust.isEmpty());
}

bool DRawDecoding::operator==(const DRawDecoding& other) const
{
    return lightness     == other.lightness    &&
           contrast      == other.contrast     &&
           gamma         == other.gamma        &&
           saturation    == other.saturation   &&
           exposureComp  == other.exposureComp &&
           curveAdjust   == other.curveAdjust;
}

}  // namespace Digikam
