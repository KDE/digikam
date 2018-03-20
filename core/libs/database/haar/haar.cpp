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
 * Copyright (C) 2003      by Ricardo Niederberger Cabral <nieder at mail dot ru>
 * Copyright (C) 2008-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2008-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "haar.h"

// C++ includes

#include <cstdlib>
#include <cmath>
#include <queue>

// Qt includes

#include <QImage>
#include <QString>

// Local includes

#include "dimg.h"

using namespace std;

namespace Digikam
{

namespace Haar
{

/** Signature structure
 */
class valStruct
{

public:

    Unit d;   // [f]abs(a[i])
    int  i;   // index i of a[i]

    bool operator< (const valStruct& right) const
    {
        return d > right.d;
    }
};

typedef std::priority_queue<valStruct> valqueue;

// --------------------------------------------------------------------

/** Write pixels of a QImage in three arrays (one per color channel, pixels linearly)
 */
void ImageData::fillPixelData(const QImage& im)
{
    QImage image = im.scaled(Haar::NumberOfPixels, Haar::NumberOfPixels, Qt::IgnoreAspectRatio);
    int cn       = 0;

    for (int h = 0; h < Haar::NumberOfPixels; ++h)
    {
        // Get a scanline:
        QRgb* const line = reinterpret_cast<QRgb*>(image.scanLine(h));

        for (int w = 0; w < Haar::NumberOfPixels; ++w)
        {
            QRgb pixel = line[w];
            data1[cn]  = qRed  (pixel);
            data2[cn]  = qGreen(pixel);
            data3[cn]  = qBlue (pixel);
            ++cn;
        }
    }
}

// --------------------------------------------------------------------

/** Write pixels of a DImg in three arrays (one per color channel, pixels linearly)
 */
void ImageData::fillPixelData(const DImg& im)
{
    DImg image(im);
    image.convertToEightBit();
    image      = image.smoothScale(Haar::NumberOfPixels, Haar::NumberOfPixels, Qt::IgnoreAspectRatio);
    uchar* ptr = image.bits();
    int cn     = 0;

    for (int h = 0; h < Haar::NumberOfPixels; ++h)
    {
        for (int w = 0; w < Haar::NumberOfPixels; ++w)
        {
            data1[cn] = ptr[2];
            data2[cn] = ptr[1];
            data3[cn] = ptr[0];
            ptr       += 4;
            ++cn;
        }
    }
}

// --------------------------------------------------------------------

/** Setup initial fixed Haar weights that each coefficient represents
 */
WeightBin::WeightBin()
{
    int i, j;

    /*
    0 1 2 3 4 5 6 i
    0 0 1 2 3 4 5 5
    1 1 1 2 3 4 5 5
    2 2 2 2 3 4 5 5
    3 3 3 3 3 4 5 5
    4 4 4 4 4 4 5 5
    5 5 5 5 5 5 5 5
    5 5 5 5 5 5 5 5
    j
    */

    // Every position has value 5
    memset(m_bin, 5, NumberOfPixelsSquared);

    // Except for the 5 by 5 upper-left quadrant
    for (i = 0; i < 5; ++i)
    {
        for (j = 0; j < 5; ++j)
        {
            m_bin[i*128+j] = qMax(i, j);
            // NOTE: imgBin[0] == 0
        }
    }
}

// --------------------------------------------------------------------

Calculator::Calculator()
{
}

Calculator::~Calculator()
{
}

/** Do the Haar tensorial 2d transform itself.
    Here input is RGB data [0..255] in Unit arrays
    Computation is (almost) in-situ.
*/
void Calculator::haar2D(Unit a[])
{
    int  i;
    Unit t[NumberOfPixels >> 1];

    // scale by 1/sqrt(128) = 0.08838834764831843:
    /*
    for (i = 0; i < NUM_PIXELS_SQUARED; ++i)
        a[i] *= 0.08838834764831843;
    */

    // Decompose rows:
    for (i = 0; i < NumberOfPixelsSquared; i += NumberOfPixels)
    {
        int h, h1;
        Unit C = 1;

        for (h = NumberOfPixels; h > 1; h = h1)
        {
            int j1, j2, k;

            h1 = h >> 1;        // h = 2*h1
            C *= 0.7071;        // 1/sqrt(2)

            for (k = 0, j1 = j2 = i; k < h1; ++k, ++j1, j2 += 2)
            {
                int j21 = j2+1;
                t[k]    = (a[j2] - a[j21]) * C;
                a[j1]   = (a[j2] + a[j21]);
            }

            // Write back subtraction results:
            memcpy(a+i+h1, t, h1*sizeof(a[0]));
        }

        // Fix first element of each row:
        a[i] *= C;  // C = 1/sqrt(NUM_PIXELS)
    }

    // scale by 1/sqrt(128) = 0.08838834764831843:
    /*
    for (i = 0; i < NUM_PIXELS_SQUARED; ++i)
        a[i] *= 0.08838834764831843;
    */

    // Decompose columns:
    for (i = 0; i < NumberOfPixels; ++i)
    {
        Unit C = 1;
        int  h, h1;

        for (h = NumberOfPixels; h > 1; h = h1)
        {
            int j1, j2, k;

            h1 = h >> 1;
            C *= 0.7071;       // 1/sqrt(2) = 0.7071

            for (k = 0, j1 = j2 = i; k < h1; ++k, j1 += NumberOfPixels, j2 += 2*NumberOfPixels)
            {
                int j21 = j2+NumberOfPixels;
                t[k]    = (a[j2] - a[j21]) * C;
                a[j1]   = (a[j2] + a[j21]);
            }

            // Write back subtraction results:
            for (k = 0, j1 = i+h1*NumberOfPixels; k < h1; ++k, j1 += NumberOfPixels)
            {
                a[j1] = t[k];
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
void Calculator::transform(ImageData* const data)
{
    // RGB -> YIQ colorspace conversion; Y luminance, I,Q chrominance.
    // If RGB in [0..255] then Y in [0..255] and I,Q in [-127..127].
    Unit* a = data->data1;
    Unit* b = data->data2;
    Unit* c = data->data3;

    for (int i = 0; i < NumberOfPixelsSquared; ++i)
    {
        Unit Y, I, Q;

        Y    = 0.299 * a[i] + 0.587 * b[i] + 0.114 * c[i];
        I    = 0.596 * a[i] - 0.275 * b[i] - 0.321 * c[i];
        Q    = 0.212 * a[i] - 0.523 * b[i] + 0.311 * c[i];
        a[i] = Y;
        b[i] = I;
        c[i] = Q;
    }

    haar2D(a);
    haar2D(b);
    haar2D(c);

    // Reintroduce the skipped scaling factors
    a[0] /= 256 * 128;
    b[0] /= 256 * 128;
    c[0] /= 256 * 128;
}

/** Find the m=NUM_COEFS largest numbers in cdata[] (in magnitude that is)
    and store their indices in sig[].
    Skips entry 0.
*/
void Calculator::getmLargests(Unit* const cdata, Idx* const sig)
{
    int       cnt, i;
    valStruct val;
    valqueue  vq;     // dynamic priority queue of valStruct's

    // Could skip i=0: goes into separate avgl

    // Fill up the bounded queue. (Assuming NUM_PIXELS_SQUARED > NUM_COEFS)
    for (i = 1; i < NumberOfCoefficients+1; ++i)
    {
        val.i = i;
        val.d = fabs(cdata[i]);
        vq.push(val);
    }

    // Queue is full (size is NUM_COEFS)

    for (/*i = NUM_COEFS+1*/; i < NumberOfPixelsSquared; ++i)
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
    cnt = 0;

    do
    {
        int t;

        val = vq.top();
        t   = (cdata[val.i] <= 0);       // t = 0 if pos else 1
        // i - 0 ^ 0 = i; i - 1 ^ 0b111..1111 = 2-compl(i) = -i
        sig[cnt++] = (val.i - t) ^ -t;   // never 0
        vq.pop();
    }
    while (!vq.empty());

    // Must have cnt==NUM_COEFS here.
}

/** Determines a total of NUM_COEFS positions in the image that have the
    largest magnitude (absolute value) in color value. Returns linearized
    coordinates in sig1, sig2, and sig3. avgl are the [0,0] values.
    The order of occurrence of the coordinates in sig doesn't matter.
    Complexity is 3 x NUM_PIXELS^2 x 2log(NUM_COEFS).
*/
int Calculator::calcHaar(ImageData* const data, SignatureData* const sigData)
{
    sigData->avg[0]=data->data1[0];
    sigData->avg[1]=data->data2[0];
    sigData->avg[2]=data->data3[0];

    // Color channel 1:
    getmLargests(data->data1, sigData->sig[0]);

    // Color channel 2:
    getmLargests(data->data2, sigData->sig[1]);

    // Color channel 3:
    getmLargests(data->data3, sigData->sig[2]);

    return 1;
}

} // namespace Haar

} // namespace Digikam
