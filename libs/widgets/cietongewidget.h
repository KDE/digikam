/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2006-01-10
 * Description : a widget to display CIE tonge from
 * an ICC profile.
 * 
 * Copyright 2006 by Gilles Caulier
 *
 * Any source code are inspired from lprof project and
 * Copyright (C) 1998-2001 Marti Maria
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

#ifndef CIETONGEWIDGET_H
#define CIETONGEWIDGET_H

#include <config.h>

// C++ includes.

#include <cmath>

// Qt includes.

#include <qwidget.h>
#include <qcolor.h>
#include <qpainter.h>
#include <qpixmap.h>

// Local includes

#include "lcmsprf.h"
#include "digikam_export.h"

// Others

#include LCMS_HEADER

namespace Digikam
{

class DIGIKAM_EXPORT CIETongeWidget : public QWidget
{
Q_OBJECT

private:
    

public:

    CIETongeWidget(int w, int h, QWidget *parent=0, cmsHPROFILE hMonitor=0);
    ~CIETongeWidget();

    void setProfileData(QByteArray *profileData);

protected:
    
    double        m_gridside;
    int           m_xBias;
    int           m_yBias;
    int           m_pxcols;
    int           m_pxrows;
    
    QPainter      m_pnt;
    QPixmap       m_pix;
    
    cmsHPROFILE   m_hMonitorProfile;
    cmsHPROFILE   m_hXYZProfile;
    cmsHTRANSFORM m_hXFORM;

protected:

    int  Grids(double d) const { return (int) floor(d * m_gridside + .5); };

    void OutlineTonge();
    void FillTonge();
    void DrawTongeAxis();
    void DrawTongeGrid();
    void DrawLabels();

    QRgb ColorByCoord(double x, double y);  
    void DrawSmallElipse(LPcmsCIExyY xyY, BYTE r, BYTE g, BYTE b, int sz);

    void paintEvent( QPaintEvent * );

private:

    void DrawTonge();
    void DrawColorantTriangle(LPcmsCIExyYTRIPLE Primaries);
    void DrawWhitePoint(LPcmsCIEXYZ WhitePoint);
    void DrawPatches(LPMEASUREMENT m);
    
    void MapPoint(int& icx, int& icy, LPcmsCIExyY xyY);
    void BiasedLine(int x1, int y1, int x2, int y2);
    void BiasedText(int x, int y, const char* Txt);

    void Sweep_sRGB(void);

private :
    
    MEASUREMENT     Measurement;

    cmsCIExyYTRIPLE Primaries;
    
    cmsCIEXYZ       MediaWhite;
    
};

}  // namespace Digikam

#endif /* CIETONGEWIDGET_H */
