/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-12-09
 * Description : Raw decoding settings
 *
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2007-2008 by Guillaume Castagnino <casta at xwing dot info>
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

#include "drawdecodersettings.h"

namespace Digikam
{

DRawDecoderSettings::DRawDecoderSettings()
{
    fixColorsHighlights        = false;
    autoBrightness             = true;
    sixteenBitsImage           = false;
    brightness                 = 1.0;
    RAWQuality                 = BILINEAR;
    inputColorSpace            = NOINPUTCS;
    outputColorSpace           = SRGB;
    RGBInterpolate4Colors      = false;
    DontStretchPixels          = false;
    unclipColors               = 0;
    whiteBalance               = CAMERA;
    customWhiteBalance         = 6500;
    customWhiteBalanceGreen    = 1.0;
    medianFilterPasses         = 0;

    halfSizeColorImage         = false;

    enableBlackPoint           = false;
    blackPoint                 = 0;

    enableWhitePoint           = false;
    whitePoint                 = 0;

    NRType                     = NONR;
    NRThreshold                = 0;

    enableCACorrection         = false;
    caMultiplier[0]            = 0.0;
    caMultiplier[1]            = 0.0;

    inputProfile               = QString();
    outputProfile              = QString();

    deadPixelMap               = QString();

    whiteBalanceArea           = QRect();

    //-- Extended demosaicing settings ----------------------------------------------------------

    dcbIterations              = -1;
    dcbEnhanceFl               = false;
    eeciRefine                 = false;
    esMedPasses                = 0;
    NRChroThreshold            = 0;
    expoCorrection             = false;
    expoCorrectionShift        = 1.0;
    expoCorrectionHighlight    = 0.0;
}

DRawDecoderSettings::~DRawDecoderSettings()
{
}

DRawDecoderSettings& DRawDecoderSettings::operator=(const DRawDecoderSettings& o)
{
    fixColorsHighlights     = o.fixColorsHighlights;
    autoBrightness          = o.autoBrightness;
    sixteenBitsImage        = o.sixteenBitsImage;
    brightness              = o.brightness;
    RAWQuality              = o.RAWQuality;
    inputColorSpace         = o.inputColorSpace;
    outputColorSpace        = o.outputColorSpace;
    RGBInterpolate4Colors   = o.RGBInterpolate4Colors;
    DontStretchPixels       = o.DontStretchPixels;
    unclipColors            = o.unclipColors;
    whiteBalance            = o.whiteBalance;
    customWhiteBalance      = o.customWhiteBalance;
    customWhiteBalanceGreen = o.customWhiteBalanceGreen;
    halfSizeColorImage      = o.halfSizeColorImage;
    enableBlackPoint        = o.enableBlackPoint;
    blackPoint              = o.blackPoint;
    enableWhitePoint        = o.enableWhitePoint;
    whitePoint              = o.whitePoint;
    NRType                  = o.NRType;
    NRThreshold             = o.NRThreshold;
    enableCACorrection      = o.enableCACorrection;
    caMultiplier[0]         = o.caMultiplier[0];
    caMultiplier[1]         = o.caMultiplier[1];
    medianFilterPasses      = o.medianFilterPasses;
    inputProfile            = o.inputProfile;
    outputProfile           = o.outputProfile;
    deadPixelMap            = o.deadPixelMap;
    whiteBalanceArea        = o.whiteBalanceArea;

    //-- Extended demosaicing settings ----------------------------------------------------------

    dcbIterations           = o.dcbIterations;
    dcbEnhanceFl            = o.dcbEnhanceFl;
    eeciRefine              = o.eeciRefine;
    esMedPasses             = o.esMedPasses;
    NRChroThreshold         = o.NRChroThreshold;
    expoCorrection          = o.expoCorrection;
    expoCorrectionShift     = o.expoCorrectionShift;
    expoCorrectionHighlight = o.expoCorrectionHighlight;

    return *this;
}

bool DRawDecoderSettings::operator==(const DRawDecoderSettings& o) const
{
    return fixColorsHighlights     == o.fixColorsHighlights
        && autoBrightness          == o.autoBrightness
        && sixteenBitsImage        == o.sixteenBitsImage
        && brightness              == o.brightness
        && RAWQuality              == o.RAWQuality
        && inputColorSpace         == o.inputColorSpace
        && outputColorSpace        == o.outputColorSpace
        && RGBInterpolate4Colors   == o.RGBInterpolate4Colors
        && DontStretchPixels       == o.DontStretchPixels
        && unclipColors            == o.unclipColors
        && whiteBalance            == o.whiteBalance
        && customWhiteBalance      == o.customWhiteBalance
        && customWhiteBalanceGreen == o.customWhiteBalanceGreen
        && halfSizeColorImage      == o.halfSizeColorImage
        && enableBlackPoint        == o.enableBlackPoint
        && blackPoint              == o.blackPoint
        && enableWhitePoint        == o.enableWhitePoint
        && whitePoint              == o.whitePoint
        && NRType                  == o.NRType
        && NRThreshold             == o.NRThreshold
        && enableCACorrection      == o.enableCACorrection
        && caMultiplier[0]         == o.caMultiplier[0]
        && caMultiplier[1]         == o.caMultiplier[1]
        && medianFilterPasses      == o.medianFilterPasses
        && inputProfile            == o.inputProfile
        && outputProfile           == o.outputProfile
        && deadPixelMap            == o.deadPixelMap
        && whiteBalanceArea        == o.whiteBalanceArea

        //-- Extended demosaicing settings ----------------------------------------------------------

        && dcbIterations           == o.dcbIterations
        && dcbEnhanceFl            == o.dcbEnhanceFl
        && eeciRefine              == o.eeciRefine
        && esMedPasses             == o.esMedPasses
        && NRChroThreshold         == o.NRChroThreshold
        && expoCorrection          == o.expoCorrection
        && expoCorrectionShift     == o.expoCorrectionShift
        && expoCorrectionHighlight == o.expoCorrectionHighlight
        ;
}

void DRawDecoderSettings::optimizeTimeLoading()
{
    fixColorsHighlights     = false;
    autoBrightness          = true;
    sixteenBitsImage        = true;
    brightness              = 1.0;
    RAWQuality              = BILINEAR;
    inputColorSpace         = NOINPUTCS;
    outputColorSpace        = SRGB;
    RGBInterpolate4Colors   = false;
    DontStretchPixels       = false;
    unclipColors            = 0;
    whiteBalance            = CAMERA;
    customWhiteBalance      = 6500;
    customWhiteBalanceGreen = 1.0;
    halfSizeColorImage      = true;
    medianFilterPasses      = 0;

    enableBlackPoint        = false;
    blackPoint              = 0;

    enableWhitePoint        = false;
    whitePoint              = 0;

    NRType                  = NONR;
    NRThreshold             = 0;

    enableCACorrection      = false;
    caMultiplier[0]         = 0.0;
    caMultiplier[1]         = 0.0;

    inputProfile            = QString();
    outputProfile           = QString();

    deadPixelMap            = QString();

    whiteBalanceArea        = QRect();

    //-- Extended demosaicing settings ----------------------------------------------------------

    dcbIterations           = -1;
    dcbEnhanceFl            = false;
    eeciRefine              = false;
    esMedPasses             = 0;
    NRChroThreshold         = 0;
    expoCorrection          = false;
    expoCorrectionShift     = 1.0;
    expoCorrectionHighlight = 0.0;
}

QDebug operator<<(QDebug dbg, const DRawDecoderSettings& s)
{
    dbg.nospace() << endl;
    dbg.nospace() << "-- RAW DECODING SETTINGS --------------------------------" << endl;
    dbg.nospace() << "-- autoBrightness:          " << s.autoBrightness          << endl;
    dbg.nospace() << "-- sixteenBitsImage:        " << s.sixteenBitsImage        << endl;
    dbg.nospace() << "-- brightness:              " << s.brightness              << endl;
    dbg.nospace() << "-- RAWQuality:              " << s.RAWQuality              << endl;
    dbg.nospace() << "-- inputColorSpace:         " << s.inputColorSpace         << endl;
    dbg.nospace() << "-- outputColorSpace:        " << s.outputColorSpace        << endl;
    dbg.nospace() << "-- RGBInterpolate4Colors:   " << s.RGBInterpolate4Colors   << endl;
    dbg.nospace() << "-- DontStretchPixels:       " << s.DontStretchPixels       << endl;
    dbg.nospace() << "-- unclipColors:            " << s.unclipColors            << endl;
    dbg.nospace() << "-- whiteBalance:            " << s.whiteBalance            << endl;
    dbg.nospace() << "-- customWhiteBalance:      " << s.customWhiteBalance      << endl;
    dbg.nospace() << "-- customWhiteBalanceGreen: " << s.customWhiteBalanceGreen << endl;
    dbg.nospace() << "-- halfSizeColorImage:      " << s.halfSizeColorImage      << endl;
    dbg.nospace() << "-- enableBlackPoint:        " << s.enableBlackPoint        << endl;
    dbg.nospace() << "-- blackPoint:              " << s.blackPoint              << endl;
    dbg.nospace() << "-- enableWhitePoint:        " << s.enableWhitePoint        << endl;
    dbg.nospace() << "-- whitePoint:              " << s.whitePoint              << endl;
    dbg.nospace() << "-- NoiseReductionType:      " << s.NRType                  << endl;
    dbg.nospace() << "-- NoiseReductionThreshold: " << s.NRThreshold             << endl;
    dbg.nospace() << "-- enableCACorrection:      " << s.enableCACorrection      << endl;
    dbg.nospace() << "-- caMultiplier:            " << s.caMultiplier[0]
                  << ", "                           << s.caMultiplier[1]         << endl;
    dbg.nospace() << "-- medianFilterPasses:      " << s.medianFilterPasses      << endl;
    dbg.nospace() << "-- inputProfile:            " << s.inputProfile            << endl;
    dbg.nospace() << "-- outputProfile:           " << s.outputProfile           << endl;
    dbg.nospace() << "-- deadPixelMap:            " << s.deadPixelMap            << endl;
    dbg.nospace() << "-- whiteBalanceArea:        " << s.whiteBalanceArea        << endl;

    //-- Extended demosaicing settings ----------------------------------------------------------

    dbg.nospace() << "-- dcbIterations:           " << s.dcbIterations           << endl;
    dbg.nospace() << "-- dcbEnhanceFl:            " << s.dcbEnhanceFl            << endl;
    dbg.nospace() << "-- eeciRefine:              " << s.eeciRefine              << endl;
    dbg.nospace() << "-- esMedPasses:             " << s.esMedPasses             << endl;
    dbg.nospace() << "-- NRChrominanceThreshold:  " << s.NRChroThreshold         << endl;
    dbg.nospace() << "-- expoCorrection:          " << s.expoCorrection          << endl;
    dbg.nospace() << "-- expoCorrectionShift:     " << s.expoCorrectionShift     << endl;
    dbg.nospace() << "-- expoCorrectionHighlight: " << s.expoCorrectionHighlight << endl;
    dbg.nospace() << "---------------------------------------------------------" << endl;

    return dbg.space();
}

}  // namespace Digikam
