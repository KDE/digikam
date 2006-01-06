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

// KDE includes.

#include <kdebug.h>

// Local includes.

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
        baseColor       =  theme.baseColor;      
        textRegColor    =  theme.textRegColor;   
        textSelColor    =  theme.textSelColor;
        textSpecialRegColor = theme.textSpecialRegColor;
        textSpecialSelColor = theme.textSpecialSelColor;

        bannerColor     =  theme.bannerColor;    
        bannerColorTo   =  theme.bannerColorTo;  
        bannerBevel     =  theme.bannerBevel;    
        bannerGrad      =  theme.bannerGrad;
        bannerBorder    =  theme.bannerBorder;
        bannerBorderColor =  theme.bannerBorderColor;

        thumbRegColor   =  theme.thumbRegColor;  
        thumbRegColorTo =  theme.thumbRegColorTo;
        thumbRegBevel   =  theme.thumbRegBevel;  
        thumbRegGrad    =  theme.thumbRegGrad;   
        thumbRegBorder  =  theme.thumbRegBorder;
        thumbRegBorderColor =  theme.thumbRegBorderColor;

        thumbSelColor   =  theme.thumbSelColor;  
        thumbSelColorTo =  theme.thumbSelColorTo;
        thumbSelBevel   =  theme.thumbSelBevel;  
        thumbSelGrad    =  theme.thumbSelGrad;   
        thumbSelBorder  =  theme.thumbSelBorder;
        thumbSelBorderColor =  theme.thumbSelBorderColor;

        listRegColor    =  theme.listRegColor;   
        listRegColorTo  =  theme.listRegColorTo; 
        listRegBevel    =  theme.listRegBevel;   
        listRegGrad     =  theme.listRegGrad;    
        listRegBorder   =  theme.listRegBorder;
        listRegBorderColor =  theme.listRegBorderColor;

        listSelColor    =  theme.listSelColor;   
        listSelColorTo  =  theme.listSelColorTo; 
        listSelBevel    =  theme.listSelBevel;   
        listSelGrad     =  theme.listSelGrad;    
        listSelBorder   =  theme.listSelBorder;
        listSelBorderColor =  theme.listSelBorderColor;
    }
}

Theme& Theme::operator=(const Theme& theme)
{
    if (this != &theme)
    {
        baseColor       =  theme.baseColor;      
        textRegColor    =  theme.textRegColor;   
        textSelColor    =  theme.textSelColor;   
        textSpecialRegColor = theme.textSpecialRegColor;
        textSpecialSelColor = theme.textSpecialSelColor;

        bannerColor     =  theme.bannerColor;    
        bannerColorTo   =  theme.bannerColorTo;  
        bannerBevel     =  theme.bannerBevel;    
        bannerGrad      =  theme.bannerGrad;
        bannerBorder    =  theme.bannerBorder;
        bannerBorderColor =  theme.bannerBorderColor;

        thumbRegColor   =  theme.thumbRegColor;  
        thumbRegColorTo =  theme.thumbRegColorTo;
        thumbRegBevel   =  theme.thumbRegBevel;  
        thumbRegGrad    =  theme.thumbRegGrad;   
        thumbRegBorder  =  theme.thumbRegBorder;
        thumbRegBorderColor =  theme.thumbRegBorderColor;

        thumbSelColor   =  theme.thumbSelColor;  
        thumbSelColorTo =  theme.thumbSelColorTo;
        thumbSelBevel   =  theme.thumbSelBevel;  
        thumbSelGrad    =  theme.thumbSelGrad;   
        thumbSelBorder  =  theme.thumbSelBorder;
        thumbSelBorderColor =  theme.thumbSelBorderColor;

        listRegColor    =  theme.listRegColor;   
        listRegColorTo  =  theme.listRegColorTo; 
        listRegBevel    =  theme.listRegBevel;   
        listRegGrad     =  theme.listRegGrad;    
        listRegBorder   =  theme.listRegBorder;
        listRegBorderColor =  theme.listRegBorderColor;

        listSelColor    =  theme.listSelColor;   
        listSelColorTo  =  theme.listSelColorTo; 
        listSelBevel    =  theme.listSelBevel;   
        listSelGrad     =  theme.listSelGrad;    
        listSelBorder   =  theme.listSelBorder;
        listSelBorderColor =  theme.listSelBorderColor;
    }
    return *this;
}

void Theme::print()
{
    /*
    kdDebug() << "Theme :                      " << name << endl;
    
    kdDebug() << "Base Color:                  " << baseColor << endl;
    kdDebug() << "Text Regular  Color:         " << textRegColor << endl;
    kdDebug() << "Text Selected Color:         " << textSelColor << endl;
    kdDebug() << "Text Special Regular Color:  " << textSpecialRegColor << endl;
    kdDebug() << "Text Special Selected Color: " << textSpecialSelColor << endl;

    kdDebug() << "Banner Color:                " << bannerColor << endl;
    kdDebug() << "Banner ColorTo :             " << bannerColorTo << endl;
    kdDebug() << "Banner Bevel :               " << bannerBevel << endl;
    kdDebug() << "Banner Gradient :            " << bannerGrad << endl;
    kdDebug() << "Banner Border :              " << bannerBorder << endl;
    kdDebug() << "Banner Border Color :        " << bannerBorderColor << endl;

    kdDebug() << "ThumbReg Color:              " << thumbRegColor << endl;
    kdDebug() << "ThumbReg ColorTo :           " << thumbRegColorTo << endl;
    kdDebug() << "ThumbReg Bevel :             " << thumbRegBevel << endl;
    kdDebug() << "ThumbReg Gradient :          " << thumbRegGrad << endl;
    kdDebug() << "ThumbReg Border :            " << thumbRegBorder << endl;
    kdDebug() << "ThumbReg Border Color :      " << thumbRegBorderColor << endl;

    kdDebug() << "ThumbSel Color:              " << thumbSelColor << endl;
    kdDebug() << "ThumbSel ColorTo :           " << thumbSelColorTo << endl;
    kdDebug() << "ThumbSel Bevel :             " << thumbSelBevel << endl;
    kdDebug() << "ThumbSel Gradient :          " << thumbSelGrad << endl;
    kdDebug() << "ThumbSel Border :            " << thumbSelBorder << endl;
    kdDebug() << "ThumbSel Border Color :      " << thumbSelBorderColor << endl;

    kdDebug() << "ListReg Color:              " << listRegColor << endl;
    kdDebug() << "ListReg ColorTo :           " << listRegColorTo << endl;
    kdDebug() << "ListReg Bevel :             " << listRegBevel << endl;
    kdDebug() << "ListReg Gradient :          " << listRegGrad << endl;
    kdDebug() << "ListReg Border :            " << listRegBorder << endl;
    kdDebug() << "ListReg Border Color :      " << listRegBorderColor << endl;

    kdDebug() << "ListSel Color:              " << listSelColor << endl;
    kdDebug() << "ListSel ColorTo :           " << listSelColorTo << endl;
    kdDebug() << "ListSel Bevel :             " << listSelBevel << endl;
    kdDebug() << "ListSel Gradient :          " << listSelGrad << endl;
    kdDebug() << "ListSel Border :            " << listSelBorder << endl;
    kdDebug() << "ListSel Border Color :      " << listSelBorderColor << endl;
    */
}

}  // NameSpace Digikam
