/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-24-01
 * Description : Chanels mixer filter
 *
 * Copyright (C) 2005-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef MIXERFILTER_H
#define MIXERFILTER_H

// Local includes

#include "digikam_export.h"
#include "dimgthreadedfilter.h"
#include "globals.h"

using namespace Digikam;

namespace Digikam
{

class DImg;
class MixerFilterPriv;

class DIGIKAM_EXPORT MixerContainer
{

public:

    MixerContainer()
    {
        bPreserveLum = true;
        bMonochrome  = false;
        rrGain       = 0.0;
        rgGain       = 0.0;
        rbGain       = 0.0;
        grGain       = 0.0;
        ggGain       = 0.0;
        gbGain       = 0.0;
        brGain       = 0.0; 
        bgGain       = 0.0;
        bbGain       = 0.0;    
    };

    ~MixerContainer(){};

public:

    bool  bPreserveLum;
    bool  bMonochrome;
    
    float rrGain;
    float rgGain;
    float rbGain;
    float grGain;
    float ggGain;
    float gbGain;
    float brGain; 
    float bgGain;
    float bbGain;    
};

// -----------------------------------------------------------------------------------------------

class DIGIKAM_EXPORT MixerFilter : public DImgThreadedFilter
{

public:

    explicit MixerFilter(DImg* orgImage, QObject* parent=0, const MixerContainer& settings=MixerContainer());
    MixerFilter(uchar* bits, uint width, uint height, bool sixteenBits, 
                const MixerContainer& settings=MixerContainer());
    virtual ~MixerFilter();

private:

    void filterImage();

    void channelMixerImage(DImg& image);
    void channelMixerImage(uchar* data, uint width, uint height, bool sixteenBit);
    
    inline double CalculateNorm(float RedGain, float GreenGain, float BlueGain, bool bPreserveLum);

    inline unsigned short MixPixel(float RedGain, float GreenGain, float BlueGain,
                                   unsigned short R, unsigned short G, unsigned short B, bool sixteenBit,
                                   double Norm);

private:

    MixerContainer m_settings;
};

}  // namespace Digikam

#endif /* MIXERFILTER_H */
