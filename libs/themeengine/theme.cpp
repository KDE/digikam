/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-08-02
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju <renchi@pooh.tam.uiuc.edu>

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
    DDebug() << "Theme :                      " << name << endl;
    
    DDebug() << "Base Color:                  " << baseColor << endl;
    DDebug() << "Text Regular  Color:         " << textRegColor << endl;
    DDebug() << "Text Selected Color:         " << textSelColor << endl;
    DDebug() << "Text Special Regular Color:  " << textSpecialRegColor << endl;
    DDebug() << "Text Special Selected Color: " << textSpecialSelColor << endl;

    DDebug() << "Banner Color:                " << bannerColor << endl;
    DDebug() << "Banner ColorTo :             " << bannerColorTo << endl;
    DDebug() << "Banner Bevel :               " << bannerBevel << endl;
    DDebug() << "Banner Gradient :            " << bannerGrad << endl;
    DDebug() << "Banner Border :              " << bannerBorder << endl;
    DDebug() << "Banner Border Color :        " << bannerBorderColor << endl;

    DDebug() << "ThumbReg Color:              " << thumbRegColor << endl;
    DDebug() << "ThumbReg ColorTo :           " << thumbRegColorTo << endl;
    DDebug() << "ThumbReg Bevel :             " << thumbRegBevel << endl;
    DDebug() << "ThumbReg Gradient :          " << thumbRegGrad << endl;
    DDebug() << "ThumbReg Border :            " << thumbRegBorder << endl;
    DDebug() << "ThumbReg Border Color :      " << thumbRegBorderColor << endl;

    DDebug() << "ThumbSel Color:              " << thumbSelColor << endl;
    DDebug() << "ThumbSel ColorTo :           " << thumbSelColorTo << endl;
    DDebug() << "ThumbSel Bevel :             " << thumbSelBevel << endl;
    DDebug() << "ThumbSel Gradient :          " << thumbSelGrad << endl;
    DDebug() << "ThumbSel Border :            " << thumbSelBorder << endl;
    DDebug() << "ThumbSel Border Color :      " << thumbSelBorderColor << endl;

    DDebug() << "ListReg Color:              " << listRegColor << endl;
    DDebug() << "ListReg ColorTo :           " << listRegColorTo << endl;
    DDebug() << "ListReg Bevel :             " << listRegBevel << endl;
    DDebug() << "ListReg Gradient :          " << listRegGrad << endl;
    DDebug() << "ListReg Border :            " << listRegBorder << endl;
    DDebug() << "ListReg Border Color :      " << listRegBorderColor << endl;

    DDebug() << "ListSel Color:              " << listSelColor << endl;
    DDebug() << "ListSel ColorTo :           " << listSelColorTo << endl;
    DDebug() << "ListSel Bevel :             " << listSelBevel << endl;
    DDebug() << "ListSel Gradient :          " << listSelGrad << endl;
    DDebug() << "ListSel Border :            " << listSelBorder << endl;
    DDebug() << "ListSel Border Color :      " << listSelBorderColor << endl;
    */
}

}  // NameSpace Digikam
