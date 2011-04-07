/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-08-02
 * Description : theme manager
 *
 * Copyright (C) 2004 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

namespace Digikam
{

Theme::Theme()
{
}

Theme::Theme(const Theme& theme)
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
        kDebug() << "Base Color:                  " << baseColor;
        kDebug() << "Text Regular  Color:         " << textRegColor;
        kDebug() << "Text Selected Color:         " << textSelColor;
        kDebug() << "Text Special Regular Color:  " << textSpecialRegColor;
        kDebug() << "Text Special Selected Color: " << textSpecialSelColor;

        kDebug() << "Banner Color:                " << bannerColor;
        kDebug() << "Banner ColorTo :             " << bannerColorTo;
        kDebug() << "Banner Bevel :               " << bannerBevel;
        kDebug() << "Banner Gradient :            " << bannerGrad;
        kDebug() << "Banner Border :              " << bannerBorder;
        kDebug() << "Banner Border Color :        " << bannerBorderColor;

        kDebug() << "ThumbReg Color:              " << thumbRegColor;
        kDebug() << "ThumbReg ColorTo :           " << thumbRegColorTo;
        kDebug() << "ThumbReg Bevel :             " << thumbRegBevel;
        kDebug() << "ThumbReg Gradient :          " << thumbRegGrad;
        kDebug() << "ThumbReg Border :            " << thumbRegBorder;
        kDebug() << "ThumbReg Border Color :      " << thumbRegBorderColor;

        kDebug() << "ThumbSel Color:              " << thumbSelColor;
        kDebug() << "ThumbSel ColorTo :           " << thumbSelColorTo;
        kDebug() << "ThumbSel Bevel :             " << thumbSelBevel;
        kDebug() << "ThumbSel Gradient :          " << thumbSelGrad;
        kDebug() << "ThumbSel Border :            " << thumbSelBorder;
        kDebug() << "ThumbSel Border Color :      " << thumbSelBorderColor;

        kDebug() << "ListReg Color:              " << listRegColor;
        kDebug() << "ListReg ColorTo :           " << listRegColorTo;
        kDebug() << "ListReg Bevel :             " << listRegBevel;
        kDebug() << "ListReg Gradient :          " << listRegGrad;
        kDebug() << "ListReg Border :            " << listRegBorder;
        kDebug() << "ListReg Border Color :      " << listRegBorderColor;

        kDebug() << "ListSel Color:              " << listSelColor;
        kDebug() << "ListSel ColorTo :           " << listSelColorTo;
        kDebug() << "ListSel Bevel :             " << listSelBevel;
        kDebug() << "ListSel Gradient :          " << listSelGrad;
        kDebug() << "ListSel Border :            " << listSelBorder;
        kDebug() << "ListSel Border Color :      " << listSelBorderColor;
    */
}

}  // namespace Digikam
