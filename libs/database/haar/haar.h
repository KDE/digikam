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

#ifndef HAAR_H
#define HAAR_H

// C++ Includes

#include <queue>

/** Number of pixels on one side of image; required to be a power of 2.
*/
#define NUM_PIXELS 128

/** Totals pixels in a square image.
*/
#define NUM_PIXELS_SQUARED (NUM_PIXELS * NUM_PIXELS)

/** Number of Haar coeffients we retain as signature for an image.
*/
#define NUM_COEFS 40

namespace Digikam
{

class Haar
{

public:

    typedef double Unit;
    typedef int    Idx;

public:

    Haar();
    ~Haar();

    int     calcHaar(Unit* cdata1, Unit* cdata2, Unit* cdata3,
                     Idx* sig1, Idx* sig2, Idx* sig3, double* avgl);

    void    transform(Unit* a, Unit* b, Unit* c);
    void    transformChar(unsigned char* c1, unsigned char* c2, unsigned char* c3,
                          Unit* a, Unit* b, Unit* c);

private:

    /** Signature structure
    */
    typedef struct valStruct_
    {
        Unit d;         // [f]abs(a[i])
        int  i;         // index i of a[i]

        bool operator< (const valStruct_& right) const
        {
            return d > right.d;
        }

    } valStruct;

    typedef std::priority_queue<valStruct> valqueue;

private:

    void        haar2D(Unit a[]);
    inline void getmLargests(Unit *cdata, Idx *sig);
};

}  // namespace Digikam

#endif // HAAR_H
