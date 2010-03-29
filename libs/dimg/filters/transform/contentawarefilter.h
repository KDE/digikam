/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-01
 * Description : Content aware resizer class.
 *
 * Copyright (C) 2009 by Julien Pontabry <julien dot pontabry at ulp dot u-strasbg dot fr>
 * Copyright (C) 2009-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef CONTENT_AWARE_FILTER_H
#define CONTENT_AWARE_FILTER_H

// Qt includes

#include <QImage>

// Liquid rescale library include

#include "lqr.h"

// Local includes

#include "dcolor.h"
#include "digikam_export.h"
#include "dimgthreadedfilter.h"

namespace Digikam
{

class DIGIKAM_EXPORT ContentAwareContainer
{

public:

    ContentAwareContainer()
    {
        preserve_skin_tones = false;
        width               = 0;
        height              = 0;
        step                = 1; 
        side_switch_freq    = 4; 
        rigidity            = 0.0;
        func                = LQR_EF_GRAD_XABS;
        resize_order        = LQR_RES_ORDER_HOR;
    };

    ~ContentAwareContainer(){};

public:

    bool                     preserve_skin_tones;

    uint                     width;
    uint                     height;
    
    int                      step; 
    int                      side_switch_freq;

    double                   rigidity;

    QImage                   mask; 

    LqrEnergyFuncBuiltinType func;
    LqrResizeOrder           resize_order;
};  
  
class ContentAwareFilterPriv;

class DIGIKAM_EXPORT ContentAwareFilter : public DImgThreadedFilter
{

public:

    explicit ContentAwareFilter(DImg* orgImage, QObject* parent = 0, const ContentAwareContainer& settings = ContentAwareContainer());
    ~ContentAwareFilter();

    void progressCallback(int progress);

private:

    void cancelFilter();
    void filterImage();

    void buildBias(const QImage& mask);
    void buildSkinToneBias();
    inline bool isSkinTone(const DColor& c);
    void getEnergyImage();
    
private:

    ContentAwareFilterPriv* const d;
};

} // namespace Digikam

#endif /*CONTENT_AWARE_FILTER_H*/
