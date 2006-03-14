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

// C++ includes.
 
#include <cmath>

// Digikam includes.

#include <digikamheaders.h>

namespace DigikamNoiseReductionImagesPlugin
{

class NoiseReduction : public Digikam::DImgThreadedFilter
{

public:
    
    NoiseReduction(Digikam::DImg *orgImage, QObject *parent, 
                   double radius, double lsmooth, double effect, double texture, double sharp,
                   double csmooth, double lookahead, double gamma, double damping, double phase);
    ~NoiseReduction(){};
    
private:

    struct iir_param
    {
        double  B, b1, b2, b3, b0, r, q;
        double *p;
    } m_iir;

    int    m_clamp;
    
    double m_radius;
    double m_lsmooth;
    double m_csmooth;
    double m_effect;
    double m_lookahead;
    double m_gamma;
    double m_damping;
    double m_phase;
    double m_texture;
    double m_sharp;

private:  

    void filterImage(void);

    void iir_init(double r);
    void box_filter(double *src, double *end, double *dest, double radius);
    void iir_filter(float* const start, float* const end, float* dstart, double radius, const int type);
    void filter(float *buffer, float *data, float *data2, float *rbuf, float *tbuf, int width, int color);
    void blur_line(float* const data, float* const data2, float* const buffer,
                   float* rbuf, float* tbuf, const uchar *src, uchar *dest, int len);

    inline double bsqrt(double val)
    {
        if (val >= 0.0) return sqrt(val);
        else return -sqrt(-val);
    };

    inline double sq(double val){ return val*val; };

    inline double bsq(double val){ return fabs(val) * val; };

    inline double mypow(double val, double ex)
    {
        if (fabs(val) < 1e-16) return 0.0;
        if (val > 0.0) return exp(log(val)*ex);
        return -exp(log(-val)*ex);
    }; 

};

}  // NameSpace DigikamNoiseReductionImagesPlugin

#endif /* NOISE_REDUCTION_H */
