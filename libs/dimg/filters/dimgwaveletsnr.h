/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-25
 * Description : Wavelets Noise Reduction threaded image filter.
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
  
#ifndef DIMGWAVELETSNR_H
#define DIMGWAVELETSNR_H

// C++ includes

#include <cmath>

// Local includes

#include "digikam_export.h"
#include "dimgthreadedfilter.h"

namespace Digikam
{

class DIGIKAM_EXPORT DImgWaveletsNR : public DImgThreadedFilter
{

public:

    enum ColorMode
    {
        MODE_YCBCR=0,
        MODE_LAB,
        MODE_RGB
    };

public:

    DImgWaveletsNR(DImg *orgImage, QObject *parent, double threshold, double softness);
    ~DImgWaveletsNR(){};

private:

    void filterImage();

    void waveletDenoise(float *fimg[3], unsigned int width, unsigned int height,
                        float threshold, double softness);
    inline void hatTransform(float *temp, float *base, int st, int size, int sc);

    void srgb2ycbcr(float** fimg, int size);
    void ycbcr2srgb(float** fimg, int size);
    void srgb2lab(float** fimg, int size);
    void lab2srgb(float** fimg, int size);
    void srgb2xyz(float** fimg, int size);
    void xyz2srgb(float** fimg, int size);

private:

    ColorMode m_colourMode;

    double    m_threshold;
    double    m_softness;

    float*    m_fimg[4];
    float*    m_buffer[3];
};

}  // namespace Digikam

#endif /* DIMGWAVELETSNR_H */
