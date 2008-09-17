/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-08-02
 * Description : theme manager
 * 
 * Copyright (C) 2004 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Local includes.

#include "ddebug.h"
#include "theme.h"

namespace Digikam
{

Theme::Theme(const QString& _name, const QString& _path)
{
    name     = _name;
    filePath = _path;
}

Theme::Theme(const Theme& theme)
{
    if (this != &theme)
    {
        baseColor           =  theme.baseColor;
        textRegColor        =  theme.textRegColor;
        textSelColor        =  theme.textSelColor;
        textSpecialRegColor = theme.textSpecialRegColor;
        textSpecialSelColor = theme.textSpecialSelColor;

        bannerColor         =  theme.bannerColor;
        bannerColorTo       =  theme.bannerColorTo;
        bannerBevel         =  theme.bannerBevel;
        bannerGrad          =  theme.bannerGrad;
        bannerBorder        =  theme.bannerBorder;
        bannerBorderColor   =  theme.bannerBorderColor;

        thumbRegColor       =  theme.thumbRegColor;
        thumbRegColorTo     =  theme.thumbRegColorTo;
        thumbRegBevel       =  theme.thumbRegBevel;
        thumbRegGrad        =  theme.thumbRegGrad; 
        thumbRegBorder      =  theme.thumbRegBorder;
        thumbRegBorderColor =  theme.thumbRegBorderColor;

        thumbSelColor       =  theme.thumbSelColor;
        thumbSelColorTo     =  theme.thumbSelColorTo;
        thumbSelBevel       =  theme.thumbSelBevel;
        thumbSelGrad        =  theme.thumbSelGrad; 
        thumbSelBorder      =  theme.thumbSelBorder;
        thumbSelBorderColor =  theme.thumbSelBorderColor;

        listRegColor        =  theme.listRegColor;
        listRegColorTo      =  theme.listRegColorTo;
        listRegBevel        =  theme.listRegBevel;
        listRegGrad         =  theme.listRegGrad;
        listRegBorder       =  theme.listRegBorder;
        listRegBorderColor  =  theme.listRegBorderColor;

        listSelColor        =  theme.listSelColor;
        listSelColorTo      =  theme.listSelColorTo;
        listSelBevel        =  theme.listSelBevel;
        listSelGrad         =  theme.listSelGrad;
        listSelBorder       =  theme.listSelBorder;
        listSelBorderColor  =  theme.listSelBorderColor;
    }
}

Theme& Theme::operator=(const Theme& theme)
{
    if (this != &theme)
    {
        baseColor           = theme.baseColor;
        textRegColor        = theme.textRegColor;
        textSelColor        = theme.textSelColor;
        textSpecialRegColor = theme.textSpecialRegColor;
        textSpecialSelColor = theme.textSpecialSelColor;

        bannerColor         = theme.bannerColor;
        bannerColorTo       = theme.bannerColorTo;
        bannerBevel         = theme.bannerBevel;
        bannerGrad          = theme.bannerGrad;
        bannerBorder        = theme.bannerBorder;
        bannerBorderColor   = theme.bannerBorderColor;

        thumbRegColor       = theme.thumbRegColor;
        thumbRegColorTo     = theme.thumbRegColorTo;
        thumbRegBevel       = theme.thumbRegBevel;
        thumbRegGrad        = theme.thumbRegGrad;
        thumbRegBorder      = theme.thumbRegBorder;
        thumbRegBorderColor = theme.thumbRegBorderColor;

        thumbSelColor       = theme.thumbSelColor;
        thumbSelColorTo     = theme.thumbSelColorTo;
        thumbSelBevel       = theme.thumbSelBevel;
        thumbSelGrad        = theme.thumbSelGrad;
        thumbSelBorder      = theme.thumbSelBorder;
        thumbSelBorderColor = theme.thumbSelBorderColor;

        listRegColor        = theme.listRegColor;
        listRegColorTo      = theme.listRegColorTo;
        listRegBevel        = theme.listRegBevel;
        listRegGrad         = theme.listRegGrad;
        listRegBorder       = theme.listRegBorder;
        listRegBorderColor  = theme.listRegBorderColor;

        listSelColor        = theme.listSelColor;
        listSelColorTo      = theme.listSelColorTo;
        listSelBevel        = theme.listSelBevel;
        listSelGrad         = theme.listSelGrad;
        listSelBorder       = theme.listSelBorder;
        listSelBorderColor  = theme.listSelBorderColor;
    }
    return *this;
}

void Theme::print()
{
    /*
    DDebug(50003) << "Theme :                      " << name << endl;

    DDebug(50003) << "Base Color:                  " << baseColor << endl;
    DDebug(50003) << "Text Regular  Color:         " << textRegColor << endl;
    DDebug(50003) << "Text Selected Color:         " << textSelColor << endl;
    DDebug(50003) << "Text Special Regular Color:  " << textSpecialRegColor << endl;
    DDebug(50003) << "Text Special Selected Color: " << textSpecialSelColor << endl;

    DDebug(50003) << "Banner Color:                " << bannerColor << endl;
    DDebug(50003) << "Banner ColorTo :             " << bannerColorTo << endl;
    DDebug(50003) << "Banner Bevel :               " << bannerBevel << endl;
    DDebug(50003) << "Banner Gradient :            " << bannerGrad << endl;
    DDebug(50003) << "Banner Border :              " << bannerBorder << endl;
    DDebug(50003) << "Banner Border Color :        " << bannerBorderColor << endl;

    DDebug(50003) << "ThumbReg Color:              " << thumbRegColor << endl;
    DDebug(50003) << "ThumbReg ColorTo :           " << thumbRegColorTo << endl;
    DDebug(50003) << "ThumbReg Bevel :             " << thumbRegBevel << endl;
    DDebug(50003) << "ThumbReg Gradient :          " << thumbRegGrad << endl;
    DDebug(50003) << "ThumbReg Border :            " << thumbRegBorder << endl;
    DDebug(50003) << "ThumbReg Border Color :      " << thumbRegBorderColor << endl;

    DDebug(50003) << "ThumbSel Color:              " << thumbSelColor << endl;
    DDebug(50003) << "ThumbSel ColorTo :           " << thumbSelColorTo << endl;
    DDebug(50003) << "ThumbSel Bevel :             " << thumbSelBevel << endl;
    DDebug(50003) << "ThumbSel Gradient :          " << thumbSelGrad << endl;
    DDebug(50003) << "ThumbSel Border :            " << thumbSelBorder << endl;
    DDebug(50003) << "ThumbSel Border Color :      " << thumbSelBorderColor << endl;

    DDebug(50003) << "ListReg Color:              " << listRegColor << endl;
    DDebug(50003) << "ListReg ColorTo :           " << listRegColorTo << endl;
    DDebug(50003) << "ListReg Bevel :             " << listRegBevel << endl;
    DDebug(50003) << "ListReg Gradient :          " << listRegGrad << endl;
    DDebug(50003) << "ListReg Border :            " << listRegBorder << endl;
    DDebug(50003) << "ListReg Border Color :      " << listRegBorderColor << endl;

    DDebug(50003) << "ListSel Color:              " << listSelColor << endl;
    DDebug(50003) << "ListSel ColorTo :           " << listSelColorTo << endl;
    DDebug(50003) << "ListSel Bevel :             " << listSelBevel << endl;
    DDebug(50003) << "ListSel Gradient :          " << listSelGrad << endl;
    DDebug(50003) << "ListSel Border :            " << listSelBorder << endl;
    DDebug(50003) << "ListSel Border Color :      " << listSelBorderColor << endl;
    */
}

}  // NameSpace Digikam
