/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-12-01
 * Description : image curves manipulation methods.
 * 
 * Copyright 2004-2005 by Gilles Caulier
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

#define ROUND(x) ((int) ((x) + 0.5))

// Qt includes.

#include <qpoint.h>
                                   
// KDE includes.

#include <kurl.h>

// Digikam includes.

#include "digikam_export.h"
                                                                      
namespace Digikam
{

class DIGIKAM_EXPORT ImageCurves
{

public:

enum CurveType
{
    CURVE_SMOOTH = 0,            // Smooth curve type
    CURVE_FREE                   // Freehand curve type.
};

private:

struct _Curves
{
    CurveType        curve_type[5];     // Curve types by channels (Smooth or Free).
    int              points[5][17][2];  // Curve main points in Smooth mode ([channel][point id][x,y]).
    unsigned short   curve[5][65536];   // Curve values by channels.
};

struct _Lut
{
    unsigned short **luts;
    int              nchannels;
};

public:
    
    ImageCurves(bool sixteenBit);
    ~ImageCurves();

    typedef double CRMatrix[4][4];

    // Methods for to manipulate the curves data.        
    
    void   curvesReset(void);
    void   curvesChannelReset(int channel);
    void   curvesCalculateCurve(int channel);
    float  curvesLutFunc(int n_channels, int channel, float value);
    void   curvesLutSetup(int nchannels, bool overIndicator=false);
    void   curvesLutProcess(uchar *srcPR, uchar *destPR, int w, int h);

    // Methods for to set manually the curves values.        
    
    void   setCurveValue(int channel, int bin, int val);
    void   setCurvePoint(int channel, int point, QPoint val);
    void   setCurvePointX(int channel, int point, int x);
    void   setCurvePointY(int channel, int point, int y);
    void   setCurveType(int channel, CurveType type);
    
    int    getCurveValue(int channel, int bin);
    QPoint getCurvePoint(int channel, int point);
    int    getCurvePointX(int channel, int point);
    int    getCurvePointY(int channel, int point);
    int    getCurveType(int channel);

    // Methods for to save/load the curves values to/from a Gimp curves text file.        
    
    bool   saveCurvesToGimpCurvesFile(KURL fileUrl);
    bool   loadCurvesFromGimpCurvesFile(KURL fileUrl);
    
private:

    // Curves data.
    struct _Curves *m_curves;
    
    // Lut data.
    struct _Lut    *m_lut;

    int             m_segmentMax;

private:
    
    void curvesPlotCurve(int channel, int p1, int p2, int p3, int p4);
    void curvesCRCompose(CRMatrix a, CRMatrix b, CRMatrix ab);
};

}  // NameSpace Digikam


#endif /* IMAGECURVES_H */
