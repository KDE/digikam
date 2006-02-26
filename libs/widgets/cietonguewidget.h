/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2006-01-10
 * Description : a widget to display CIE tongue from
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

#ifndef CIETONGUEWIDGET_H
#define CIETONGUEWIDGET_H

#include <config.h>

// Qt includes.

#include <qwidget.h>
#include <qcolor.h>

// KDE includes.

#include <kurl.h>

// lcms includes

#include LCMS_HEADER
#if LCMS_VERSION < 114
#define cmsTakeCopyright(profile) "Unknown"
#endif // LCMS_VERSION < 114

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class CIETongueWidgetPriv;

class DIGIKAM_EXPORT CIETongueWidget : public QWidget
{
Q_OBJECT

public:

    CIETongueWidget(int w, int h, QWidget *parent=0, cmsHPROFILE hMonitor=0);
    ~CIETongueWidget();

    bool setProfileData(QByteArray *profileData=0);
    bool setProfileFromFile(const KURL& file=KURL());
    bool setProfileHandler(cmsHPROFILE hProfile=0);
    
protected:

    int Grids(double val) const;

    void OutlineTongue();
    void FillTongue();
    void DrawTongueAxis();
    void DrawTongueGrid();
    void DrawLabels();

    QRgb ColorByCoord(double x, double y);  
    void DrawSmallElipse(LPcmsCIExyY xyY, BYTE r, BYTE g, BYTE b, int sz);

    void paintEvent( QPaintEvent * );

private:

    void DrawColorantTriangle(void);
    void DrawWhitePoint(void);
    void DrawPatches(void);
    
    void MapPoint(int& icx, int& icy, LPcmsCIExyY xyY);
    void BiasedLine(int x1, int y1, int x2, int y2);
    void BiasedText(int x, int y, QString Txt);

    void Sweep_sRGB(void);

    void setProfile(cmsHPROFILE hProfile);

private :

    CIETongueWidgetPriv* d;

};

}  // namespace Digikam

#endif /* CIETONGUEWIDGET_H */
