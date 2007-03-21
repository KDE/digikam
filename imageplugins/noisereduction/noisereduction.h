/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at gmail dot com>
 *          Peter Heckert <peter dot heckert at arcor dot de>
 * Date   : 2005-05-25
 * Description : Noise Reduction threaded image filter.
 * 
 * Copyright 2005-2007 by Gilles Caulier
 *
 * Original Noise Filter algorithm copyright (C) 2005 
 * Peter Heckert <peter dot heckert at arcor dot de>
 * from dcamnoise2 gimp plugin available at this url :
 * http://home.arcor.de/peter.heckert/dcamnoise2-0.63.c
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

#include "dimgthreadedfilter.h"

/**============= NOTICE TO USE THE FILTER ===============================================================
 *
 * Let me explain, how the filter works, some understanding is necessary to use it:
 *
 * Hint for the novice user:
 * In most cases only Filter Max Radius, Filter treshold and Texture Detail are needed and the other
 * params can be left at their default setting.
 *
 * Main Filter (Preprocessing) 
 * First, a filtered template is generated, using an adaptive filter.
 * To see this template, we must set _Luminance tolerance,  _Color tolerance to 1.0.
 *------------------------------------------------------------------------------------------------------------
 *
 * "Filter max. Radius" is preset to 5.0 
 * This is good for most noise situations. 
 * In any case it must be about the same size as noise granularity ore somewhat more.
 * If it is set higher than necessary, then it can cause unwanted blur.
 *------------------------------------------------------------------------------------------------------------
 *
 * "Filter Threshold" should be set so that edges are clearly visible and noise is  smoothed out. 
 * This threshold value is not bound to any intensity value, it is bound to the second derivative of
 * intensity values.
 * Simply adjust it and watch the preview. Adjustment must be made carefully, because the gap
 * between "noisy", "smooth", and "blur" is very small. Adjust it as carefully as you would adjust
 * the focus of a camera.
 *------------------------------------------------------------------------------------------------------------
 *
 * "Lookahead" defines the pixel distance in which the filter looks ahead for luminance variations
 * Normally the default value should do.
 * When _Lookahead is increased, then spikenoise is erased. 
 * Eventually readjust Filter treshold, when you changed lookahead.
 * When the value is to high, then the adaptive filter cannot longer accurately track image details, and
 * noise can reappear or blur can occur.
 *
 * Minimum value is 1.0, this gives best accuracy when blurring very weak noise.
 *
 * I never had good success with other values than 2.0.
 * However, for images with extemely high or low resolution another value possibly is better.
 * Use it only as a last ressort.
 *------------------------------------------------------------------------------------------------------------
 *
 * "Phase Jitter Damping" defines how fast the adaptive filter-radius reacts to luminance variations.
 * I have preset a value, that should do in most cases.
 * If increased, then edges appear smoother, if too high, then blur may occur.
 * If at minimum then noise and phase jitter at edges can occur.
 * It can suppress Spike noise when increased and this is the preferred method to remove spike noise.
 *------------------------------------------------------------------------------------------------------------
 *
 * "Sharpness" does just what it says, it improves sharpness. It improves the frequency response for the filter.
 * When it is too strong then not all noise can be removed, or spike noise may appear.
 * Set it near to maximum, if you want to remove weak noise or JPEG-artifacts, without loosing detail.
 *------------------------------------------------------------------------------------------------------------
 *
 * "Erosion". The new filter gives better sharpness and this also gives problems
 * with spike noise. The Erosion param erodes singular spikes and it has a smooth effect to edges, and sharpens
 * edges by erosion, so noise at edges is eroded.  
 * The effect is dependant from sharpness,phase-jitter damping and lookahead.
 * Set it to minimum (zero), if you want to remove weak noise or JPEG-artifacts.
 * When "Erosion" is increased, then also increasing "Phase Jitter Damping" is often useful 
 *
 * It works nicely. Apart from removing spike noise it has a sharpening and antialiasing effect to edges 
 * (Sharpening occurs by erosion, not by deconvolution) 
 *------------------------------------------------------------------------------------------------------------
 * 
 * "Texture Detail" can be used, to get more or less texture accuracy.
 * When decreased, then noise and texture are blurred out, when increased then texture is
 * amplified, but also noise will increase.
 * It has almost no effect to image edges, opposed to Filter theshold, which would blur edges, when increased.  
 *
 * E.g. if Threshold is adjusted in away so that edges are sharp, and there is still too much area noise, then
 * Texture detail could be used to reduce noise without blurring edges.
 * (Another way would be to decrease radius and to increase threshold)
 *
 *------------------------------------------------------------------------------------------------------------
 *
 * The filtered image that is now seen in the preview, is used as template for the following processing steps,
 * therefore it is  important to do this adjustment in first place and to do it as good as possible.
 *------------------------------------------------------------------------------------------------------------
 * 
 * Combining original image and filtered image, using tolerance thresholds (Postprocessing)
 * This can give a final touch of sharpness to your image. 
 * It is not necessary to do this, if you want to reduce JPEG-artifacts or weak noise.
 * It's purpose is to master strong noise without loosing too much sharpness.
 *
 * Note, that this all is done in one filter invocation. Preprocessing and postprocessing is done in one run,
 * but logically and in the algorithm they are different and ordered processes.
 *
 *
 * Adjust _Color tolerance or/and Luminance tolerance, (if necessary) so that you get the final image.
 * I recommend to use only one, either _Color  or _Luminance. 
 * These settings do not influence the main smoothing process. What they really do is this:
 *
 * The tolerance values are used as error-thresholds to compare the filtered template with the original
 * image. The plugin algorithm uses them to combine the filtered template with the original image
 * so that  noise and filter errors (blur) are thrown out.
 * A filtered pixel, that is too far away from the original pixel will be overridden by original image content. 
 *
 * Hint:
 * If you cange other sliders, like lookahead or Texture Detail, then you should set color tolerance and 
 * luminance tolerance to 1.0 (right end), because otherwise the filtered template is partially hidden 
 * and e.g. the effects for the damping filter cant be seen clearly and cant be optimized. 
 *------------------------------------------------------------------------------------------------------------
 *
 * _Gamma can be used to increase the tolerance values for darker areas (which commonly are more noisy)
 * This results in more blur for shadow areas.
 *
 * Hint for users of previous versions:
 * Gamma also influences the main-filter process. While the previous version did not have this feature,
 * I have reimplemented it, however, the algorithm used is totally new.
 * 
 *
 * Keep in mind, how the filter works, then usage should be easy!  
 *
 *
 * ================ THEORY AND TECHNIC =======================================================================
 *
 * Some interesting things (theoretic and technic)
 * This plugin bases on the assumption, that noise has no 2-dimensional correlation and therefore
 * can be removed in a 1-dimensional process.
 * To remove noise, I use a four-times  boxfilter with variable radius.
 *
 * The radius is calculated from 2nd derivative of pixeldata.
 * A gauss filter is used to calculte 2nd derivative.
 * The filter has some inbuilt features to clip low amplitude noise to clip very high values that would
 * slow down response time.
 * The 2nd derivative is lowpassfiltered and then radius is calculated as (Filter Treshold)/2nd_derivative. 
 * The radius modulation data is precalulated and buffered an is used to steer filter radius when
 * the actual filtering occurs.
 *
 * Noise and texture can be further suppressed by nonlinear distortion before adaptive filtering.
 * To make this possible I subtract low frequency from image data before denoising, so that I get a 
 * bipolar, zerosymmetric image signal.
 *
 * The filter works in a /one-dimensional/ way. It is applied to x and then to y axis.
 *
 * After filtering a zerodimensional point operator  (pixel by pixel comparison)  is used, where 
 * filter-errors are thrown out.
 * This is meant to limit and control filter errors,it can give "final touch" to the image, but it has 
 * nothing to do with the main filter process. 
 *
 * I do not know if something like this filter already exists.
 * It is all based on my own ideas and experiments.
 * Possibly a separable adaptive gauss-filter is a new thing.
 * Also it is an impossible thing, from a mathemathical point of view ;-)
 * It is possible only for bandwidth limited images.
 * Happyly most photographic images are bandwidth limited, or when they are noisy then we want
 * to limit banwith locally. And this is, what the filter does: It limits bandwidth locally, dependent
 * from (approximately) 2nd derivative of intensity.
 * 
 * Because gauss filtering is essentially linear diffusion, and because this filter uses a variable
 * nonlinear modulated gaussfilter (four box passes are almost gauss) we could say, that this filter
 * implements a special subclass of nonlinear adaptive diffusion, which is separable, and indeed, 
 * results are very similar to nonlinear diffusion filters.
 * However, because the filter is separable, it is much faster and needs less memory.
 */

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

    void filterImage(void);

    void iir_init(double r);
    void box_filter(double *src, double *end, double *dest, double radius);
    void iir_filter(float* const start, float* const end, float* dstart, double radius, const int type);
    void filter(float *buffer, float *data, float *data2, float *rbuf, float *tbuf, int width, int color);
    void blur_line(float* const data, float* const data2, float* const buffer,
                   float* rbuf, float* tbuf, const uchar *src, uchar *dest, int len);

    inline double mypow(double val, double ex)
    {
        if (fabs(val) < 1e-16) return 0.0;
        if (val > 0.0) return exp(log(val)*ex);
        return -exp(log(-val)*ex);
    };
 
private:

    struct iir_param
    {
        double  B, b1, b2, b3, b0, r, q;
        double *p;
    } m_iir;

    enum IIRFilteringMode
    {
        Gaussian=0,
        SecondDerivative
    };
    
    int    m_clampMax;
    
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
};

}  // NameSpace DigikamNoiseReductionImagesPlugin

#endif /* NOISE_REDUCTION_H */
