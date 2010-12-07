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
    rawPrm = prm;

    resetPostProcessingSettings();
}

DRawDecoding::~DRawDecoding()
{
}

void DRawDecoding::optimizeTimeLoading()
{
    rawPrm.optimizeTimeLoading();
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
    return rawPrm       == other.rawPrm       &&
           lightness    == other.lightness    &&
           contrast     == other.contrast     &&
           gamma        == other.gamma        &&
           saturation   == other.saturation   &&
           exposureComp == other.exposureComp &&
           curveAdjust  == other.curveAdjust;
}

DRawDecoding& DRawDecoding::operator=(const DRawDecoding& o)
{
    rawPrm       = o.rawPrm;
    lightness    = o.lightness;
    contrast     = o.contrast;
    gamma        = o.gamma;
    saturation   = o.saturation;
    exposureComp = o.exposureComp;
    curveAdjust  = o.curveAdjust;
    return *this;
}

}  // namespace Digikam
