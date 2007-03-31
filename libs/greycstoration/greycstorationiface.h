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

// Qt includes.

#include <qimage.h>

// Local includes.

#include "dimg.h"
#include "dimgthreadedfilter.h"
#include "greycstorationsettings.h"
#include "digikam_export.h"

class QObject;

namespace Digikam
{

class GreycstorationIfacePriv;

class DIGIKAM_EXPORT GreycstorationIface : public DImgThreadedFilter
{

public:

    enum MODE
    {
        Restore = 0,
        InPainting,
        Resize,
        SimpleResize    // Mode to resize image without to use Greycstoration algorithm.
    };

public:

    GreycstorationIface(DImg *orgImage,
                        GreycstorationSettings settings, 
                        int mode=Restore, 
                        int newWidth=0, int newHeight=0,
                        const QImage& inPaintingMask=QImage(), 
                        QObject *parent=0);
    
    ~GreycstorationIface();

    void stopComputation();

private:  

    void initFilter();
    void filterImage();

    void restoration();
    void inpainting();
    void resize();
    void simpleResize();
    void iterationLoop(uint iter);
    
private:

    GreycstorationIfacePriv *d;
};    

}  // NameSpace Digikam

#endif /* GREYCSTORATIONIFACE_H */
