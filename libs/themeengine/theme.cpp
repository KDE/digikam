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

#include "theme.h"

// KDE includes

#include <kdebug.h>


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
    kDebug(50003) << "Theme :                      " << name;

    kDebug(50003) << "Base Color:                  " << baseColor;
    kDebug(50003) << "Text Regular  Color:         " << textRegColor;
    kDebug(50003) << "Text Selected Color:         " << textSelColor;
    kDebug(50003) << "Text Special Regular Color:  " << textSpecialRegColor;
    kDebug(50003) << "Text Special Selected Color: " << textSpecialSelColor;

    kDebug(50003) << "Banner Color:                " << bannerColor;
    kDebug(50003) << "Banner ColorTo :             " << bannerColorTo;
    kDebug(50003) << "Banner Bevel :               " << bannerBevel;
    kDebug(50003) << "Banner Gradient :            " << bannerGrad;
    kDebug(50003) << "Banner Border :              " << bannerBorder;
    kDebug(50003) << "Banner Border Color :        " << bannerBorderColor;

    kDebug(50003) << "ThumbReg Color:              " << thumbRegColor;
    kDebug(50003) << "ThumbReg ColorTo :           " << thumbRegColorTo;
    kDebug(50003) << "ThumbReg Bevel :             " << thumbRegBevel;
    kDebug(50003) << "ThumbReg Gradient :          " << thumbRegGrad;
    kDebug(50003) << "ThumbReg Border :            " << thumbRegBorder;
    kDebug(50003) << "ThumbReg Border Color :      " << thumbRegBorderColor;

    kDebug(50003) << "ThumbSel Color:              " << thumbSelColor;
    kDebug(50003) << "ThumbSel ColorTo :           " << thumbSelColorTo;
    kDebug(50003) << "ThumbSel Bevel :             " << thumbSelBevel;
    kDebug(50003) << "ThumbSel Gradient :          " << thumbSelGrad;
    kDebug(50003) << "ThumbSel Border :            " << thumbSelBorder;
    kDebug(50003) << "ThumbSel Border Color :      " << thumbSelBorderColor;

    kDebug(50003) << "ListReg Color:              " << listRegColor;
    kDebug(50003) << "ListReg ColorTo :           " << listRegColorTo;
    kDebug(50003) << "ListReg Bevel :             " << listRegBevel;
    kDebug(50003) << "ListReg Gradient :          " << listRegGrad;
    kDebug(50003) << "ListReg Border :            " << listRegBorder;
    kDebug(50003) << "ListReg Border Color :      " << listRegBorderColor;

    kDebug(50003) << "ListSel Color:              " << listSelColor;
    kDebug(50003) << "ListSel ColorTo :           " << listSelColorTo;
    kDebug(50003) << "ListSel Bevel :             " << listSelBevel;
    kDebug(50003) << "ListSel Gradient :          " << listSelGrad;
    kDebug(50003) << "ListSel Border :            " << listSelBorder;
    kDebug(50003) << "ListSel Border Color :      " << listSelBorderColor;
    */
}

}  // namespace Digikam
