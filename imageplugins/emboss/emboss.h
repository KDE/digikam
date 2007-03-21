/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at gmail dot com>
 *          Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Date   : 2005-05-25
 * Description : Emboss threaded image filter.
 * 
 * Copyright 2005 by Gilles Caulier
 * Copyright 2006-2007 by Gilles Caulier and Marcel Wiesweg
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
  
#ifndef EMBOSS_H
#define EMBOSS_H

// Digikam includes.

#include "dimgthreadedfilter.h"

namespace DigikamEmbossImagesPlugin
{

class Emboss : public Digikam::DImgThreadedFilter
{

public:

    Emboss(Digikam::DImg *orgImage, QObject *parent=0, int depth=30);
    ~Emboss(){};

private:  

    virtual void filterImage(void);


    void embossImage(Digikam::DImg *orgImage, Digikam::DImg *destImage, int d);

    // Function to limit the max and min values defined by the developer.

    inline int Lim_Max (int Now, int Up, int Max)
    {
        --Max;
        while (Now > Max - Up)
            --Up;
        return (Up);
    };

    inline int getOffset(int Width, int X, int Y, int bytesDepth)
    {
        return (Y * Width * bytesDepth) + (X * bytesDepth);
    };

private:  

    int m_depth;
};

}  // NameSpace DigikamEmbossImagesPlugin

#endif /* EMBOSS_H */
