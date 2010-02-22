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

// Local includes

#include "digikam_export.h"
#include "dimgthreadedfilter.h"
#include "globals.h"
#include "imagecurves.h"

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
        curves   = 0;
        type     = BWGeneric;
        preview  = false;
        strength = 1.0;
    };

    ~BWSepiaContainer(){};

public:

    bool         preview;

    int          type;

    double       strength;
    
    ImageCurves* curves;

};

// -----------------------------------------------------------------------------------------------

class DIGIKAM_EXPORT BWSepiaFilter : public DImgThreadedFilter
{

public:

    explicit BWSepiaFilter(DImg* orgImage, QObject* parent=0, const BWSepiaContainer& settings=BWSepiaContainer());
    BWSepiaFilter(uchar* bits, uint width, uint height, bool sixteenBits, 
                  const BWSepiaContainer& settings=BWSepiaContainer());
    virtual ~BWSepiaFilter();

private:

    void filterImage();

    DImg getThumbnailForEffect(const DImg& img);
    DImg getThumbnailForEffect(uchar* data, int w, int h, bool sb);
    
    void blackAndWhiteConversion(int type);
    void blackAndWhiteConversion(uchar* data, int w, int h, bool sb, int type);
    void applyChannelMixer(uchar* data, int w, int h, bool sb);

private:

    BWSepiaFilterPriv* const d;
};

}  // namespace Digikam

#endif /* BWSEPIAFILTER_H */
