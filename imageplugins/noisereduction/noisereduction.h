/* ============================================================
 * File  : noisereduction.h
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2005-05-25
 * Description : Noise Reduction threaded image filter.
 * 
 * Copyright 2005-2006 by Gilles Caulier
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
  
#ifndef NOISE_REDUCTION_H
#define NOISE_REDUCTION_H

// Digikam includes.

#include <digikamheaders.h>

namespace DigikamNoiseReductionImagesPlugin
{

class NoiseReduction : public Digikam::DImgThreadedFilter
{

public:
    
    NoiseReduction(Digikam::DImg *orgImage, QObject *parent, int radius, int black_level,
                   int white_level, bool adaptativeFilter=true, bool recursiveFilter=false);
    ~NoiseReduction(){};
    
private:  

    int  m_radius;
    int  m_black_level;
    int  m_white_level;
    bool m_adaptativeFilter;
    bool m_recursiveFilter; 

private:  

    void filterImage(void);

    void despeckle();
    void despeckle16();

    int  quick_median_select(uchar **p, uchar *i, int n);
    int  quick_median_select16(unsigned short **p, unsigned short *i, int n);

    uchar pixel_intensity(const uchar *p);
    unsigned short pixel_intensity16(const unsigned short *p);

};

}  // NameSpace DigikamNoiseReductionImagesPlugin

#endif /* NOISE_REDUCTION_H */
