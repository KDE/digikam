/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-01
 * Description : Content aware resizer class.
 *
 * Copyright (C) 2009 by Julien Pontabry <julien dot pontabry at ulp dot u-strasbg dot fr>
 * Copyright (C) 2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef CONTENT_AWARE_RESIZER_H
#define CONTENT_AWARE_RESIZER_H

// Qt includes

#include <QImage>

// Liquid rescale library include

#include "lqr.h"

// Local includes

#include "dcolor.h"
#include "dimgthreadedfilter.h"

using namespace Digikam;

namespace DigikamContentAwareResizingImagesPlugin
{

class ContentAwareResizerPriv;

class ContentAwareResizer : public Digikam::DImgThreadedFilter
{

public:

    ContentAwareResizer(DImg *orgImage, uint width, uint height,
                        int step=1, double rigidity=0.0, int side_switch_freq=4, 
                        LqrEnergyFuncBuiltinType func=LQR_EF_GRAD_XABS,
                        LqrResizeOrder resize_order=LQR_RES_ORDER_HOR, const QImage& mask=QImage(), 
                        bool preserve_skin_tones=false, QObject *parent=0);
    ~ContentAwareResizer();

    void progressCallback(int progress);

private:

    void cancelFilter();
    void filterImage();

    void buildBias(const QImage& mask);
    void buildSkinToneBias();
    inline bool isSkinTone(const DColor& c);

private:

    ContentAwareResizerPriv* const d;
};

} // namespace DigikamContentAwareResizingImagesPlugin

#endif /*CONTENT_AWARE_RESIZER_H*/
