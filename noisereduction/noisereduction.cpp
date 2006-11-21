/* ============================================================
 * File   : noisereduction.cpp
 * Authors: Gilles Caulier <caulier dot gilles at kdemail dot net>
 *          Peter Heckert <peter dot heckert at arcor dot de>
 * Date   : 2005-05-25
 * Description : Noise reduction threaded image filter.
 * 
 * Copyright 2005-2006 by Gilles Caulier
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

#define IIR1(dest,src)  (dest) = (d3 = ((((src) * b + d3) * b3 + d2) * b2 + d1) * b1)
#define IIR2(dest,src)  (dest) = (d2 = ((((src) * b + d2) * b3 + d1) * b2 + d3) * b1)
#define IIR3(dest,src)  (dest) = (d1 = ((((src) * b + d1) * b3 + d3) * b2 + d2) * b1)

#define IIR1A(dest,src)  (dest) = fabs(d3 = ((((src) * b + d3) * b3 + d2) * b2 + d1) * b1)
#define IIR2A(dest,src)  (dest) = fabs(d2 = ((((src) * b + d2) * b3 + d1) * b2 + d3) * b1)
#define IIR3A(dest,src)  (dest) = fabs(d1 = ((((src) * b + d1) * b3 + d3) * b2 + d2) * b1)

#define FR 0.212671
#define FG 0.715160
#define FB 0.072169

// Local includes.

#include "noisereduction.h"

namespace DigikamNoiseReductionImagesPlugin
{

NoiseReduction::NoiseReduction(Digikam::DImg *orgImage, QObject *parent, 
                double radius, double lsmooth, double effect, double texture, double sharp,
                double csmooth, double lookahead, double gamma, double damping, double phase)
              : Digikam::DImgThreadedFilter(orgImage, parent, "NoiseReduction")
{ 
    m_radius    = radius;    /* default radius                   default = 1.0  */
    m_sharp     = sharp;     /* Sharpness factor                 default = 0.25 */
    m_lsmooth   = lsmooth;   /* Luminance Tolerance              default = 1.0  */
    m_effect    = effect;    /* Adaptive filter-effect threshold default = 0.08 */
    m_texture   = texture;   /* Texture Detail                   default = 0.0  */

    m_csmooth   = csmooth;   /* RGB Tolerance                    default = 1.0  */
    m_lookahead = lookahead; /* Lookahead                        default = 2.0  */
    m_gamma     = gamma;     /* Filter gamma                     default = 1.0  */
    m_damping   = damping;   /* Phase jitter Damping             default = 5.0  */
    m_phase     = phase;     /* Area Noise Clip                  default = 1.0  */

    m_iir.B  = 0.0;
    m_iir.b1 = 0.0;
    m_iir.b2 = 0.0;
    m_iir.b3 = 0.0;
    m_iir.b0 = 0.0;
    m_iir.r  = 0.0;
    m_iir.q  = 0.0;
    m_iir.p  = 0;

    m_clampMax = m_orgImage.sixteenBit() ? 65535 : 255;
    
    initFilter();
}

// Remove noise on the region, given a source region, dest.
// region, width and height of the regions, and corner coordinates of
// a subregion to act upon. Everything outside the subregion is unaffected.

void NoiseReduction::filterImage(void)
{
    int    bytes  = m_orgImage.bytesDepth(); // Bytes per pixel sample
    uchar *srcPR  = m_orgImage.bits();
    uchar *destPR = m_destImage.bits();
    int    width  = m_orgImage.width();
    int    height = m_orgImage.height();

    int    row, col, i, progress;
    float  prob = 0.0;
    
    int w = (int)((m_radius + m_lookahead + m_damping + m_phase) * 4.0 + 40.0);
    
    // NOTE: commented from original implementation
    // if (radius < m_lookahead) w = m_lookahead * 4.0 + 40.0;
    
    float csmooth = m_csmooth;
    
    // Raw Filter preview

    if (csmooth >= 0.99) csmooth = 1.0; 
        
    // Allocate and init buffers

    uchar *src    = new uchar[ QMAX (width, height) * bytes ];
    uchar *dest   = new uchar[ QMAX (width, height) * bytes ];
    float *data   = new float[ QMAX (width, height) + 2*w ];
    float *data2  = new float[ QMAX (width, height) + 2*w ];
    float *buffer = new float[ QMAX (width, height) + 2*w ];
    float *rbuf   = new float[ QMAX (width, height) + 2*w ];
    float *tbuf   = new float[ QMAX (width, height) + 2*w ];

    memset (src,  0, QMAX (width, height) * bytes);
    memset (dest, 0, QMAX (width, height) * bytes);
    
    for (i=0 ; i < QMAX(width,height)+2*w-1 ; i++)
        data[i] = data2[i] = buffer[i] = rbuf[i] = tbuf[i] = 0.0;
    
    // Initialize the damping filter coefficients
    
    iir_init(m_radius);
    
    // blur the rows
    
    for (row = 0 ; !m_cancel && (row < height) ; row++)
    {
        memcpy(src, srcPR + row*width*bytes, width*bytes);
        memcpy(dest, src, width*bytes);
        
        blur_line (data+w, data2+w, buffer+w, rbuf+w, tbuf+w, src, dest, width);
        
        memcpy(destPR + row*width*bytes, dest, width*bytes);
      
        progress = (int)(((double)row * 20.0) / height);
        if ( progress%2 == 0 )
           postProgress( progress );   
    }
  
    // blur the cols

    for (col = 0 ; !m_cancel && (col < width) ; col++)
    {
        for (int n = 0 ; n < height ; n++)
            memcpy(src + n*bytes, destPR + (col + width*n)*bytes, bytes);
                
        for (int n = 0 ; n < height ; n++)
            memcpy(dest + n*bytes, srcPR + (col + width*n)*bytes, bytes);
        
        blur_line (data+w, data2+w, buffer+w, rbuf+w, tbuf+w, src, dest, height);
        
        for (int n = 0 ; n < height ; n++)
            memcpy(destPR + (col + width*n)*bytes, dest + n*bytes, bytes);

        progress = (int)(20.0 + ((double)col * 20.0) / width);
        if ( progress%2 == 0 )
           postProgress( progress );   
    }
    
    // merge the source and destination (which currently contains
    // the blurred version) images
    
    for (row = 0 ; !m_cancel && (row < height) ; row++)
    {
        uchar *s            = src;
        uchar *d            = dest;
        unsigned short *s16 = (unsigned short *)src;
        unsigned short *d16 = (unsigned short *)dest;
        float  value;
        int    u, v;
    
        // get source row
        
        memcpy(src,  srcPR  + row*width*bytes, width*bytes);
        memcpy(dest, destPR + row*width*bytes, width*bytes);

        // get dest row and combine the two
        
        float t  = m_csmooth;
        float t2 = m_lsmooth;
        
        // Values are squared, so that sliders get a nonlinear chracteristic
	// for better adjustment accuracy when values are small.
        t*=t;
        t2*=t2;         
  
        for (u = 0 ; !m_cancel && (u < width) ; u++)
        {
            float dpix[3], spix[3];
            float lum,  red,  green,  blue;
            float lum2, red2, green2, blue2;

            if (m_orgImage.sixteenBit())       // 16 bits image
            {
                red   = (float) s16[2]/(float)m_clampMax;
                green = (float) s16[1]/(float)m_clampMax;
                blue  = (float) s16[0]/(float)m_clampMax;
            }
            else                                // 8 bits image
            {
                red   = (float) s[2]/(float)m_clampMax;
                green = (float) s[1]/(float)m_clampMax;
                blue  = (float) s[0]/(float)m_clampMax;
            }
            
            spix[2] = red;
            spix[1] = green;
            spix[0] = blue;
                    
            lum = (FR*red + FG*green + FB*blue);
            
            if (m_orgImage.sixteenBit())       // 16 bits image
            {
                red2   = (float) d16[2]/(float)m_clampMax;
                green2 = (float) d16[1]/(float)m_clampMax;
                blue2  = (float) d16[0]/(float)m_clampMax;
            }
            else                                // 8 bits image
            {
                red2   = (float) d[2]/(float)m_clampMax;
                green2 = (float) d[1]/(float)m_clampMax;
                blue2  = (float) d[0]/(float)m_clampMax;
            }
            
            lum2 = (FR*red2 + FG*green2 + FB*blue2);
    
            // Calculate luminance error (contrast error) for filtered template.
            // This error is biggest, where edges are. Edges anyway cannot be filtered.
            // Therefore we can correct luminance error in edges without increasing noise.
            // Should be adjusted carefully, or not so carefully if you intentionally want to add noise.
            // Noise, if not colorized, /can/ look good, so this makes sense.
    
            float dl = lum - lum2;
                
            // Multiply dl with first derivative of gamma curve divided by derivative value for midtone 0.5
            // So bright tones will be corrected more (get more luminance noise and -information) than
            // darker values because bright parts of image generally are less noisy, this is what we want.
            
            dl *= pow(lum2/0.5, m_gamma-1.0);
    
            if (t2 > 0.0)
                dl *= (1.0 - exp(-dl*dl/(2.0*t2*t2)));        

            // NOTE: commented from original implementation
            // if (dl > p) dl = p;
            // if (dl < -p) dl = -p;
            
            dpix[2] =   red2 + dl;
            dpix[1] = green2 + dl;
            dpix[0] =  blue2 + dl;
            
            for (v = 0 ; !m_cancel && (v < 3) ; v++)
            {
                float value  = spix[v];
                float fvalue = dpix[v];
                float mvalue = (value + fvalue)/2.0;
                float diff   = (value) - (fvalue);
                
                // Multiply diff with first derivative of gamma curve divided by derivative value for midtone 0.5
                // So midtones will stay unchanged, darker values get more blur and brighter values get less blur
                // when we increase gamma.
                
                diff *= pow(mvalue/0.5, m_gamma-1.0);
    
                // Calculate noise probability for pixel
                // TODO : probably it's not probability but an arbitrary curve.
                // Probably we should provide a GUI-interface for this!!!
                
                if (t > 0.0)
                    prob = exp(-diff*diff/(2.0*t*t));
                else
                    prob = 0.0;
                
                // Allow viewing of raw filter output

                if (t >= 0.99)
                    prob = 1.0; 

                dpix[v] = value = fvalue * prob + value * (1.0 - prob);    
            }
    
            if (m_orgImage.sixteenBit())       // 16 bits image
            {
                value  = dpix[0]*(float)m_clampMax+0.5;
                d16[0] = (unsigned short)CLAMP(value, 0, m_clampMax);
                value  = dpix[1]*(float)m_clampMax+0.5;
                d16[1] = (unsigned short)CLAMP(value, 0, m_clampMax);
                value  = dpix[2]*(float)m_clampMax+0.5;
                d16[2] = (unsigned short)CLAMP(value, 0, m_clampMax);
                
                d16 += 4;
                s16 += 4;
            }
            else                                // 8 bits image
            {
                value = dpix[0]*(float)m_clampMax+0.5;
                d[0]  = (uchar)CLAMP(value, 0, m_clampMax);
                value = dpix[1]*(float)m_clampMax+0.5;
                d[1]  = (uchar)CLAMP(value, 0, m_clampMax);
                value = dpix[2]*(float)m_clampMax+0.5;
                d[2]  = (uchar)CLAMP(value, 0, m_clampMax);
                
                d += 4;
                s += 4;
            }
        }

        memcpy(destPR + row*width*bytes, dest, width*bytes);
        
        progress = (int)(40.0 + ((double)row * 60.0) / height);
        if ( progress%2 == 0 )
           postProgress( progress );   
    }
    
    delete [] data;
    delete [] data2;
    delete [] buffer;
    delete [] rbuf;
    delete [] tbuf;
    delete [] dest;
    delete [] src;
}

// This function is written as if it is blurring a column at a time,
// even though it can operate on rows, too.  There is no difference
// in the processing of the lines, at least to the blur_line function.
// 'len' is the length of src and dest

void NoiseReduction::blur_line(float* const data, float* const data2, float* const buffer,
                               float* rbuf, float* tbuf, const uchar *src, uchar *dest, int len)    
{
    int b;
    int row;
    int idx;

    unsigned short *src16  = (unsigned short *)src;
    unsigned short *dest16 = (unsigned short *)dest;
    
    // Calculate radius factors
    
    for (row = 0, idx = 0 ; !m_cancel && (idx < len) ; row += 4, idx++)
    {
        // Color weigths are choosen proportional to Bayer Sensor pixel count

        if (m_orgImage.sixteenBit())       // 16 bits image
        {
            data[idx] =  (float) dest16[row+2] / (float)m_clampMax * 0.25; // Red color
            data[idx] += (float) dest16[row+1] / (float)m_clampMax * 0.5;  // Green color
            data[idx] += (float) dest16[row]   / (float)m_clampMax * 0.25; // Blue color
            data[idx] = mypow(data[idx], m_gamma);
        }
        else                                // 8 bits image
        {
            data[idx] =  (float) dest[row+2] / (float)m_clampMax * 0.25; // Red color
            data[idx] += (float) dest[row+1] / (float)m_clampMax * 0.5;  // Green color
            data[idx] += (float) dest[row]   / (float)m_clampMax * 0.25; // Blue color
            data[idx] = mypow(data[idx], m_gamma);
        }
    }

    filter(data, data2, buffer, rbuf, tbuf, len, -1);
        
    // Do actual filtering
    
    for (b = 0 ; !m_cancel && (b < 3) ; b++)
    {
        for (row = b, idx = 0 ; !m_cancel && (idx < len) ; row += 4, idx++)
        {
            if (m_orgImage.sixteenBit())       // 16 bits image
                data[idx] = (float)src16[row] / (float)m_clampMax;
            else                                // 8 bits image
                data[idx] = (float)src[row] / (float)m_clampMax;
        }

        filter(data, data2, buffer, rbuf, tbuf, len, b);

        for (row = b, idx = 0 ; !m_cancel && (idx < len) ; row += 4, idx++)
        {
            int value = (int)(data[idx] * (float)m_clampMax + 0.5);
            
            if (m_orgImage.sixteenBit())       // 16 bits image
                dest16[row] = (unsigned short)CLAMP( value, 0, m_clampMax);
            else                                // 8 bits image
                dest[row] = (uchar)CLAMP( value, 0, m_clampMax);
        }
    }
}

void NoiseReduction::iir_init(double r)
{
    if (m_iir.r == r)
        return;
    
    // damping settings;
    m_iir.r = r;  

    double q;
    
    if ( r >= 2.5)
        q = 0.98711 * r - 0.96330;
    else
        q = 3.97156 - 4.14554 * sqrt(1.0 - 0.26891 * r);
    
    m_iir.q  = q;
    m_iir.b0 = 1.57825 + ((0.422205 * q  + 1.4281) * q + 2.44413) *  q;
    m_iir.b1 = ((1.26661 * q +2.85619) * q + 2.44413) * q / m_iir.b0;
    m_iir.b2 = - ((1.26661*q +1.4281) * q * q ) / m_iir.b0;
    m_iir.b3 = 0.422205 * q * q * q / m_iir.b0;
    m_iir.B  = 1.0 - (m_iir.b1 + m_iir.b2 + m_iir.b3);
}

void NoiseReduction::box_filter(double *src, double *end, double *dest, double radius)
{
    int   boxwidth = 1;
    float box      = (*src);
    float fbw      = 2.0 * radius;
    
    if (fbw < 1.0)
        fbw = 1.0;
    
    while(boxwidth+2 <= (int) fbw) boxwidth+=2, box += (src[boxwidth/2]) + (src[-boxwidth/2]);
    
    double frac = (fbw - (double) boxwidth) / 2.0;
    int    bh   = boxwidth / 2;
    int    bh1  = boxwidth / 2+1;
     
    for ( ; src <= end ; src++, dest++)
    {
        *dest = (box + frac * ((src[bh1])+(src[-bh1]))) / fbw;
        box   = box - (src[-bh]) + (src[bh1]);
    }
}

// Bidirectional IIR-filter, speed optimized 

void NoiseReduction::iir_filter(float* const start, float* const end, float* dstart, 
                                double radius, const int type)
{
    if (!dstart)
        dstart = start;
        
    int    width;
    float *src  = start;
    float *dest = dstart;
    float *dend = dstart + (end - start);

    radius = floor((radius + 0.1) / 0.5) * 0.5;

    // NOTE: commented from original implementation
    // gfloat boxwidth = radius * 2.0;
    // gint bw = (gint) boxwidth;
    
    int ofs = (int)radius;
    if (ofs < 1) ofs = 1;
    
    double d1, d2, d3;
    
    width = end - start + 1;

    if (radius < 0.25)
    { 
        if ( start != dest )
        { 
            memcpy(dest, start, width*sizeof(*dest));
            return;       
        }
    }
    
    iir_init(radius);

    const double b1 = m_iir.b1;
    const double b2 = m_iir.b2 / m_iir.b1;
    const double b3 = m_iir.b3 / m_iir.b2;
    const double b  = m_iir.B  / m_iir.b3;
    
    switch(type)
    {
        case Gaussian:
        
            d1 = d2 = d3 = *dest; 
            dend -= 6;
            src--;
            dest--;

            while (dest < dend)
            {
                IIR1(*(++dest), *(++src));
                IIR2(*(++dest), *(++src));
                IIR3(*(++dest), *(++src));
                IIR1(*(++dest), *(++src));
                IIR2(*(++dest), *(++src));
                IIR3(*(++dest), *(++src));
            }

            dend += 6;

            while (1)
            {
                if (++dest > dend) break; 
                    IIR1(*dest,*(++src));
                if (++dest > dend) break; 
                    IIR2(*dest,*(++src));
                if (++dest > dend) break; 
                    IIR3(*dest,*(++src));
            }
        
            d1 = d2 = d3 = dest[-1];
            dstart += 6;
            
            while (dest > dstart)
            {
                --dest, IIR1(*dest, *dest);
                --dest, IIR2(*dest, *dest);
                --dest, IIR3(*dest, *dest);
                --dest, IIR1(*dest, *dest);
                --dest, IIR2(*dest, *dest);
                --dest, IIR3(*dest, *dest);
            }
            
            dstart -= 6;
            
            while (1)
            {
                if (--dest < dstart) break; 
                    IIR1(*dest, *dest);
                if (--dest < dstart) break; 
                    IIR2(*dest, *dest);
                if (--dest < dstart) break; 
                    IIR3(*dest, *dest);
            }
        
        break;
            
        case SecondDerivative: // rectified and filtered second derivative, source and dest may be equal 
            
            d1 = d2 = d3 = 0.0;
            dest[0] = dest[ofs] = 0.0;
            dend -= 6; 
            dest--;
            src--;
            
            while (dest < dend)
            {
                ++src, IIR1(*(++dest), src[ofs]-src[0]);
                ++src, IIR2(*(++dest), src[ofs]-src[0]);
                ++src, IIR3(*(++dest), src[ofs]-src[0]);
                ++src, IIR1(*(++dest), src[ofs]-src[0]);
                ++src, IIR2(*(++dest), src[ofs]-src[0]);
                ++src, IIR3(*(++dest), src[ofs]-src[0]);
            }
            
            dend += 6; 

            while (1)
            {
                if (++dest > dend) break; 
                    ++src, IIR1(*dest, src[ofs]-src[0]);
                if (++dest > dend) break; 
                    ++src, IIR2(*dest, src[ofs]-src[0]);
                if (++dest > dend) break; 
                    ++src, IIR3(*dest, src[ofs]-src[0]);
            }
            
            d1 = d2 = d3 = 0.0;
            dest[-1] = dest[-ofs-1] = 0.0;
            dstart += 6;
                
            while (dest > dstart)
            {
                --dest, IIR1A(*dest, dest[0]-dest[-ofs]);
                --dest, IIR2A(*dest, dest[0]-dest[-ofs]);
                --dest, IIR3A(*dest, dest[0]-dest[-ofs]);
                --dest, IIR1A(*dest, dest[0]-dest[-ofs]);
                --dest, IIR2A(*dest, dest[0]-dest[-ofs]);
                --dest, IIR3A(*dest, dest[0]-dest[-ofs]);
            }

            dstart -= 6;

            while (1)
            {
                if (--dest < dstart) break;
                    IIR1A(*dest, dest[0]-dest[-ofs]);
                if (--dest < dstart) break;
                    IIR2A(*dest, dest[0]-dest[-ofs]);
                if (--dest < dstart) break;
                    IIR3A(*dest, dest[0]-dest[-ofs]);
            }
            
        break;
    }
}

// A forward-backward box filter is used here and the radius is adapted to luminance jump.
// Radius is calculated fron 1st and 2nd derivative of intensity values.
// (Its not exactly 2nd derivative, but something similar, optimized by experiment)
// The radius variations are filtered. This reduces spatial phase jitter.

void NoiseReduction::filter(float *buffer, float *data, float *data2, float *rbuf, 
                            float */*tbuf*/, int width, int color)
{
    float *lp        = data;
    float *rp        = data + width-1;
    float *lp2       = data2; 
    float *blp       = buffer;
    float *brp       = buffer + width-1;
    float *rbuflp    = rbuf;
    float *rbufrp    = rbuf + width-1;
    float  fboxwidth = m_radius*2.0;
    float  fradius   = m_radius;
    float *p1, *p2;
    
    if (fboxwidth < 1.0) fboxwidth = 1.0 ;
    if (fradius < 0.5) fradius = 0.5;
    
    int    i, pass;
    int    ofs, ofs2;
    float  maxrad;
    float  fbw;
    float  val;
    double rfact = m_effect*m_effect;
    double sharp = m_sharp;
    
    ofs2  = (int)floor(m_damping * 2.0 + 0.1);
    ofs   = (int)floor(m_lookahead * 2.0 + 0.1);
    int w = (int)(fboxwidth + m_damping + m_lookahead + m_phase + 2.0);
    
    // Mirror image edges    

    for (i=1 ; i <= w ; i++) 
        blp[-i] = blp[i]; 

    for (i=1 ; i <= w ; i++) 
        brp[i] = brp[-i];
    
    if (color < 0) // Calc 2nd derivative
    {
        // boost high frequency in rbuf
        
        for (p1 = blp, p2 = rbuflp ; p1 <= brp ; p1++, p2++)
        {
            *p2 = (sharp+1.0) * p1[0] - sharp * 0.5 * (p1[-ofs]+p1[ofs]);
        }
    
        iir_filter(rbuflp-w, rbufrp+w, blp-w, m_lookahead, SecondDerivative);
    
        // Mirror image edges

        for (i = 1 ; i <= w ; i++) 
            blp[-i] = blp[i];

        for (i = 1 ; i <= w ; i++) 
            brp[i] = brp[-i];
    
        // boost high frequency in rbuf
        
        for (p1 = blp, p2 = rbuflp ; p1 <= brp ; p1++, p2++)
        {
            *p2 = ((sharp+1.0) * (p1[0]) - sharp * 0.5 * ((p1[-ofs2])+(p1[ofs2])));
        }
    
        // Mirror rbuf edges

        for (i = 1 ; i <= w ; i++) 
            rbuflp[-i] = rbuflp[i];

        for (i = 1 ; i <= w ; i++) 
            rbufrp[i] = rbufrp[-i];
    
        // Lowpass (gauss) filter rbuf, remove phase jitter
        
        iir_filter(rbuflp-w+5, rbufrp+w-5, rbuflp-w+5, m_damping, Gaussian);
    
        for (i = -w+5; i < width-1+w-5 ; i++)
        {
            // NOTE: commented from original implementation
            // val = rbuflp[i];

            val = rbuflp[i]-rfact;
    
            // Avoid division by zero, clip negative filter overshoot
            
            if (val < rfact/fradius) val=rfact/fradius;
    
            val = rfact/val;

            // NOTE: commented from original implementation
            // val = pow(val/fradius,m_phase)*fradius;

            if (val < 0.5) val = 0.5;
    
            rbuflp[i] = val*2.0;
        }
    
        // Mirror rbuf edges

        for (i=1 ; i <= w ; i++) 
            rbuflp[-i] = rbuflp[i];

        for (i=1 ; i <= w ; i++) 
            rbufrp[i] = rbufrp[-i];

        return;
    } 
    
    // Calc lowpass filtered input signal
    
    iir_filter(blp-w+1, brp+w-1, lp2-w+1, m_radius, Gaussian);
    
    // Subtract low frequency from input signal (aka original image data)
    // and predistort this signal
    
    val = m_texture + 1.0;

    for (i = -w+1 ; i <= width-1+w-1 ; i++)
    {
        blp[i] = mypow(blp[i] - lp2[i], val);
    }
    
    float *src, *dest;
    val = m_texture + 1.0;
    
    pass = 2;
    
    while (pass--)
    {
        float sum;
        int   ibw;
        src    = blp;
        dest   = lp;
        maxrad = 0.0;
    
        // Mirror left edge
        
        for (i=1 ; i <= w ; i++)
            src[-i] = src[i]; 
    
        sum = (src[-1] += src[-2]);
    
        // forward pass
        
        for (rbuf = rbuflp-(int) m_phase ; rbuf <= rbufrp; src++, dest++, rbuf++)
        {
            // NOTE: commented from original implementation
            //fbw = fabs( rbuf[-ofs2]*ll2+rbuf[-ofs2-1]*rl2);
        
            fbw = *rbuf;
        
            if (fbw > (maxrad += 1.0)) fbw = maxrad;
            else if (fbw < maxrad) maxrad = fbw;
            
            ibw   = (int)fbw;        
            *src  = sum += *src;
            *dest = (sum-src[-ibw]+(src[-ibw]-src[-ibw-1])*(fbw-ibw))/fbw;
        }
    
        src    = rp;
        dest   = brp;
        maxrad = 0.0;
    
        // Mirror right edge
        
        for (i=1 ; i <= w ; i++)
            src[i] = src[-i]; 
    
        sum = (src[1] += src[2]);

        // backward pass

        for ( rbuf = rbufrp +(int) m_phase ; rbuf >= rbuflp; src--, dest--, rbuf--)
        {
            // NOTE: commented from original implementation
            //fbw = fabs( rbuf[ofs2]*ll2+rbuf[ofs2+1]*rl2);

            fbw = *rbuf;
        
            if (fbw > (maxrad +=1.0)) fbw = maxrad;
            else if (fbw < maxrad) maxrad = fbw;
        
            ibw = (int)fbw;
        
            *src  = sum += *src;
            *dest = (sum-src[ibw]+(src[ibw]-src[ibw+1])*(fbw-ibw))/fbw;
        }
    }
    
    val = 1.0 / (m_texture + 1.0);

    for (i = -w+1 ; i <= width-1+w-1 ; i++)
    {
        // Undo  predistortion
        
        blp[i]= mypow(blp[i],val);
    
        // Add in low frequency
        
        blp[i] += lp2[i]; 
    
        // NOTE: commented from original implementation
        // if (blp[i] >= 0.0) blp[i] = pow(blp[i],val);
        // else blp[i] = 0.0;
    }
}

}  // NameSpace DigikamNoiseReductionImagesPlugin
