/* ============================================================
 * File  : border.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-05-25
 * Description : border threaded image filter.
 * 
 * Copyright 2005 by Gilles Caulier
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

class Border : public Digikam::ThreadedFilter
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
    
    Border(QImage *orgImage, QObject *parent=0, int orgWidth=0, int orgHeight=0, 
           QString borderPath=QString::null, int borderType=SolidBorder, 
           int borderWidth1=100, int borderWidth2=20, int borderWidth3=20, int borderWidth4=10, 
           QColor solidColor=QColor::QColor( 0, 0, 0 ), 
           QColor niepceBorderColor=QColor::QColor( 0, 0, 0 ),
           QColor niepceLineColor=QColor::QColor( 0, 0, 0 ), 
           QColor bevelUpperLeftColor=QColor::QColor( 0, 0, 0 ),
           QColor bevelLowerRightColor=QColor::QColor( 0, 0, 0 ), 
           QColor decorativeFirstColor=QColor::QColor( 0, 0, 0 ),
           QColor decorativeSecondColor=QColor::QColor( 0, 0, 0 ));
    
    ~Border(){};
        
private:  // Border filter data.

    int    m_orgWidth;
    int    m_orgHeight;
    
    int    m_borderType;
    int    m_borderWidth1;
    int    m_borderWidth2;
    int    m_borderWidth3;
    int    m_borderWidth4;
    
    QColor m_solidColor;
    QColor m_niepceBorderColor;
    QColor m_niepceLineColor;
    QColor m_bevelUpperLeftColor; 
    QColor m_bevelLowerRightColor;
    QColor m_decorativeFirstColor; 
    QColor m_decorativeSecondColor;
    
    QString m_borderPath;
        
private:  // Border filter methods.

    virtual void filterImage(void);
    
    void solid(QImage &src, QImage &dest, const QColor &fg, int borderWidth);
    void niepce(QImage &src, QImage &dest, const QColor &fg, int borderWidth, const QColor &bg, int lineWidth);
    void bevel(QImage &src, QImage &dest, const QColor &topColor, const QColor &btmColor, int borderWidth);
    void pattern(QImage &src, QImage &dest, int borderWidth, const QColor &firstColor, 
                 const QColor &secondColor, int firstWidth, int secondWidth);
};    

}  // NameSpace DigikamBorderImagesPlugin

#endif /* BORDER_H */
