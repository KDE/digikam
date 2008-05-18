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
 * Copyright (C) 2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef HAAR_H
#define HAAR_H

// C++ Includes

#include <queue>

// Qt includes

#include <QtGlobal>

namespace Digikam
{

 /** Weights for the Haar coefficients. Straight from the referenced paper
     "Fast Multiresolution Image Querying"
     by Charles E. Jacobs, Adam Finkelstein and David H. Salesin.
     http://www.cs.washington.edu/homes/salesin/abstracts.html
 */
 static const float s_haar_weights[2][6][3] =
 {
     // For scanned picture (sketch=0):
     //   Y      I      Q        idx  total occurs
        {{ 5.00, 19.21, 34.37 },   // 0   58.58       1 (`DC' component)
         { 0.83,  1.26,  0.36 },   // 1    2.45       3
         { 1.01,  0.44,  0.45 },   // 2    1.90       5
         { 0.52,  0.53,  0.14 },   // 3    1.19       7
         { 0.47,  0.28,  0.18 },   // 4    0.93       9
         { 0.30,  0.14,  0.27 }},  // 5    0.71       16384-25=16359

     // For handdrawn/painted sketch (sketch=1):
     //   Y      I      Q
        {{ 4.04, 15.14, 22.62 },
         { 0.78,  0.92,  0.40 },
         { 0.46,  0.53,  0.63 },
         { 0.42,  0.26,  0.25 },
         { 0.41,  0.14,  0.15 },
         { 0.32,  0.07,  0.38 }}
};

class Haar
{
public:

    /** Number of pixels on one side of image; required to be a power of 2. */
    enum { NumberOfPixels = 128 };

    /** Total pixels in a square image. */
    enum { NumberOfPixelsSquared = NumberOfPixels * NumberOfPixels };

    /** Number of Haar coeffients we retain as signature for an image. */
    enum { NumberOfCoefficients = 40 };

    typedef double Unit;
    typedef qint32 Idx;

    class ImageData
    {
        public:
            Unit data1[NumberOfPixelsSquared];
            Unit data2[NumberOfPixelsSquared];
            Unit data3[NumberOfPixelsSquared];
    };

    class SigData
    {
        public:
            Idx  sig1[NumberOfCoefficients];
            Idx  sig2[NumberOfCoefficients];
            Idx  sig3[NumberOfCoefficients];
    };

    class WeightBin
    {
        public:

            WeightBin();

            /** Fixed weight mask for pixel positions (i,j).
                Each entry x = i*NUM_PIXELS + j, gets value max(i,j) saturated at 5.
                To be treated as a constant.
            */
            unsigned char m_bin[16384];

            unsigned char bin(int index) { return m_bin[index]; }
    };

    class Weights
    {
        public:
            enum SketchType
            {
                ScannedSketch = 0,
                PaintedSketch = 1
            };

            Weights(SketchType type = ScannedSketch)
            : m_type(type)
            {
            }

            float weight(int weight, int channel) { return s_haar_weights[(int)m_type][weight][channel]; }
            float weightForAverage(int channel) { return s_haar_weights[(int)m_type][0][channel]; }

        private:

            SketchType m_type;
    };

public:

    Haar();
    ~Haar();

    int     calcHaar(ImageData *data,
                     Idx* sig1, Idx* sig2, Idx* sig3, double* avgl);

    void    transform(ImageData *data);
    //void    transformChar(unsigned char* c1, unsigned char* c2, unsigned char* c3,
      //                    Unit* a, Unit* b, Unit* c);

private:

    void        haar2D(Unit a[]);
    inline void getmLargests(Unit *cdata, Idx *sig);
};

}  // namespace Digikam

#endif // HAAR_H
