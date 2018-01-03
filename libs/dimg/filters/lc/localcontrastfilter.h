/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-09
 * Description : Enhance image with local contrasts (as human eye does).
 *               LDR ToneMapper <http://zynaddsubfx.sourceforge.net/other/tonemapping>
 *
 * Copyright (C) 2009      by Julien Pontabry <julien dot pontabry at gmail dot com>
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2010      by Martin Klapetek <martin dot klapetek at gmail dot com>
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

#ifndef LOCALCONTRASTFILTER_H
#define LOCALCONTRASTFILTER_H

// Qt includes

#include <QImage>

// Local includes

#include "digikam_export.h"
#include "dimgthreadedfilter.h"
#include "digikam_globals.h"
#include "localcontrastcontainer.h"

namespace Digikam
{

class DIGIKAM_EXPORT LocalContrastFilter : public DImgThreadedFilter
{

public:

    explicit LocalContrastFilter(QObject* const parent = 0);
    explicit LocalContrastFilter(DImg* const image, QObject* const parent=0, const LocalContrastContainer& par=LocalContrastContainer());
    ~LocalContrastFilter();

    static QString          FilterIdentifier()
    {
        return QLatin1String("digikam:LocalContrastFilter");
    }

    static QString          DisplayableName()
    {
        return QString::fromUtf8(I18N_NOOP("Local Contrast Filter"));
    }

    static QList<int>       SupportedVersions()
    {
        return QList<int>() << 2;
    }

    static int              CurrentVersion()
    {
        return 2;
    }

    virtual QString         filterIdentifier() const
    {
        return FilterIdentifier();
    }

    virtual FilterAction    filterAction();

    void                    readParameters(const FilterAction& action);

private:

    struct Args
    {
        uint   start;
        uint   stop;
        float  a;
        float* data;
        int    sizex;
        int    sizey;
        float  blur;
        float  denormal_remove;
    };

private:

    void filterImage();

    void  processRgbImage(float* const img, int sizex, int sizey);
    void  process8bitRgbImage(unsigned char* const img, int sizex, int sizey);
    void  process16bitRgbImage(unsigned short* const img, int sizex, int sizey);

    float func(float x1, float x2);

    void  inplaceBlur(float* const data, int sizex, int sizey, float blur);
    void  stretchContrast(float* const data, int datasize);

    inline void rgb2hsv(const float& r, const float& g, const float& b, float& h, float& s, float& v);
    inline void hsv2rgb(const float& h, const float& s, const float& v, float& r, float& g, float& b);

    void blurMultithreaded(uint start, uint stop, float* const img, float* const blurimage);
    void saturationMultithreaded(uint start, uint stop, float* const img, float* const srcimg);

    void inplaceBlurYMultithreaded(const Args& prm);
    void inplaceBlurXMultithreaded(const Args& prm);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // LOCALCONTRASTFILTER_H
