/** ===========================================================
 * 
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2012-03-12
 * @brief  a command line tool to test local Adjustment tool of DImg
 *
 * @author Copyright (C) 2012-2013 by Gilles Caulier
 *         <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a>
 *         Copyright (C) 2012-2013 by Sayantan Datta
 *         <a href="mailto:sayantan dot knz at gmail dot com">sayantan dot knz at gmail dot com</a>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include <QFileInfo>
#include <QString>
#include <QRect>
#include <QImage>
#include <QPoint>

// KDE includes

#include <kdebug.h>

// LibKExiv2 includes

#include <libkexiv2/kexiv2.h>

// Local includes

#include "dimg.h"
#include "drawdecoding.h"
#include "localadjustmenttool.h"
#include "dimgthreadedfilter.h"

using namespace Digikam;
using namespace KExiv2Iface;
using namespace KDcrawIface;

int main(int argc, char** argv)
{
    qDebug() << "ARgc is " << argc;
    if (argc == 6 || argc ==7)
    {
        qDebug() << "Test for local Adjustment in Digikam Begin.";
    }
    else
    {
        qDebug() << "testautocrop - test auto-crop transform";
        qDebug() << "Usage: ";
        qDebug() << "To extract selection : -e <image> <centerX> <centerY> <radius>";
        qDebug() << "To apply selection   : -a <image> <selection> <centerX> <centerY> <radius>";
        return -1;
    }
    
    KExiv2::initializeExiv2();
    
    QFileInfo input(argv[2]);
    QString   outFilePath(input.baseName() + QString(".modified.png"));
    QString   selectionFilePath(input.baseName() + QString(".selection.png"));
    
    RawDecodingSettings settings;
    settings.halfSizeColorImage    = false;
    settings.sixteenBitsImage      = true;
    settings.RGBInterpolate4Colors = false;
    settings.RAWQuality            = RawDecodingSettings::BILINEAR;
    
    DImg img(input.filePath(), 0, DRawDecoding(settings));
    
    
    
//     QRect rect = ac.autoInnerCrop();
    QString choice = argv[1];
    if ( choice == "-e" )
    {
        qDebug() << "Extract selection from image";
        //extract selection from image
        QString temp(argv[3]);
        int x      = temp.toInt();
        temp       = argv[4];
        int y      = temp.toInt();
        temp       = argv[5];
        qDebug() << "ARgv[5] " << argv[5];
        int radius = temp.toInt();
        qDebug() << "Center point : ("<<x<<" , "<<y<<")";
        qDebug() << "Radius       : "<<radius;
        LocalAdjustments la(&img, x, y, radius);
        la.startFilterDirectly();
        DImg selection=la.getSelection();
        selection.save(selectionFilePath,"PNG");
    }
    
    if ( choice == "-a" )
    {
        //attach selection to image
        QString temp(argv[4]);
        int x      = temp.toInt();
        temp       = argv[5];
        int y      = temp.toInt();
        temp       = argv[6];
        int radius = temp.toInt();
        LocalAdjustments la(&img, x, y, radius);
        la.startFilterDirectly();
        QFileInfo maskPath(argv[3]);
        DImg selection(maskPath.filePath(), 0, DRawDecoding(settings));
        DImg output = la.applySelection(&selection);
        output.save(outFilePath, "PNG");
    }
//     kDebug() << "Cropped image area: " << rect;
//     
//     img.crop(rect);
//     img.save(outFilePath, "PNG");
    
    KExiv2::cleanupExiv2();
    
    return 0;
}
