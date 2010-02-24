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

#ifndef BWSEPIAFILTER_H
#define BWSEPIAFILTER_H

#include <QPolygon>

// Local includes

#include "digikam_export.h"
#include "dimgthreadedfilter.h"
#include "globals.h"
#include "bcgfilter.h"
#include "tonalityfilter.h"

using namespace Digikam;

namespace Digikam
{

class DImg;
class BWSepiaFilterPriv;

class DIGIKAM_EXPORT BWSepiaContainer
{

public:
  
    enum BlackWhiteConversionType
    {
        BWNoFilter=0,         // B&W filter to the front of lens.
        BWGreenFilter,
        BWOrangeFilter,
        BWRedFilter,
        BWYellowFilter,
        BWYellowGreenFilter,
        BWBlueFilter,

        BWGeneric,            // B&W film simulation.
        BWAgfa200X,
        BWAgfapan25,
        BWAgfapan100,
        BWAgfapan400,
        BWIlfordDelta100,
        BWIlfordDelta400,
        BWIlfordDelta400Pro3200,
        BWIlfordFP4,
        BWIlfordHP5,
        BWIlfordPanF,
        BWIlfordXP2Super,
        BWKodakTmax100,
        BWKodakTmax400,
        BWKodakTriX,

        BWIlfordSFX200,       // Infrared film simulation.
        BWIlfordSFX400,
        BWIlfordSFX800,

        BWNoTone,             // Chemical color tone filter.
        BWSepiaTone,
        BWBrownTone,
        BWColdTone,
        BWSeleniumTone,
        BWPlatinumTone,
        BWGreenTone
    };  
  
public:

    BWSepiaContainer()
    {
        previewType = BWGeneric;
        preview     = false;
        filmType    = BWGeneric;
        filterType  = BWNoFilter;
        toneType    = BWNoTone;
        strength    = 1.0;
    };

    BWSepiaContainer(int ptype)
    {
        previewType = ptype;
        preview     = true;
        strength    = 1.0;
        filmType    = BWGeneric;
        filterType  = BWNoFilter;
        toneType    = BWNoTone;
    };

    ~BWSepiaContainer(){};

public:

    bool         preview;

    int          previewType;
    int          filmType;
    int          filterType;
    int          toneType;

    double       strength;
    
    QPolygon     curvePts;

    BCGContainer bcgPrm;
};

// -----------------------------------------------------------------------------------------------

class DIGIKAM_EXPORT BWSepiaFilter : public DImgThreadedFilter
{

public:

    BWSepiaFilter(DImg* orgImage, QObject* parent=0, const BWSepiaContainer& settings=BWSepiaContainer());
    virtual ~BWSepiaFilter();

private:

    void filterImage();

    DImg getThumbnailForEffect(DImg& img);
    
    void blackAndWhiteConversion(DImg& img, int type);
    void applyChannelMixer(DImg& img);
    void applyInfraredFilter(DImg& img, int sensibility);
    void applyToneFilter(DImg& img, TonalityContainer& settings);

private:

    BWSepiaFilterPriv* const d;
};

}  // namespace Digikam

#endif /* BWSEPIAFILTER_H */
