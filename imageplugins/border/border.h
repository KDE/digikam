/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-25
 * Description : border threaded image filter.
 * 
 * Copyright 2005-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright 2006-2007 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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
  
#ifndef BORDER_H
#define BORDER_H

// Qt includes.

#include <qstring.h>
#include <qcolor.h>
#include <qimage.h>

// Digikam includes.

#include "dimgthreadedfilter.h"

namespace DigikamBorderImagesPlugin
{

class Border : public Digikam::DImgThreadedFilter
{

public:

    enum BorderTypes 
    {
        SolidBorder=0,
        NiepceBorder,
        BeveledBorder,
        PineBorder,
        WoodBorder,
        PaperBorder,
        ParqueBorder,
        IceBorder,
        LeafBorder,
        MarbleBorder,
        RainBorder,
        CratersBorder,
        DriedBorder,
        PinkBorder,
        StoneBorder,
        ChalkBorder,
        GraniteBorder,
        RockBorder,
        WallBorder
    };

public:

    /** Constructor using settings to preserve aspect ratio of image. */
    Border(Digikam::DImg *orgImage, QObject *parent=0, int orgWidth=0, int orgHeight=0,
           QString borderPath=QString(), int borderType=SolidBorder, float borderPercent=0.1,
           Digikam::DColor solidColor = Digikam::DColor(),
           Digikam::DColor niepceBorderColor = Digikam::DColor(),
           Digikam::DColor niepceLineColor = Digikam::DColor(),
           Digikam::DColor bevelUpperLeftColor = Digikam::DColor(),
           Digikam::DColor bevelLowerRightColor = Digikam::DColor(),
           Digikam::DColor decorativeFirstColor = Digikam::DColor(),
           Digikam::DColor decorativeSecondColor = Digikam::DColor());

    /** Constructor using settings to not-preserve aspect ratio of image. */
    Border(Digikam::DImg *orgImage, QObject *parent=0, int orgWidth=0, int orgHeight=0,
           QString borderPath=QString(), int borderType=SolidBorder,
           int borderWidth1=100, int borderWidth2=20, int borderWidth3=20, int borderWidth4=10,
           Digikam::DColor solidColor = Digikam::DColor(),
           Digikam::DColor niepceBorderColor = Digikam::DColor(),
           Digikam::DColor niepceLineColor = Digikam::DColor(),
           Digikam::DColor bevelUpperLeftColor = Digikam::DColor(),
           Digikam::DColor bevelLowerRightColor = Digikam::DColor(),
           Digikam::DColor decorativeFirstColor = Digikam::DColor(),
           Digikam::DColor decorativeSecondColor = Digikam::DColor());

    ~Border(){};

private:  

    virtual void filterImage(void);
    
    
    /** Methods to preserve aspect ratio of image. */
    void solid(Digikam::DImg &src, Digikam::DImg &dest, const Digikam::DColor &fg, int borderWidth);
    void niepce(Digikam::DImg &src, Digikam::DImg &dest, const Digikam::DColor &fg, int borderWidth, 
                const Digikam::DColor &bg, int lineWidth);
    void bevel(Digikam::DImg &src, Digikam::DImg &dest, const Digikam::DColor &topColor, 
               const Digikam::DColor &btmColor, int borderWidth);
    void pattern(Digikam::DImg &src, Digikam::DImg &dest, int borderWidth, const Digikam::DColor &firstColor, 
                 const Digikam::DColor &secondColor, int firstWidth, int secondWidth);

    /** Methods to not-preserve aspect ratio of image. */
    void solid2(Digikam::DImg &src, Digikam::DImg &dest, const Digikam::DColor &fg, int borderWidth);
    void niepce2(Digikam::DImg &src, Digikam::DImg &dest, const Digikam::DColor &fg, int borderWidth, 
                 const Digikam::DColor &bg, int lineWidth);
    void bevel2(Digikam::DImg &src, Digikam::DImg &dest, const Digikam::DColor &topColor, 
                const Digikam::DColor &btmColor, int borderWidth);
    void pattern2(Digikam::DImg &src, Digikam::DImg &dest, int borderWidth, const Digikam::DColor &firstColor, 
                  const Digikam::DColor &secondColor, int firstWidth, int secondWidth);

private:  

    bool            m_preserveAspectRatio;

    int             m_orgWidth;
    int             m_orgHeight;

    int             m_borderType;
    
    int             m_borderWidth1;
    int             m_borderWidth2;
    int             m_borderWidth3;
    int             m_borderWidth4;

    int             m_borderMainWidth;
    int             m_border2ndWidth;

    float           m_orgRatio;

    QString         m_borderPath;
    
    Digikam::DColor m_solidColor;
    Digikam::DColor m_niepceBorderColor;
    Digikam::DColor m_niepceLineColor;
    Digikam::DColor m_bevelUpperLeftColor; 
    Digikam::DColor m_bevelLowerRightColor;
    Digikam::DColor m_decorativeFirstColor; 
    Digikam::DColor m_decorativeSecondColor;
};    

}  // NameSpace DigikamBorderImagesPlugin

#endif /* BORDER_H */
