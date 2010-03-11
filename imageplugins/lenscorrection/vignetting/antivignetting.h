/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-25
 * Description : Antivignetting threaded image filter.
 *
 * Copyright (C) 2005-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef ANTIVIGNETTING_H
#define ANTIVIGNETTING_H

// Local includes

#include "dimgthreadedfilter.h"

using namespace Digikam;

namespace DigikamAntiVignettingImagesPlugin
{

class AntiVignetting : public DImgThreadedFilter
{

public:

    explicit AntiVignetting(DImg* orgImage, QObject* parent=0, double density=2.0,
                            double power=1.0, double innerradius=1.0, double outerradius=1.0, double xshift=0, double yshift=0,
                            bool anti=true);

    ~AntiVignetting(){};

private:

    void filterImage();

    double         hypothenuse(double x, double y);
    int            approx(double x);
    uchar          clamp8bits(double x);
    unsigned short clamp16bits(double x);
    double         attenuation(double r1, double r2, double distcenter);
    double         real_attenuation(double r1, double r2, double distcenter);

private:

    bool   m_normalize;

    int    m_xshift;
    int    m_yshift;

    double m_density;
    double m_power;
    double m_inner_radius;
    double m_outer_radius;

    bool   m_add_vignetting;
};

}  // namespace DigikamAntiVignettingImagesPlugin

#endif /* ANTIVIGNETTING_H */
