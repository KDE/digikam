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

#ifndef IMAGECURVES_H
#define IMAGECURVES_H

// Qt includes

#include <QSharedDataPointer>
#include <QPoint>
#include <QPolygon>
#include <QUrl>

// Local includes

#include "digikam_globals.h"
#include "digikam_export.h"

namespace Digikam
{

class CurvesContainer;

class DIGIKAM_EXPORT ImageCurves
{

public:

    /**
     * The max number of points contained in a curve.
     */
    const static int NUM_POINTS = 17;

    /**
     * Number of channels in a curve.
     */
    const static int NUM_CHANNELS = 5;

    /**
     * Curve points have to multiplied with this value for 16 bit images.
     */
    const static int MULTIPLIER_16BIT = 255;

    enum CurveType
    {
        CURVE_SMOOTH = 0,            // Smooth curve type
        CURVE_FREE                   // Freehand curve type.
    };

    typedef double CRMatrix[4][4];

public:

    explicit ImageCurves(bool sixteenBit);
    explicit ImageCurves(const CurvesContainer& container);
    ImageCurves(const ImageCurves& other);
    ~ImageCurves();

    ImageCurves& operator=(const ImageCurves& other);

    /**
     * Fills this curves with the data supplied by another curves object. This
     * ensures that 8 and 16 bit curves are properly converted.
     *
     * @param otherCurves other curves object to adapt config from
     */
    void fillFromOtherCurves(ImageCurves* const otherCurves);


    // Methods for to manipulate the curves data.

    void   curvesReset();
    void   curvesChannelReset(int channel);
    void   curvesCalculateCurve(int channel);
    void   curvesCalculateAllCurves();
    float  curvesLutFunc(int n_channels, int channel, float value);
    void   curvesLutSetup(int nchannels);
    void   curvesLutProcess(uchar* const srcPR, uchar* const destPR, int w, int h);

    // Methods for to set manually the curves values.

    void   setCurveValue(int channel, int bin, int val);
    void   setCurvePointX(int channel, int point, int x);
    void   setCurvePointY(int channel, int point, int y);
    void   setCurveType(int channel, CurveType type);
    void   setCurveType(CurveType type);

    void   setCurvePoint(int channel, int point, const QPoint& val);
    void   setCurvePoints(int channel, const QPolygon& vals);
    void   setCurveValues(int channel, const QPolygon& vals);

    // Properties.

    bool   isDirty()                                     const;
    bool   isSixteenBits()                               const;

    int       getCurveValue(int channel, int bin)        const;
    int       getCurvePointX(int channel, int point)     const;
    int       getCurvePointY(int channel, int point)     const;
    CurveType getCurveType(int channel)                  const;

    bool     isCurvePointEnabled(int channel, int point) const;
    QPoint   getCurvePoint(int channel, int point)       const;
    QPolygon getCurvePoints(int channel)                 const;
    QPolygon getCurveValues(int channel)                 const;

    /// Note that bits depth must match
    void   setContainer(const CurvesContainer& container);

    /**
     * Returns a container with the settings for all channels of this Curves object
     */
    CurvesContainer getContainer() const;

    /**
     * Returns a container containing the values of this Curves
     * object for the given channel, and linear values for all
     * other channels.
     */
    CurvesContainer getContainer(int channel) const;

    /**
     * Returns true if the curve is linear for the given channel, or all channels.
     */
    bool isLinear(int channel) const;
    bool isLinear()            const;

    /**
     * Writes the given channel to a raw binary representation.
     * Note that 16bit free curves take a lot of memory (~85kB)
     * while all other forms take less than 400 bytes.
     */
    QByteArray channelToBinary(int channel) const;

    /**
     * Set the channel from the given raw binary representation.
     * The data is checked for validity, only on valid data true is returned.
     * Note that the bytes depth (isSixteenBits()) of the encoded
     * representation must match the depth of this curves object.
     */
    bool setChannelFromBinary(int channel, const QByteArray& array);

    /// Methods for to save/load the curves values to/from a Gimp curves text file.

    bool saveCurvesToGimpCurvesFile(const QUrl& fileUrl) const;
    bool loadCurvesFromGimpCurvesFile(const QUrl& fileUrl);

    static QPoint getDisabledValue();

private:

    void curvesPlotCurve(int channel, int p1, int p2, int p3, int p4);
    void curvesCRCompose(CRMatrix a, CRMatrix b, CRMatrix ab);
    void freeLutData();

private:

    class Private;
    QSharedDataPointer<Private> d;
};

}  // namespace Digikam

#endif /* IMAGECURVES_H */
