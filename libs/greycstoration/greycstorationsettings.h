/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-21-07
 * Description : Greycstoration settings container.
 * 
 * Copyright (C) 2007 by Gilles Caulier <caulier dot gilles at gmail dot com> 
 * 
 * For a full settings description, look at this url :
 * http://www.greyc.ensicaen.fr/~dtschump/greycstoration/guide.html
 *
 * For demonstration of settings, look at this url :
 *
 * http://www.greyc.ensicaen.fr/~dtschump/greycstoration/demonstration.html
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

#ifndef GREYCSTORATIONSETTINGS_H
#define GREYCSTORATIONSETTINGS_H

// Local includes.

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT GreycstorationSettings
{

public:

    enum INTERPOLATION
    {
        NearestNeighbor = 0,
        Linear,
        RungeKutta
    };

public:
    
    GreycstorationSettings()
    {
        setRestorationDefaultSettings();
    };
    
    ~GreycstorationSettings(){};

    void setRestorationDefaultSettings()
    {
        fastApprox = true;
    
        tile       = 256;
        btile      = 4;
    
        nbIter     = 1;
        interp     = NearestNeighbor;
    
        amplitude  = 60.0;
        sharpness  = 0.7;
        anisotropy = 0.3;
        alpha      = 0.6;
        sigma      = 1.1;
        gaussPrec  = 2.0;
        dl         = 0.8;
        da         = 30.0;
    };

    void setInpaintingDefaultSettings()
    {
        fastApprox = true;
    
        tile       = 256;
        btile      = 4;
    
        nbIter     = 30;
        interp     = NearestNeighbor;
    
        amplitude  = 20.0;
        sharpness  = 0.3;
        anisotropy = 1.0;
        alpha      = 0.8;
        sigma      = 2.0;
        gaussPrec  = 2.0;
        dl         = 0.8;
        da         = 30.0;
    };

    void setResizeDefaultSettings()
    {
        fastApprox = true;
    
        tile       = 256;
        btile      = 4;
    
        nbIter     = 3;
        interp     = NearestNeighbor; 
    
        amplitude  = 20.0;
        sharpness  = 0.2;
        anisotropy = 0.9;
        alpha      = 0.1;
        sigma      = 1.5;
        gaussPrec  = 2.0;
        dl         = 0.8;
        da         = 30.0;
    };

public:

    bool  fastApprox;

    int   tile;
    int   btile;

    uint  nbIter;
    uint  interp;

    float amplitude;
    float sharpness;
    float anisotropy;
    float alpha;
    float sigma;
    float gaussPrec;
    float dl;
    float da;
};

}  // namespace Digikam

#endif  // GREYCSTORATIONSETTINGS_H
