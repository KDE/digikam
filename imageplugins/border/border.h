/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
           Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Date  : 2005-05-25
 * Description : border threaded image filter.
 * 
 * Copyright 2005 by Gilles Caulier
 * Copyright 2006 by Gilles Caulier and Marcel Wiesweg
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

#include <digikamheaders.h>

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

    Border(Digikam::DImg *orgImage, QObject *parent=0, int orgWidth=0, int orgHeight=0,
           QString borderPath=QString::null, int borderType=SolidBorder, float borderRatio=0.1,
           Digikam::DColor solidColor = Digikam::DColor(),
           Digikam::DColor niepceBorderColor = Digikam::DColor(),
           Digikam::DColor niepceLineColor = Digikam::DColor(),
           Digikam::DColor bevelUpperLeftColor = Digikam::DColor(),
           Digikam::DColor bevelLowerRightColor = Digikam::DColor(),
           Digikam::DColor decorativeFirstColor = Digikam::DColor(),
           Digikam::DColor decorativeSecondColor = Digikam::DColor());

    ~Border(){};

private:  

    int     m_orgWidth;
    int     m_orgHeight;
    
    int     m_borderType;
    int     m_borderMainWidth;
    int     m_border2ndWidth;

    float   m_orgRatio;

    QString m_borderPath;
    
    Digikam::DColor m_solidColor;
    Digikam::DColor m_niepceBorderColor;
    Digikam::DColor m_niepceLineColor;
    Digikam::DColor m_bevelUpperLeftColor; 
    Digikam::DColor m_bevelLowerRightColor;
    Digikam::DColor m_decorativeFirstColor; 
    Digikam::DColor m_decorativeSecondColor;
    
private:  

    virtual void filterImage(void);
    
    void solid(Digikam::DImg &src, Digikam::DImg &dest, const Digikam::DColor &fg, int borderWidth);
    void niepce(Digikam::DImg &src, Digikam::DImg &dest, const Digikam::DColor &fg, int borderWidth, 
                const Digikam::DColor &bg, int lineWidth);
    void bevel(Digikam::DImg &src, Digikam::DImg &dest, const Digikam::DColor &topColor, 
               const Digikam::DColor &btmColor, int borderWidth);
    void pattern(Digikam::DImg &src, Digikam::DImg &dest, int borderWidth, const Digikam::DColor &firstColor, 
                 const Digikam::DColor &secondColor, int firstWidth, int secondWidth);
};    

}  // NameSpace DigikamBorderImagesPlugin

#endif /* BORDER_H */
