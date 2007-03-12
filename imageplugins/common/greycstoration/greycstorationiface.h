/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at gmail dot com>
 * Date   : 2007-12-03
 * Description : Greycstoration interface.
 * 
 * Copyright 2007 by Gilles Caulier
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
 
#ifndef GREYCSTORATIONIFACE_H
#define GREYCSTORATIONIFACE_H

// Digikam includes.

#include <digikamheaders.h>

// Local includes.
 
#include "greycstorationsettings.h"

class QObject;
class QImage;

namespace DigikamImagePlugins
{

class GreycstorationIfacePriv;

class GreycstorationIface : public Digikam::DImgThreadedFilter
{

public:

    GreycstorationIface(Digikam::DImg *orgImage,
                        GreycstorationSettings settings, 
                        bool restoreMode=true, bool inpaintMode=false, bool resizeMode=false, 
                        int newWidth=0, int newHeight=0,
                        QImage *inPaintingMask=0, QObject *parent=0);
    
    ~GreycstorationIface();

private:  

    void initFilter();
    void filterImage();
    void cleanupFilter();

    void restoration();
    void inpainting();
    void resize();
    
private:

    GreycstorationIfacePriv *d;
};    

}  // NameSpace DigikamImagePlugins

#endif /* GREYCSTORATIONIFACE_H */
