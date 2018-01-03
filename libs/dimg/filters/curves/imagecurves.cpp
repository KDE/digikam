/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-12-01
 * Description : image curves manipulation methods.
 *
 * Copyright (C) 2004-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "imagecurves.h"

// C++ includes

#include <cstdio>
#include <cmath>
#include <cstring>
#include <cstdlib>
#include <cerrno>

// Qt includes

#include <QFile>
#include <QSharedData>
#include <QDataStream>

// Local includes

#include "digikam_debug.h"
#include "curvescontainer.h"
#include "filteraction.h"
#include "digikam_globals.h"

namespace Digikam
{

class ImageCurves::Private : public QSharedData
{

public:

    struct _Curves
    {
        /**
         * Curve types by channels (Smooth or Free).
         */
        ImageCurves::CurveType curve_type[ImageCurves::NUM_CHANNELS];
        /**
         * Curve main points in Smooth mode ([channel][point id][x,y])
         */
        int                    points[ImageCurves::NUM_CHANNELS][ImageCurves::NUM_POINTS][2];
        /**
         * Curve values by channels
         */
        unsigned short         curve[ImageCurves::NUM_CHANNELS][NUM_SEGMENTS_16BIT];
    };

    struct _Lut
    {
        unsigned short** luts;
        int              nchannels;
    };

public:

    Private() :
        curves(0),
        lut(0),
        segmentMax(0),
        dirty(false)
    {
    }

    ~Private()
    {
        if (lut)
        {
            freeLutData();
            delete lut;
        }

        if (curves)
        {
            delete curves;
        }
    }

    void init(bool sixteenBit)
    {
        lut        = new Private::_Lut;
        lut->luts  = NULL;
        curves     = new Private::_Curves;
        segmentMax = sixteenBit ? MAX_SEGMENT_16BIT : MAX_SEGMENT_8BIT;
    }

    bool isPointEnabled(const QPoint& point) const
    {
        return (point.x() > - 1) && (point.y() > -1);
    }

    void freeLutData()
    {
        if (lut->luts)
        {
            for (int i = 0 ; i < lut->nchannels ; ++i)
            {
                delete [] lut->luts[i];
            }

            delete [] lut->luts;
        }
    }

    // Curves data.
    struct _Curves* curves;

    // Lut data.
    struct _Lut*    lut;

    int             segmentMax;

    bool            dirty;
};

ImageCurves::CRMatrix CR_basis =
{
    { -0.5,  1.5, -1.5,  0.5 },
    {  1.0, -2.5,  2.0, -0.5 },
    { -0.5,  0.0,  0.5,  0.0 },
    {  0.0,  1.0,  0.0,  0.0 },
};

ImageCurves::ImageCurves(bool sixteenBit)
    : d(new Private)
{
    d->init(sixteenBit);
    curvesReset();
}

ImageCurves::ImageCurves(const CurvesContainer& container)
    : d(new Private)
{
    d->init(container.sixteenBit);
    curvesReset();
    setContainer(container);
}

ImageCurves::ImageCurves(const ImageCurves& other)
    : d(other.d)
{
}

ImageCurves::~ImageCurves()
{
}

ImageCurves& ImageCurves::operator=(const ImageCurves& other)
{
    d = other.d;
    return *this;
}

void ImageCurves::fillFromOtherCurves(ImageCurves* const otherCurves)
{
    //qCDebug(DIGIKAM_DIMG_LOG) << "Filling this curve from other curve " << otherCurves;

    curvesReset();

    // if the other curves have the same bit depth, simply copy their data
    if (isSixteenBits() == otherCurves->isSixteenBits())
    {
        //qCDebug(DIGIKAM_DIMG_LOG) << "Both curves have same type: isSixteenBits = " << isSixteenBits();

        for (int channel = 0; channel < NUM_CHANNELS; ++channel)
        {
            if (otherCurves->getCurveType(channel) == CURVE_SMOOTH)
            {
                //qCDebug(DIGIKAM_DIMG_LOG) << "Other is CURVE_SMOOTH";
                setCurveType(channel, CURVE_SMOOTH);

                for (int point = 0; point < NUM_POINTS; ++point)
                {
                    QPoint p = otherCurves->getCurvePoint(channel, point);

                    if (d->isPointEnabled(p))
                    {
                        setCurvePoint(channel, point, p);
                    }
                }
            }
            else
            {
                //qCDebug(DIGIKAM_DIMG_LOG) << "Other is CURVE_FREE";
                setCurveType(channel, CURVE_FREE);

                for (int i = 0 ; i <= d->segmentMax ; ++i)
                {
                    setCurveValue(channel, i, otherCurves->getCurveValue(channel, i));
                }
            }
        }
    }
    // other curve is 8 bit and this curve is 16 bit
    else if (isSixteenBits() && !otherCurves->isSixteenBits())
    {
        //qCDebug(DIGIKAM_DIMG_LOG) << "This curve is 16 bit and the other is 8 bit";

        for (int channel = 0; channel < NUM_CHANNELS; ++channel)
        {
            if (otherCurves->getCurveType(channel) == CURVE_SMOOTH)
            {
                //qCDebug(DIGIKAM_DIMG_LOG) << "Other is CURVE_SMOOTH";
                setCurveType(channel, CURVE_SMOOTH);

                for (int point = 0; point < NUM_POINTS; ++point)
                {
                    QPoint p = otherCurves->getCurvePoint(channel, point);

                    if (d->isPointEnabled(p))
                    {
                        p.setX(p.x() * MULTIPLIER_16BIT);
                        p.setY(p.y() * MULTIPLIER_16BIT);
                        setCurvePoint(channel, point, p);
                    }
                }
            }
            else
            {
                //qCDebug(DIGIKAM_DIMG_LOG) << "Other is CURVE_FREE";
                setCurveType(channel, CURVE_FREE);

                for (int i = 0 ; i <= d->segmentMax ; ++i)
                {
                    setCurveValue(channel, i * MULTIPLIER_16BIT, otherCurves->getCurveValue(channel, i) * MULTIPLIER_16BIT);
                }
            }
        }
    }
    // other curve is 16 bit and this is 8 bit
    else if (!isSixteenBits() && otherCurves->isSixteenBits())
    {
        //qCDebug(DIGIKAM_DIMG_LOG) << "This curve is 8 bit and the other is 16 bit";

        for (int channel = 0; channel < NUM_CHANNELS; ++channel)
        {
            if (otherCurves->getCurveType(channel) == CURVE_SMOOTH)
            {
                //qCDebug(DIGIKAM_DIMG_LOG) << "Other is CURVE_SMOOTH";
                setCurveType(channel, CURVE_SMOOTH);

                //qCDebug(DIGIKAM_DIMG_LOG) << "Adopting points of channel " << channel;
                for (int point = 0; point < NUM_POINTS; ++point)
                {
                    QPoint p = otherCurves->getCurvePoint(channel, point);

                    //qCDebug(DIGIKAM_DIMG_LOG) << "Point " << point << " in original is " << p;
                    if (d->isPointEnabled(p))
                    {
                        p.setX(p.x() / MULTIPLIER_16BIT);
                        p.setY(p.y() / MULTIPLIER_16BIT);
                        setCurvePoint(channel, point, p);
                        //qCDebug(DIGIKAM_DIMG_LOG) << "Setting curve point " << point << " to " << getCurvePoint(channel, point);
                    }
                    else
                    {
                        //qCDebug(DIGIKAM_DIMG_LOG) << "ignoring this point";
                    }
                }
            }
            else
            {
                //qCDebug(DIGIKAM_DIMG_LOG) << "Other is CURVE_FREE";
                setCurveType(channel, CURVE_FREE);

                for (int i = 0 ; i <= d->segmentMax ; ++i)
                {
                    setCurveValue(channel, i, otherCurves->getCurveValue(channel, i * MULTIPLIER_16BIT) / MULTIPLIER_16BIT);
                }
            }
        }
    }
    else
    {
        qCDebug(DIGIKAM_DIMG_LOG) << "Bad logic error, could not fill one curve into another";
    }

    // invoke calculations once
    curvesCalculateAllCurves();
}

bool ImageCurves::isDirty() const
{
    return d->dirty;
}

bool ImageCurves::isSixteenBits() const
{
    return (d->segmentMax == MAX_SEGMENT_16BIT);
}

void ImageCurves::curvesReset()
{
    memset(d->curves, 0, sizeof(struct Private::_Curves));
    d->freeLutData();
    d->lut->luts      = NULL;
    d->lut->nchannels = 0;
    d->dirty          = false;

    for (int channel = 0 ; channel < NUM_CHANNELS ; ++channel)
    {
        setCurveType(channel, CURVE_SMOOTH);
        curvesChannelReset(channel);
    }
}

void ImageCurves::curvesChannelReset(int channel)
{
    int j;

    if (!d->curves)
    {
        return;
    }

    // Construct a linear curve.

    for (j = 0 ; j <= d->segmentMax ; ++j)
    {
        d->curves->curve[channel][j] = j;
    }

    // Init coordinates points to null.

    for (j = 0 ; j < NUM_POINTS ; ++j)
    {
        d->curves->points[channel][j][0] = -1;
        d->curves->points[channel][j][1] = -1;
    }

    // First and last points init.

    d->curves->points[channel][0][0]              = 0;
    d->curves->points[channel][0][1]              = 0;
    d->curves->points[channel][NUM_POINTS - 1][0] = d->segmentMax;
    d->curves->points[channel][NUM_POINTS - 1][1] = d->segmentMax;
}

void ImageCurves::curvesCalculateCurve(int channel)
{
    int i;
    int points[NUM_POINTS];
    int num_pts;
    int p1, p2, p3, p4;

    if (!d->curves)
    {
        return;
    }

    switch (d->curves->curve_type[channel])
    {
        case CURVE_FREE:
            break;

        case CURVE_SMOOTH:
        {
            //  Cycle through the curves

            num_pts = 0;

            for (i = 0 ; i < ImageCurves::NUM_POINTS ; ++i)
                if (d->curves->points[channel][i][0] != -1)
                {
                    points[num_pts++] = i;
                }

            //  Initialize boundary curve points

            if (num_pts != 0)
            {
                for (i = 0 ; i < d->curves->points[channel][points[0]][0] ; ++i)
                {
                    d->curves->curve[channel][i] = d->curves->points[channel][points[0]][1];
                }

                for (i = d->curves->points[channel][points[num_pts - 1]][0] ; i <= d->segmentMax ; ++i)
                {
                    d->curves->curve[channel][i] = d->curves->points[channel][points[num_pts - 1]][1];
                }
            }

            for (i = 0 ; i < num_pts - 1 ; ++i)
            {
                p1 = (i == 0) ? points[i] : points[(i - 1)];
                p2 = points[i];
                p3 = points[(i + 1)];
                p4 = (i == (num_pts - 2)) ? points[(num_pts - 1)] : points[(i + 2)];

                curvesPlotCurve(channel, p1, p2, p3, p4);
            }

            // Ensure that the control points are used exactly

            for (i = 0 ; i < num_pts ; ++i)
            {
                int x, y;

                x = d->curves->points[channel][points[i]][0];
                y = d->curves->points[channel][points[i]][1];
                d->curves->curve[channel][x] = y;
            }

            break;
        }
    }
}

float ImageCurves::curvesLutFunc(int n_channels, int channel, float value)
{
    float  f;
    int    index;
    double inten;
    int    j;

    if (!d->curves)
    {
        return 0.0;
    }

    if (n_channels == 1)
    {
        j = 0;
    }
    else
    {
        j = channel + 1;
    }

    inten = value;

    // For color images this runs through the loop with j = channel +1
    // the first time and j = 0 the second time.

    // For bw images this runs through the loop with j = 0 the first and
    // only time.

    for (; j >= 0 ; j -= (channel + 1))
    {
        // Don't apply the overall curve to the alpha channel.

        if (j == 0 && (n_channels == 2 || n_channels == 4) && channel == n_channels - 1)
        {
            return inten;
        }

        if (inten < 0.0)
        {
            inten = d->curves->curve[j][0] / (float)d->segmentMax;
        }
        else if (inten >= 1.0)
        {
            inten = d->curves->curve[j][d->segmentMax] / (float)(d->segmentMax);
        }
        else       // interpolate the curve.
        {
            index = (int)floor(inten * (float)(d->segmentMax));
            f = inten * (float)(d->segmentMax) - index;
            inten = ((1.0 - f) * d->curves->curve[j][index    ] +
                     (f) * d->curves->curve[j][index + 1]) / (float)(d->segmentMax);
        }
    }

    return inten;
}

void ImageCurves::curvesPlotCurve(int channel, int p1, int p2, int p3, int p4)
{
    CRMatrix geometry;
    CRMatrix tmp1, tmp2;
    CRMatrix deltas;
    double   x, dx, dx2, dx3;
    double   y, dy, dy2, dy3;
    double   d1, d2, d3;
    int      lastx, lasty;
    int      newx, newy;
    int      i;
    int      loopdiv = d->segmentMax * 3;

    if (!d->curves)
    {
        return;
    }

    // Construct the geometry matrix from the segment.

    for (i = 0 ; i < 4 ; ++i)
    {
        geometry[i][2] = 0;
        geometry[i][3] = 0;
    }

    for (i = 0 ; i < 2 ; ++i)
    {
        geometry[0][i] = d->curves->points[channel][p1][i];
        geometry[1][i] = d->curves->points[channel][p2][i];
        geometry[2][i] = d->curves->points[channel][p3][i];
        geometry[3][i] = d->curves->points[channel][p4][i];
    }

    // Subdivide the curve 1000 times.
    // n can be adjusted to give a finer or coarser curve.

    d1 = 1.0 / loopdiv;
    d2 = d1 * d1;
    d3 = d1 * d1 * d1;

    // Construct a temporary matrix for determining the forward differencing deltas.

    tmp2[0][0] = 0;
    tmp2[0][1] = 0;
    tmp2[0][2] = 0;
    tmp2[0][3] = 1;
    tmp2[1][0] = d3;
    tmp2[1][1] = d2;
    tmp2[1][2] = d1;
    tmp2[1][3] = 0;
    tmp2[2][0] = 6 * d3;
    tmp2[2][1] = 2 * d2;
    tmp2[2][2] = 0;
    tmp2[2][3] = 0;
    tmp2[3][0] = 6 * d3;
    tmp2[3][1] = 0;
    tmp2[3][2] = 0;
    tmp2[3][3] = 0;

    // Compose the basis and geometry matrices.

    curvesCRCompose(CR_basis, geometry, tmp1);

    // Compose the above results to get the deltas matrix.

    curvesCRCompose(tmp2, tmp1, deltas);

    // Extract the x deltas.

    x   = deltas[0][0];
    dx  = deltas[1][0];
    dx2 = deltas[2][0];
    dx3 = deltas[3][0];

    // Extract the y deltas.

    y   = deltas[0][1];
    dy  = deltas[1][1];
    dy2 = deltas[2][1];
    dy3 = deltas[3][1];

    lastx = (int)CLAMP(x, 0.0, (double)d->segmentMax);
    lasty = (int)CLAMP(y, 0.0, (double)d->segmentMax);

    d->curves->curve[channel][lastx] = lasty;

    // Loop over the curve.

    for (i = 0 ; i < loopdiv ; ++i)
    {
        // Increment the x values.

        x   += dx;
        dx  += dx2;
        dx2 += dx3;

        // Increment the y values.

        y   += dy;
        dy  += dy2;
        dy2 += dy3;

        newx = CLAMP((int)lround(x), 0, d->segmentMax);
        newy = CLAMP((int)lround(y), 0, d->segmentMax);

        // If this point is different than the last one...then draw it.

        if ((lastx != newx) || (lasty != newy))
        {
            d->curves->curve[channel][newx] = newy;
        }

        lastx = newx;
        lasty = newy;
    }
}

void ImageCurves::curvesCRCompose(CRMatrix a, CRMatrix b, CRMatrix ab)
{
    int i, j;

    for (i = 0 ; i < 4 ; ++i)
    {
        for (j = 0 ; j < 4 ; ++j)
        {
            ab[i][j] = (a[i][0] * b[0][j] +
                        a[i][1] * b[1][j] +
                        a[i][2] * b[2][j] +
                        a[i][3] * b[3][j]);
        }
    }
}

void ImageCurves::curvesLutSetup(int nchannels)
{
    int    i;
    uint   v;
    double val;

    curvesCalculateAllCurves();

    d->freeLutData();

    d->lut->nchannels = nchannels;
    d->lut->luts      = new unsigned short*[d->lut->nchannels];

    for (i = 0 ; i < d->lut->nchannels ; ++i)
    {
        d->lut->luts[i] = new unsigned short[d->segmentMax + 1];

        for (v = 0 ; v <= (uint)d->segmentMax ; ++v)
        {
            // To add gamma correction use func(v ^ g) ^ 1/g instead.

            val = (double)(d->segmentMax) * curvesLutFunc(d->lut->nchannels, i, v / (float)(d->segmentMax)) + 0.5;

            d->lut->luts[i][v] = (unsigned short)CLAMP(val, 0.0, (double)d->segmentMax);
        }
    }
}

void ImageCurves::curvesLutProcess(uchar* const srcPR, uchar* const destPR, int w, int h)
{
    unsigned short* lut0 = NULL, *lut1 = NULL, *lut2 = NULL, *lut3 = NULL;
    int i;

    if (d->lut->nchannels > 0)
    {
        lut0 = d->lut->luts[0];
    }

    if (d->lut->nchannels > 1)
    {
        lut1 = d->lut->luts[1];
    }

    if (d->lut->nchannels > 2)
    {
        lut2 = d->lut->luts[2];
    }

    if (d->lut->nchannels > 3)
    {
        lut3 = d->lut->luts[3];
    }

    if (!isSixteenBits())        // 8 bits image.
    {
        uchar red, green, blue, alpha;
        uchar* ptr = srcPR;
        uchar* dst = destPR;

        for (i = 0 ; i < w * h ; ++i)
        {
            blue  = ptr[0];
            green = ptr[1];
            red   = ptr[2];
            alpha = ptr[3];

            if (d->lut->nchannels > 0)
            {
                red = lut0[red];
            }

            if (d->lut->nchannels > 1)
            {
                green = lut1[green];
            }

            if (d->lut->nchannels > 2)
            {
                blue = lut2[blue];
            }

            if (d->lut->nchannels > 3)
            {
                alpha = lut3[alpha];
            }

            dst[0] = blue;
            dst[1] = green;
            dst[2] = red;
            dst[3] = alpha;

            ptr += 4;
            dst += 4;
        }
    }
    else               // 16 bits image.
    {
        unsigned short red, green, blue, alpha;
        unsigned short* ptr = reinterpret_cast<unsigned short*>(srcPR);
        unsigned short* dst = reinterpret_cast<unsigned short*>(destPR);

        for (i = 0 ; i < w * h ; ++i)
        {
            blue  = ptr[0];
            green = ptr[1];
            red   = ptr[2];
            alpha = ptr[3];

            if (d->lut->nchannels > 0)
            {
                red = lut0[red];
            }

            if (d->lut->nchannels > 1)
            {
                green = lut1[green];
            }

            if (d->lut->nchannels > 2)
            {
                blue = lut2[blue];
            }

            if (d->lut->nchannels > 3)
            {
                alpha = lut3[alpha];
            }

            dst[0] = blue;
            dst[1] = green;
            dst[2] = red;
            dst[3] = alpha;

            ptr += 4;
            dst += 4;
        }
    }
}

QPoint ImageCurves::getDisabledValue()
{
    return QPoint(-1, -1);
}

bool ImageCurves::isCurvePointEnabled(int channel, int point) const
{
    if (d->curves && channel >= 0 && channel < NUM_CHANNELS
        && point >= 0 && point < NUM_POINTS
        && d->curves->points[channel][point][0] >= 0
        && d->curves->points[channel][point][1] >= 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

QPoint ImageCurves::getCurvePoint(int channel, int point) const
{
    if (d->curves &&
        channel >= 0 && channel < NUM_CHANNELS &&
        point >= 0 && point < NUM_POINTS)
    {
        return(QPoint(d->curves->points[channel][point][0],
                      d->curves->points[channel][point][1]));
    }

    return getDisabledValue();
}

QPolygon ImageCurves::getCurvePoints(int channel) const
{
    QPolygon array(NUM_POINTS);

    if (d->curves && channel >= 0 && channel < NUM_CHANNELS)
    {
        for (int j = 0 ; j < NUM_POINTS ; ++j)
        {
            array.setPoint(j, getCurvePoint(channel, j));
        }
    }

    return array;
}

int ImageCurves::getCurveValue(int channel, int bin) const
{
    if (d->curves &&
        channel >= 0 && channel < NUM_CHANNELS &&
        bin >= 0 && bin <= d->segmentMax)
    {
        return(d->curves->curve[channel][bin]);
    }

    return 0;
}

QPolygon ImageCurves::getCurveValues(int channel) const
{
    QPolygon array(d->segmentMax + 1);

    if (d->curves && channel >= 0 && channel < NUM_CHANNELS)
    {
        for (int j = 0 ; j <= d->segmentMax ; ++j)
        {
            array.setPoint(j, QPoint(j, getCurveValue(channel, j)));
        }
    }

    return array;
}

int ImageCurves::getCurvePointX(int channel, int point) const
{
    if (d->curves &&
        channel >= 0 && channel < NUM_CHANNELS &&
        point >= 0 && point < NUM_POINTS)
    {
        return(d->curves->points[channel][point][0]);
    }

    return(-1);
}

int ImageCurves::getCurvePointY(int channel, int point) const
{
    if (d->curves &&
        channel >= 0 && channel < NUM_CHANNELS &&
        point >= 0 && point < NUM_POINTS)
    {
        return(d->curves->points[channel][point][1]);
    }

    return (-1);
}

ImageCurves::CurveType ImageCurves::getCurveType(int channel) const
{
    if (d->curves && channel >= 0 && channel < NUM_CHANNELS)
    {
        return ((ImageCurves::CurveType) d->curves->curve_type[channel]);
    }

    return CURVE_SMOOTH;
}

CurvesContainer ImageCurves::getContainer() const
{
    CurveType type = CURVE_SMOOTH;

    for (int i = 0; i < ColorChannels; ++i)
    {
        if ((type = getCurveType(i)) == CURVE_FREE)
        {
            type = CURVE_FREE;
            break;
        }
    }

    CurvesContainer c(type, isSixteenBits());
    c.initialize();

    if (isLinear())
    {
        return c;
    }

    if (type == CURVE_FREE)
    {
        for (int i = 0; i < ColorChannels; ++i)
        {
            c.values[i] = getCurveValues(i);
        }
    }
    else
    {
        for (int i = 0; i < ColorChannels; ++i)
        {
            c.values[i] = getCurvePoints(i);
        }
    }

    return c;
}

CurvesContainer ImageCurves::getContainer(int channel) const
{
    CurveType type = getCurveType(channel);
    CurvesContainer c(type, isSixteenBits());

    if (isLinear(channel))
    {
        return c;
    }

    if (type == CURVE_FREE)
    {
        c.values[channel] = getCurveValues(channel);
    }
    else
    {
        c.values[channel] = getCurvePoints(channel);
    }

    return c;
}

void ImageCurves::setContainer(const CurvesContainer& container)
{
    if (container.curvesType == CURVE_FREE)
    {
        for (int i = 0; i < ColorChannels; ++i)
        {
            setCurveType(i, CURVE_FREE);
            setCurveValues(i, container.values[i]);
        }
    }
    else
    {
        for (int i = 0; i < ColorChannels; ++i)
        {
            setCurveType(i, CURVE_SMOOTH);
            setCurvePoints(i, container.values[i]);
        }
    }
}

void ImageCurves::setCurveValue(int channel, int bin, int val)
{
    if (d->curves &&
        channel >= 0 && channel < NUM_CHANNELS &&
        bin >= 0 && bin <= d->segmentMax)
    {
        d->dirty = true;
        d->curves->curve[channel][bin] = val;
    }
}

void ImageCurves::setCurveValues(int channel, const QPolygon& vals)
{
    //qCDebug(DIGIKAM_DIMG_LOG) << "vals size: " << vals.size();
    //qCDebug(DIGIKAM_DIMG_LOG) << "segmentMax: " << d->segmentMax + 1;

    if (d->curves && channel >= 0 && channel < NUM_CHANNELS)
    {
        if (vals.isEmpty())
        {
            //qCDebug(DIGIKAM_DIMG_LOG) << "No curves values to assign: reset";
            curvesChannelReset(channel);
        }
        // Bits depth are different ?
        else if (vals.size() != d->segmentMax + 1)
        {
            int index;

            if (vals.size() == 256)
            {
                //qCDebug(DIGIKAM_DIMG_LOG) << "8 to 16 bits curves transform";

                // 8 to 16 bits.
                ImageCurves curve8(false);
                ImageCurves curve16(true);

                for (int i = 0; i <= 16; ++i)
                {
                    index = CLAMP(i * 255 / 16, 0, 255);
                    curve8.setCurvePoint(channel, i, QPoint(index, vals.point(index).y()));
                }

                curve8.curvesCalculateCurve(channel);
                curve16.fillFromOtherCurves(&curve8);

                for (int j = 0 ; j <= d->segmentMax ; ++j)
                {
                    setCurveValue(channel, j, curve16.getCurveValue(channel, j));
                }
            }
            else
            {
                //qCDebug(DIGIKAM_DIMG_LOG) << "16 to 8 bits curves transform";

                // 16 to 8 bits.
                ImageCurves curve8(false);
                ImageCurves curve16(true);

                for (int i = 0; i <= 16; ++i)
                {
                    index = CLAMP(i * 65535 / 16, 0, 65535);
                    curve16.setCurvePoint(channel, i, QPoint(index, vals.point(index).y()));
                }

                curve16.curvesCalculateCurve(channel);
                curve8.fillFromOtherCurves(&curve16);

                for (int j = 0 ; j <= d->segmentMax ; ++j)
                {
                    setCurveValue(channel, j, curve8.getCurveValue(channel, j));
                }
            }
        }
        else
        {
            //qCDebug(DIGIKAM_DIMG_LOG) << "Assign curves values directly";

            for (int j = 0 ; j <= d->segmentMax ; ++j)
            {
                setCurveValue(channel, j, vals.point(j).y());
            }
        }
    }
}

void ImageCurves::setCurvePoint(int channel, int point, const QPoint& val)
{
    if (d->curves &&
        channel >= 0 && channel < NUM_CHANNELS &&
        point >= 0 && point < NUM_POINTS &&
        val.x() >= -1 && val.x() <= d->segmentMax && // x can be equal to -1
        val.y() >= 0 && val.y() <= d->segmentMax)    // if the current point is disable !!!
    {
        d->dirty = true;
        d->curves->points[channel][point][0] = val.x();
        d->curves->points[channel][point][1] = val.y();
    }
}

void ImageCurves::setCurvePoints(int channel, const QPolygon& vals)
{
    if (d->curves &&
        channel >= 0 && channel < NUM_CHANNELS)
    {
        if (vals.isEmpty())
        {
            curvesChannelReset(channel);
        }
        else if (vals.size() >= NUM_POINTS)
        {
            for (int j = 0 ; j < NUM_POINTS ; ++j)
            {
                setCurvePoint(channel, j, vals.point(j));
            }
        }
        else
        {
            curvesChannelReset(channel);

            if (vals.size() == 1)
            {
                setCurvePoint(channel, NUM_POINTS / 2, vals.first());
            }
            else
            {
                for (int j = 0 ; j < vals.size() - 1 ; ++j)
                {
                    setCurvePoint(channel, j, vals.point(j));
                }

                // set last to last
                setCurvePoint(channel, NUM_POINTS - 1, vals.last());
            }
        }
    }
    else
    {
        qCDebug(DIGIKAM_DIMG_LOG) << "Curves points list not applied (nb pts " << vals.size() << " - Channel " << channel << ")";
    }
}

void ImageCurves::setCurvePointX(int channel, int point, int x)
{
    if (d->curves &&
        channel >= 0 && channel < NUM_CHANNELS &&
        point >= 0 && point < NUM_POINTS &&
        x >= -1 && x <= d->segmentMax) // x can be equal to -1 if the current point is disable !!!
    {
        d->dirty = true;
        d->curves->points[channel][point][0] = x;
    }
}

void ImageCurves::setCurvePointY(int channel, int point, int y)
{
    if (d->curves &&
        channel >= 0 && channel < NUM_CHANNELS &&
        point >= 0 && point < NUM_POINTS &&
        y >= 0 && y <= d->segmentMax)
    {
        d->dirty = true;
        d->curves->points[channel][point][1] = y;
    }
}

void ImageCurves::setCurveType(int channel, CurveType type)
{
    if (d->curves &&
        channel >= 0 && channel < NUM_CHANNELS &&
        type >= CURVE_SMOOTH && type <= CURVE_FREE)
    {
        d->curves->curve_type[channel] = type;
    }
}

void ImageCurves::setCurveType(ImageCurves::CurveType type)
{
    for (int channel = 0; channel < NUM_CHANNELS; ++channel)
    {
        setCurveType(channel, type);
    }
}

bool ImageCurves::loadCurvesFromGimpCurvesFile(const QUrl& fileUrl)
{
    // TODO : support QUrl !

    FILE* file = 0;
    int   i, j;
    int   fields;
    char  buf[50];
    int   index[NUM_CHANNELS][NUM_POINTS];
    int   value[NUM_CHANNELS][NUM_POINTS];

    file = fopen(QFile::encodeName(fileUrl.toLocalFile()).constData(), "r");

    if (!file)
    {
        return false;
    }

    if (! fgets(buf, sizeof(buf), file))
    {
        fclose(file);
        return false;
    }

    if (strcmp(buf, "# GIMP Curves File\n") != 0)
    {
        fclose(file);
        return false;
    }

    for (i = 0 ; i < NUM_CHANNELS ; ++i)
    {
        for (j = 0 ; j < NUM_POINTS ; ++j)
        {
            // FIXME: scanf without field width limits can crash with huge input data
            fields = fscanf(file, "%d %d ", &index[i][j], &value[i][j]);

            if (fields != 2)
            {
                qCWarning(DIGIKAM_DIMG_LOG) <<  "Invalid Gimp curves file!";
                fclose(file);
                return false;
            }
        }
    }

    curvesReset();

    for (i = 0 ; i < NUM_CHANNELS ; ++i)
    {
        d->curves->curve_type[i] = CURVE_SMOOTH;

        for (j = 0 ; j < NUM_POINTS ; ++j)
        {
            d->curves->points[i][j][0] = (isSixteenBits() && (index[i][j] != -1) ?
                                          index[i][j] * MULTIPLIER_16BIT : index[i][j]);
            d->curves->points[i][j][1] = (isSixteenBits() && (value[i][j] != -1) ?
                                          value[i][j] * MULTIPLIER_16BIT : value[i][j]);
        }
    }

    curvesCalculateAllCurves();

    fclose(file);
    return true;
}

void ImageCurves::curvesCalculateAllCurves()
{
    for (int i = 0 ; i < NUM_CHANNELS ; ++i)
    {
        curvesCalculateCurve(i);
    }
}

bool ImageCurves::saveCurvesToGimpCurvesFile(const QUrl& fileUrl) const
{
    // TODO : support QUrl !

    FILE* file = 0;
    int   i, j;
    int   index;

    file = fopen(QFile::encodeName(fileUrl.toLocalFile()).constData(), "w");

    if (!file)
    {
        return false;
    }

    for (i = 0 ; i < NUM_CHANNELS ; ++i)
    {
        if (d->curves->curve_type[i] == CURVE_FREE)
        {
            //  Pick representative points from the curve and make them control points.

            for (j = 0 ; j <= 8 ; ++j)
            {
                index = CLAMP(j * 32, 0, d->segmentMax);
                d->curves->points[i][j * 2][0] = index;
                d->curves->points[i][j * 2][1] = d->curves->curve[i][index];
            }
        }
    }

    fprintf(file, "# GIMP Curves File\n");

    for (i = 0 ; i < NUM_CHANNELS ; ++i)
    {
        for (j = 0 ; j < NUM_POINTS ; ++j)
        {
            fprintf(file, "%d %d ",
                    (isSixteenBits() && (d->curves->points[i][j][0] != -1) ?
                     d->curves->points[i][j][0] / MULTIPLIER_16BIT : d->curves->points[i][j][0]),
                    (isSixteenBits() && (d->curves->points[i][j][1] != -1) ?
                     d->curves->points[i][j][1] / MULTIPLIER_16BIT : d->curves->points[i][j][1]));

            fprintf(file, "\n");
        }
    }

    fflush(file);
    fclose(file);

    return true;
}

bool ImageCurves::isLinear() const
{
    for (int i = 0; i < NUM_CHANNELS; ++i)
    {
        if (!isLinear(i))
        {
            return false;
        }
    }

    return true;
}

bool ImageCurves::isLinear(int channel) const
{
    if (!d->curves || channel < 0 || channel >= NUM_CHANNELS)
    {
        return false;
    }

    if (d->curves->curve_type[channel] == CURVE_FREE)
    {
        for (int j = 0; j < d->segmentMax; ++j)
            if (d->curves->curve[channel][j] != j)
            {
                return false;
            }

        return true;
    }
    else
    {
        bool hasFirst = false;
        bool hasLast  = false;

        // find out number of valid points
        for (int j = 0; j < NUM_POINTS; ++j)
        {
            int x = d->curves->points[channel][j][0];
            int y = d->curves->points[channel][j][1];

            // we allow one first point (0,0), one second and last point(max,max), and the rest must be disabled
            if (x > -1 && y > -1)
            {
                if (!hasFirst && !hasLast && x == 0 && y == 0)
                {
                    hasFirst = true;
                }
                else if (hasFirst && !hasLast && x == d->segmentMax && y == d->segmentMax)
                {
                    hasLast = true;
                }
                else
                {
                    return false;
                }
            }
        }

        return true;
    }
}

/**
 * Binary format:
 *
 * Version      1       :16
 * Type         0,1,2   : 8
 * Bytes depth  1,2     : 8
 * reserved             :32
 * count                :32
 *
 * Type 0 (linear curve):
 * Type 1 (smooth curve):
 *      for (0...count)
 *          point.x     :32
 *          point.y     :32
 * Type 2 (free curve):
 *      for (0...count)
 *          if (Bytes depth == 1)
 *              value   : 8
 *          else if (Bytes depth == 2)
 *              value   :16
 *
 * In Big Endian byte order.
 * Data then converted to base64.
 */

QByteArray ImageCurves::channelToBinary(int channel) const
{
    if (!d->curves || channel < 0 || channel >= NUM_CHANNELS)
    {
        return QByteArray();
    }

    QByteArray data;
    QDataStream s(&data, QIODevice::WriteOnly);

    quint8 type;

    if (isLinear(channel))
    {
        type = 0;
    }
    else if (d->curves->curve_type[channel] == CURVE_SMOOTH)
    {
        type = 1;
    }
    else //if (d->curves->curve_type[channel] == CURVE_FREE)
    {
        type = 2;
    }

    s << (quint16)1; // version
    s << (quint8)type; // type
    s << (quint8)(isSixteenBits() ? 2 : 1); // bytes depth
    s << (quint32)0; // reserved

    if (type == 0)
    {
        // write only a zero count for linear curve
        s << (quint32)0;
    }
    else if (type == 1)
    {
        quint32 count = 0;

        // find out number of valid points
        for (int j = 0; j < NUM_POINTS; ++j)
        {
            if (d->curves->points[channel][j][0] > -1 && d->curves->points[channel][j][1] > -1)
            {
                count++;
            }
        }

        s << (quint32)count; // number of stored points,

        for (int j = 0; j < NUM_POINTS; ++j)
        {
            if (d->curves->points[channel][j][0] > -1 && d->curves->points[channel][j][1] > -1)
            {
                s << (qint32)d->curves->points[channel][j][0];
                s << (qint32)d->curves->points[channel][j][1];
            }
        }
    }
    else if (type == 2)
    {
        s << (quint32)d->segmentMax; // number of stored segments

        if (isSixteenBits())
        {
            for (int j = 0; j < d->segmentMax; ++j)
            {
                s << (quint16)d->curves->curve[channel][j];
            }
        }
        else
        {
            for (int j = 0; j < d->segmentMax; ++j)
            {
                s << (quint8)d->curves->curve[channel][j];
            }
        }
    }

    return data;
}

bool ImageCurves::setChannelFromBinary(int channel, const QByteArray& data)
{
    if (!d->curves || channel < 0 || channel >= NUM_CHANNELS)
    {
        return false;
    }

    if (data.isEmpty())
    {
        curvesChannelReset(channel);
        return false;
    }

    QDataStream s(data);

    quint32 nothing, count;
    quint16 version;
    quint8  type, depth;

    s >> version;

    if (version != 1)
    {
        return false;
    }

    s >> type;

    if (type > 2)
    {
        return false;
    }

    s >> depth;

    if ((depth == 1 && isSixteenBits()) || (depth == 2 && !isSixteenBits()) || depth <= 0 || depth > 2)
    {
        return false;
    }

    s >> nothing;
    s >> count;

    if (type == 0)
    {
        // linear
        setCurveType(channel, CURVE_SMOOTH);
        curvesChannelReset(channel);
    }
    else if (type == 1)
    {
        setCurveType(channel, CURVE_SMOOTH);

        uint usedCount = qMin((uint)NUM_POINTS, count);
        QPolygon p(usedCount);
        quint32 x, y;

        for (uint j = 0; j < usedCount; ++j)
        {
            s >> x;
            s >> y;
            p.setPoint(j, x, y);
        }

        setCurvePoints(channel, p);
    }
    else // (type == 2)
    {
        if (count != (uint)d->segmentMax || s.atEnd())
        {
            return false;
        }

        setCurveType(channel, CURVE_FREE);

        if (isSixteenBits())
        {
            quint16 data;

            for (int j = 0; j < d->segmentMax; ++j)
            {
                s >> data;
                d->curves->curve[channel][j] = data;
            }
        }
        else
        {
            quint8 data;

            for (int j = 0; j < d->segmentMax; ++j)
            {
                s >> data;
                d->curves->curve[channel][j] = data;
            }
        }

    }

    return true;
}

}  // namespace Digikam
