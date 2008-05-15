/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-01-17
 * Description : Haar 2d transform
 *               Wavelet algorithms, metric and query ideas based on the paper
 *               "Fast Multiresolution Image Querying"
 *               by Charles E. Jacobs, Adam Finkelstein and David H. Salesin.
 *               http://www.cs.washington.edu/homes/salesin/abstracts.html
 *
 * Copyright (C) 2003 by Ricardo Niederberger Cabral <nieder at mail dot ru>
 * Copyright (C) 2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// C++ includes

#include <cmath>

// Local includes

#include "haar.h"

namespace Digikam
{

/** Do the Haar tensorial 2d transform itself.
    Here input is RGB data [0..255] in Unit arrays
    Computation is (almost) in-situ.
*/
void Haar::haar2D(Unit a[])
{
    int i;
    Unit t[NUM_PIXELS >> 1];

    // scale by 1/sqrt(128) = 0.08838834764831843:
    /*
    for (i = 0; i < NUM_PIXELS_SQUARED; i++)
        a[i] *= 0.08838834764831843;
    */

    // Decompose rows:
    for (i = 0; i < NUM_PIXELS_SQUARED; i += NUM_PIXELS) 
    {
        int h, h1;
        Unit C = 1;

        for (h = NUM_PIXELS; h > 1; h = h1) 
        {
            int j1, j2, k;

            h1 = h >> 1;        // h = 2*h1
            C *= 0.7071;        // 1/sqrt(2)
            for (k = 0, j1 = j2 = i; k < h1; k++, j1++, j2 += 2) 
            {
                int j21 = j2+1;
                t[k]    = (a[j2] - a[j21]) * C;
                a[j1]   = (a[j2] + a[j21]);
            }
            // Write back subtraction results:
            memcpy(a+i+h1, t, h1*sizeof(a[0]));
        }
        // Fix first element of each row:
        a[i] *= C;	// C = 1/sqrt(NUM_PIXELS)
    }

    // scale by 1/sqrt(128) = 0.08838834764831843:
    /*
    for (i = 0; i < NUM_PIXELS_SQUARED; i++)
        a[i] *= 0.08838834764831843;
    */

    // Decompose columns:
    for (i = 0; i < NUM_PIXELS; i++) 
    {
        Unit C=1;
        int  h, h1;

        for (h = NUM_PIXELS; h > 1; h = h1) 
        {
            int j1, j2, k;

            h1 = h >> 1;
            C *= 0.7071;       // 1/sqrt(2) = 0.7071
            for (k = 0, j1 = j2 = i; k < h1; k++, j1 += NUM_PIXELS, j2 += 2*NUM_PIXELS) 
            {
                int j21 = j2+NUM_PIXELS;
                t[k]    = (a[j2] - a[j21]) * C;
                a[j1]   = (a[j2] + a[j21]);
            }

            // Write back subtraction results:
            for (k = 0, j1 = i+h1*NUM_PIXELS; k < h1; k++, j1 += NUM_PIXELS)
            {
                a[j1]=t[k];
            }
        }
        // Fix first element of each column:
        a[i] *= C;
    }
}

/** Do the Haar tensorial 2d transform itself.
    Here input is RGB data [0..255] in Unit arrays.
    Results are available in a, b, and c.
    Fully inplace calculation; order of result is interleaved though,
    but we don't care about that.
*/
void Haar::transform(Unit* a, Unit* b, Unit* c)
{
    // RGB -> YIQ colorspace conversion; Y luminance, I,Q chrominance.
    // If RGB in [0..255] then Y in [0..255] and I,Q in [-127..127].

    do 
    {
        for (int i = 0; i < NUM_PIXELS_SQUARED; i++) 
        {
            Unit Y, I, Q;

            Y    = 0.299 * a[i] + 0.587 * b[i] + 0.114 * c[i];
            I    = 0.596 * a[i] - 0.275 * b[i] - 0.321 * c[i];
            Q    = 0.212 * a[i] - 0.523 * b[i] + 0.311 * c[i];
            a[i] = Y;
            b[i] = I;
            c[i] = Q;
        }
    }
    while(0);     // FIXME: Marcel, I don't understand... we will have an eternal loop here ???

    haar2D(a);
    haar2D(b);
    haar2D(c);

    // Reintroduce the skipped scaling factors
    a[0] /= 256 * 128;
    b[0] /= 256 * 128;
    c[0] /= 256 * 128;
}

/** Find the NUM_COEFS largest numbers in cdata[] (in magnitude that is)
    and store their indices in sig[].
*/
void Haar::get_m_largests(Unit *cdata, Idx *sig)
{
    int       cnt, i;
    valStruct val;
    valqueue  vq;     // dynamic priority queue of valStruct's

    // Could skip i=0: goes into separate avgl

    // Fill up the bounded queue. (Assuming NUM_PIXELS_SQUARED > NUM_COEFS)
    for (i = 1; i < NUM_COEFS+1; i++)
    {
        val.i = i;
        val.d = fabs(cdata[i]);
        vq.push(val);
    }
    // Queue is full (size is NUM_COEFS)

    for (/*i = NUM_COEFS+1*/; i < NUM_PIXELS_SQUARED; i++)
    {
        val.d = fabs(cdata[i]);

        if (val.d > vq.top().d)
        {
            // Make room by dropping smallest entry:
            vq.pop();
            // Insert val as new entry:
            val.i = i;
            vq.push(val);
        }
        // else discard: do nothing
    }

    // Empty the (non-empty) queue and fill-in sig:
    cnt=0;
    do
    {
        int t;

        val = vq.top();
        t   = (cdata[val.i] <= 0);       // t = 0 if pos else 1 
        /* i - 0 ^ 0 = i; i - 1 ^ 0b111..1111 = 2-compl(i) = -i */
        sig[cnt++] = (val.i - t) ^ -t;   // never 0
        vq.pop();
    }
    while(!vq.empty());

    // Must have cnt==NUM_COEFS here.
}

/** Determines a total of NUM_COEFS positions in the image that have the
    largest magnitude (absolute value) in color value. Returns linearized
    coordinates in sig1, sig2, and sig3. avgl are the [0,0] values.
    The order of occurrence of the coordinates in sig doesn't matter.
    Complexity is 3 x NUM_PIXELS^2 x 2log(NUM_COEFS).
*/
int Haar::calcHaar(Unit *cdata1, Unit *cdata2, Unit *cdata3,
                   Idx *sig1, Idx *sig2, Idx *sig3, double *avgl)
{
    avgl[0]=cdata1[0];
    avgl[1]=cdata2[0];
    avgl[2]=cdata3[0];

    // Color channel 1:
    get_m_largests(cdata1, sig1);

    // Color channel 2:
    get_m_largests(cdata2, sig2);

    // Color channel 3:
    get_m_largests(cdata3, sig3);

    return 1;
}

// ----------------------------------------------------------------------------
// TODO: Marcel, these public methods can be removed.

// python array wrapper. Creates a new double array
double* Haar::new_darray(int size)
{
    return (double*) malloc(size*sizeof(double));
}

// python array wrapper. Creates a new int array
int* Haar::new_iarray(int size)
{
    return (int*) malloc(size*sizeof(int));
}

/** Do the Haar tensorial 2d transform itself.
    Here input RGB data is in unsigned char arrays ([0..255])
    Results are available in a, b, and c.
*/
void Haar::transformChar(unsigned char* c1, unsigned char* c2, unsigned char* c3,
                         Unit* a, Unit* b, Unit* c)
{
    Unit *p = a;
    Unit *q = b;
    Unit *r = c;

    for (int i = 0; i < NUM_PIXELS_SQUARED; i++)
    {
        *p++ = *c1++;
        *q++ = *c2++;
        *r++ = *c3++;
    }
    transform(a, b, c);
}

}  // namespace Digikam
