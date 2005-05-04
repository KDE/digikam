/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-08-02
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju

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

#ifndef THEME_H
#define THEME_H

#include <qstring.h>
#include <qcolor.h>

class Theme
{
public:

    enum Bevel {
        FLAT =     0x00001,
        SUNKEN =   0x00002,
        RAISED =   0x00004
    };
    
    enum Gradient {
        SOLID      = 0x00000,
        HORIZONTAL = 0x00010,
        VERTICAL   = 0x00020,
        DIAGONAL   = 0x00040
    };

    Theme(const QString& _name, const QString& _path);
    Theme(const Theme& theme);
    Theme& operator=(const Theme& theme);

    void print();
    
    QString  name;
    QString  filePath;

    QColor   baseColor;
    QColor   textRegColor;
    QColor   textSelColor;
    QColor   textSpecialRegColor;
    QColor   textSpecialSelColor;
    
    QColor   bannerColor;
    QColor   bannerColorTo;
    Bevel    bannerBevel;
    Gradient bannerGrad;
    bool     bannerBorder;
    QColor   bannerBorderColor;

    QColor   thumbRegColor;
    QColor   thumbRegColorTo;
    Bevel    thumbRegBevel;
    Gradient thumbRegGrad;
    bool     thumbRegBorder;
    QColor   thumbRegBorderColor;

    QColor   thumbSelColor;
    QColor   thumbSelColorTo;
    Bevel    thumbSelBevel;
    Gradient thumbSelGrad;
    bool     thumbSelBorder;
    QColor   thumbSelBorderColor;
             
    QColor   listRegColor;
    QColor   listRegColorTo;
    Bevel    listRegBevel;
    Gradient listRegGrad;
    bool     listRegBorder;
    QColor   listRegBorderColor;
             
    QColor   listSelColor;
    QColor   listSelColorTo;
    Bevel    listSelBevel;
    Gradient listSelGrad;
    bool     listSelBorder;
    QColor   listSelBorderColor;
};

#endif /* THEME_H */
