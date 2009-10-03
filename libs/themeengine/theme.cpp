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
    kDebug(digiKamAreaCode) << "Theme :                      " << name;

    kDebug(digiKamAreaCode) << "Base Color:                  " << baseColor;
    kDebug(digiKamAreaCode) << "Text Regular  Color:         " << textRegColor;
    kDebug(digiKamAreaCode) << "Text Selected Color:         " << textSelColor;
    kDebug(digiKamAreaCode) << "Text Special Regular Color:  " << textSpecialRegColor;
    kDebug(digiKamAreaCode) << "Text Special Selected Color: " << textSpecialSelColor;

    kDebug(digiKamAreaCode) << "Banner Color:                " << bannerColor;
    kDebug(digiKamAreaCode) << "Banner ColorTo :             " << bannerColorTo;
    kDebug(digiKamAreaCode) << "Banner Bevel :               " << bannerBevel;
    kDebug(digiKamAreaCode) << "Banner Gradient :            " << bannerGrad;
    kDebug(digiKamAreaCode) << "Banner Border :              " << bannerBorder;
    kDebug(digiKamAreaCode) << "Banner Border Color :        " << bannerBorderColor;

    kDebug(digiKamAreaCode) << "ThumbReg Color:              " << thumbRegColor;
    kDebug(digiKamAreaCode) << "ThumbReg ColorTo :           " << thumbRegColorTo;
    kDebug(digiKamAreaCode) << "ThumbReg Bevel :             " << thumbRegBevel;
    kDebug(digiKamAreaCode) << "ThumbReg Gradient :          " << thumbRegGrad;
    kDebug(digiKamAreaCode) << "ThumbReg Border :            " << thumbRegBorder;
    kDebug(digiKamAreaCode) << "ThumbReg Border Color :      " << thumbRegBorderColor;

    kDebug(digiKamAreaCode) << "ThumbSel Color:              " << thumbSelColor;
    kDebug(digiKamAreaCode) << "ThumbSel ColorTo :           " << thumbSelColorTo;
    kDebug(digiKamAreaCode) << "ThumbSel Bevel :             " << thumbSelBevel;
    kDebug(digiKamAreaCode) << "ThumbSel Gradient :          " << thumbSelGrad;
    kDebug(digiKamAreaCode) << "ThumbSel Border :            " << thumbSelBorder;
    kDebug(digiKamAreaCode) << "ThumbSel Border Color :      " << thumbSelBorderColor;

    kDebug(digiKamAreaCode) << "ListReg Color:              " << listRegColor;
    kDebug(digiKamAreaCode) << "ListReg ColorTo :           " << listRegColorTo;
    kDebug(digiKamAreaCode) << "ListReg Bevel :             " << listRegBevel;
    kDebug(digiKamAreaCode) << "ListReg Gradient :          " << listRegGrad;
    kDebug(digiKamAreaCode) << "ListReg Border :            " << listRegBorder;
    kDebug(digiKamAreaCode) << "ListReg Border Color :      " << listRegBorderColor;

    kDebug(digiKamAreaCode) << "ListSel Color:              " << listSelColor;
    kDebug(digiKamAreaCode) << "ListSel ColorTo :           " << listSelColorTo;
    kDebug(digiKamAreaCode) << "ListSel Bevel :             " << listSelBevel;
    kDebug(digiKamAreaCode) << "ListSel Gradient :          " << listSelGrad;
    kDebug(digiKamAreaCode) << "ListSel Border :            " << listSelBorder;
    kDebug(digiKamAreaCode) << "ListSel Border Color :      " << listSelBorderColor;
    */
}

}  // namespace Digikam
