/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-10-18
 * Description : Auto Crop analyser
 * 
 * Algorithm based on "finding the largest axis aligned rectangle in a polygon
 * in o(n log n) time" method from Ralph P. Boland and Jorge Urrutia.
 * http://www.cccg.ca/proceedings/2001/rboland-98103.ps.gz. 
 *
 * Copyright (C) 2013 by Sayantan Datta <sayantan dot knz at gmail dot com>
 * Copyright (C) 2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// C++ includes

#include <cmath>
#include <cfloat>

// Qt includes.

#include <QTextStream>
#include <QFile>
#include <QPoint>
#include <QRect>
#include <QImage>
#include <QColor>

// KDE includes

#include <kdebug.h>

// Local includes

#include "autocrop.h"

namespace Digikam
{

class AutoCrop::Private
{
public:

    Private()
    {
    }

    QRect  cropArea;
};

AutoCrop::AutoCrop(DImg* const img, QObject* const parent)
    : DImgThreadedAnalyser(parent, "AutoCrop"), d(new Private)
{
    setOriginalImage(*img);
}

AutoCrop::~AutoCrop()
{
    delete d;
}

void AutoCrop::startAnalyse()
{
    QImage img       = m_orgImage.copyQImage();
    int breakflag    = 0;
    int topRow       = -1;
    int topColumn    = -1;
    int bottomRow    = -1;
    int bottomColumn = -1;
    int leftRow      = -1;
    int leftColumn   = -1;
    int rightRow     = -1;
    int rightColumn  = -1;
    QColor c;
    int i,j;

    postProgress(5);

    //----------------------Finding the outer boundaries of the image (i.e. with black portions)

    /* This would be done in 4 steps
        1. Search column wise:
            (a) From the left to the right, this is to get the left boundary
            (b) From the right to the left, this is to get the right boundary
        2. Search row wise :
            (a) From the top to the bottom, this is to get the top boundary
            (b) From the bottom to the top, this is to get the bottom boundary
    */

    //1(a) Traversing the image from top to bottom, left to right, to get left boundary

    breakflag  = 0;
    int width  = img.width();
    int height = img.height();

    for(i=0; i < width; i++)
    {
        for(j=0; j < height; j++)
        {
            c = QColor::fromRgb(img.pixel(i, j));

            if( c == Qt::black || !c.isValid())
            {
                // Nothing to do.
            }
            else
            {
                //we have found our pixel
                leftRow    = j;
                leftColumn = i;
                breakflag  = 1;
                break;
            }
        }

        if(breakflag == 1)
            break;
    }

    kDebug() << "Done Till step 1(a)";
    postProgress(30);

    //1(b) Traversing the image from top to bottom, right to left, to get right boundary

    breakflag = 0;

    for(i=0; i < width; i++)
    {
        for(j=0; j < height; j++)
        {
            c = QColor::fromRgb(img.pixel(i, j));

            if(c == Qt::black || !c.isValid())
            {
                // Nothing to do.
            }
            else
            {
                //we have found our pixel
                rightRow    = j;
                rightColumn = i;
                break;
            }
        }
    }

    kDebug() << "Done Till step 1(b)";
    postProgress(50);

    //2(a) Traversing the image left to right, top to down, to get top boundary

    breakflag = 0;

    for(i=0; i < height; i++)
    {
        for(j=0; j < width; j++)
        {
            c = QColor::fromRgb(img.pixel(j, i));

            if(c == Qt::black || !c.isValid())
            {
                // Nothing to do.
            }
            else
            {
                //we have found our pixel
                topRow    = i;
                topColumn = j;
                breakflag = 1;
                break;
            }
        }

        if(breakflag == 1)
            break;
    }

    kDebug() << "Done Till step 2(a)";
    postProgress(70);

    //2(b) Traversing the image from left to right, bottom up, to get lower boundary

    breakflag = 0;

    for(i=height-1; i >= 0; i--)
    {
        for(j=0; j < width; j++)
        {
            c = QColor::fromRgb(img.pixel(j,i));

            if(c == Qt::black || !c.isValid())
            {
                // Nothing to do.
            }
            else
            {
                //we have found our pixel
                bottomRow    = i;
                bottomColumn = j;
                breakflag    = 1;
                break;
            }
        }

        if(breakflag==1)
            break;
    }

    kDebug() << "Done Till step 2(b)";
    postProgress(90);

    //------making the required output--------------------

    QString outercropParameters;
    outercropParameters.append("TopMost Pixel : ( ");
    outercropParameters.append(QString::number(topRow));
    outercropParameters.append(", ");
    outercropParameters.append(QString::number(topColumn));
    outercropParameters.append(")\nBottomMost Pixel : ( ");
    outercropParameters.append(QString::number(bottomRow));
    outercropParameters.append(", ");
    outercropParameters.append(QString::number(bottomColumn));
    outercropParameters.append(")\nLeftMost Pixel : ( ");
    outercropParameters.append(QString::number(leftRow));
    outercropParameters.append(", ");
    outercropParameters.append(QString::number(leftColumn));
    outercropParameters.append(")\nRightMost Pixel : ( ");
    outercropParameters.append(QString::number(rightRow));
    outercropParameters.append(", ");
    outercropParameters.append(QString::number(rightColumn));
    outercropParameters.append(")\nDONE");
    kDebug() << outercropParameters;
    postProgress(91);

    QPoint p1;
    p1.setX(leftColumn);
    p1.setY(topRow);
    QPoint p2;
    p2.setX(rightColumn);
    p2.setY(bottomRow);
    QRect crop;
    crop.setTopLeft(p1);
    crop.setBottomRight(p2);

    // crop Image to outerCrop

    QImage result;
    QSize resultsize = QSize(crop.width(), crop.height());
    result           = QImage(resultsize, QImage::Format_RGB888);
    int ni, nj;

    kDebug() << "Outer Crop area:";
    kDebug() << "From "<< crop.top()  << " to " << crop.bottom() 
             << " & "  << crop.left() << " to " << crop.right();

    for(i=crop.top(), ni=0; i <= crop.bottom(); i++, ni++)
    {
        for(j=crop.left(), nj=0; j <= crop.right(); j++, nj++)
        {
            result.setPixel(nj, ni, img.pixel(j, i));
        }
    }

    //---------------------threshold the image

    QImage threshold = QImage(resultsize,QImage::Format_RGB888);
    int toggleflag1  = 0;
    int toggleflag2  = 0;

    //-----initialize

    for(i=0; i < result.height(); i++)
    {
        for(j=0; j < result.width(); j++)
        {
            threshold.setPixel(j, i, Qt::black);
        }
    }

    //----------mark points on horizontal scan

    for(i=0; i < result.height(); i++)
    {
        toggleflag1 = -1;
        toggleflag2 =- 1;

        for(j=0; j < result.width(); j++)
        {
            c = QColor::fromRgb(result.pixel(j, i));

            if(c != Qt::black)
            {
                toggleflag1 = j;
                break;
            }
        }

        for(j=(result.width()-1); j >= 0; j--)
        {
            c = QColor::fromRgb(result.pixel(j, i));

            if(c != Qt::black)
            {
                toggleflag2 = j;
                break;
            }
        }

        if(toggleflag1 >= 0)
        {
            for(j = toggleflag1; j <= toggleflag2; j++)
            {
                threshold.setPixel(j, i, qRgb(255, 255, 255));
            }
        }
    }

    //----------fill black points on vertical scan

    for(j=0; j < result.width(); j++)
    {
        toggleflag1 = -1;
        toggleflag2 = -2;

        for(i=0; i < result.height(); i++)
        {
            c = QColor::fromRgb(result.pixel(j, i));

            if(c != Qt::black)
            {
                toggleflag1 = i;
                break;
            }
        }

        for(i = (result.height()-1); i >= 0; i--)
        {
            c = QColor::fromRgb(result.pixel(j, i));

            if(c != Qt::black)
            {
                toggleflag2 = i;
                break;
            }
        }

        if(toggleflag1 >= 0)
        {
            for(i=0; i < toggleflag1; i++)
            {
                threshold.setPixel(j, i, qRgb(0, 0, 0));
            }
        }

        if(toggleflag2 >= 0)
        {
            for(i=(toggleflag2+1);i<(result.height());i++)
            {
                threshold.setPixel(j,i,qRgb(0,0,0));
            }
        }
    }
    
    kDebug() << "Thresholding Complete\n";

    //---------------------inner crop

    int limitcolumn      = threshold.width();
    int limitrow         = threshold.height();
    int centeri          = ((limitrow/2)-1);
    int centerj          = ((limitcolumn/2)-1);
    int startrighti      = 0;
    int startrightj      = 0;
    int endrighti        = 0;
    int endrightj        = 0;
    int startlefti       = 0;
    int startleftj       = 0;
    int endlefti         = 0;
    int endleftj         = 0;
    int startupi         = 0;
    int startupj         = 0;
    int endupi           = 0;
    int endupj           = 0;
    int startdowni       = 0;
    int startdownj       = 0;
    int enddowni         = 0;
    int enddownj         = 0;
    int travelright      = 0;
    int travelleft       = 0;
    int travelup         = 0;
    int traveldown       = 0;
    int rightmargin      = 0;
    int leftmargin       = 0;
    int bottommargin     = 0;
    int topmargin        = 0;
    int counter          = 0;
    bool fixtopmargin    = false;
    bool fixrightmargin  = false;
    bool fixleftmargin   = false;
    bool fixbottommargin = false;
    int count=0;

    endupi      = centeri;
    endupj      = centerj;
    travelright = traveldown = -1;
    travelleft  = travelup   = 0;
    count       = limitcolumn + limitrow - 2;

    while(count!=0)
    {
        count--;
        switch((counter%4))
        {
            case 0 :    //travelling right

                travelright += 2;

                if(fixrightmargin == true)
                    travelright--;

                if(fixleftmargin == true)
                    travelright--;

                if(fixtopmargin == true)
                {
                    if(fixrightmargin == false)
                        endrightj++;

                    break;
                }

                startrighti = endupi;
                startrightj = endupj;
                endrightj   = startrightj+travelright;

                if(endrightj >= limitcolumn)
                {
                    endrightj      = limitcolumn-1;
                    fixrightmargin = true;
                    rightmargin    = limitcolumn-1;
                    counter++;
                    travelright--;
                }

                i = startrighti;
                j = startrightj;

                for(j=startrightj+1; j <= endrightj; j++)
                {
                    c = QColor::fromRgb(threshold.pixel(j, i));

                    if(c == Qt::black)
                    {
                        //we have found an empty space
                        fixtopmargin = true;
                        topmargin    = i;
                        endupi++;
                        travelup--;
                        break;
                    }
                }

                endrighti = startrighti;

                break;

            case 1 :    //travelling down

                traveldown += 2;

                if(fixbottommargin == true)
                    traveldown--;

                if(fixtopmargin == true)
                    traveldown--;

                if(fixrightmargin == true)
                {
                    if(fixbottommargin == false)
                        enddowni++;

                    //endrightj--;
                    // kDebug() << "Travelling down : Case Skipped\n";
                    break;
                }

                startdowni = endrighti;
                startdownj = endrightj;
                enddowni   = startdowni + traveldown;

                if(enddowni >= limitrow)
                {
                    enddowni        = limitrow-1;
                    counter++;
                    bottommargin    = limitrow-1;
                    fixbottommargin = true;
                    traveldown--;
                }

                i = startdowni;
                j = startdownj;

                for(i=startdowni+1; i <= enddowni; i++)
                {
                    c = QColor::fromRgb(threshold.pixel(j, i));

                    if(c == Qt::black)
                    {
                        //we have found an empty space
                        fixrightmargin = true;
                        rightmargin    = j;
                        endrightj--;
                        travelright--;
                        break;
                    }
                }

                enddownj = startdownj;

                break;

            case 2 :    //travelling left

                travelleft += 2;

                if(fixleftmargin == true)
                    travelleft--;

                if(fixrightmargin == true)
                    travelleft--;

                if(fixbottommargin == true)
                {
                    if(fixleftmargin == false)
                        endleftj--;

                    break;
                }

                startlefti = enddowni;
                startleftj = enddownj;
                endleftj   = startleftj - travelleft;

                if(endleftj < 0)
                {
                    endleftj      = 0;
                    counter++;
                    leftmargin    = 0;
                    fixleftmargin = true;
                    travelleft--;
                }

                i = startlefti;
                j = startleftj;

                for(j=startleftj-1; j >= endleftj; j--)
                {
                    c = QColor::fromRgb(threshold.pixel(j,i));

                    if(c == Qt::black)
                    {
                        //we have found an empty space
                        fixbottommargin = true;
                        bottommargin    = i;
                        enddowni--;
                        traveldown--;
                        break;
                    }
                }

                endlefti = startlefti;

                break;

            case 3 :    //travelling up

                travelup += 2;

                if(fixbottommargin == true)
                    travelup--;

                if(fixtopmargin == true)
                    travelup--;

                if(fixleftmargin == true)
                {
                    if(fixtopmargin == false)
                        endupi--;

                    break;
                }

                startupi = endlefti;
                startupj = endleftj;
                endupi   = startupi - travelup;

                if(endupi < 0)
                {
                    endupi       = 0;
                    counter++;
                    fixtopmargin = true;
                    topmargin    = 0;
                    travelup--;
                }

                i = startupi;
                j = startupj;

                for(i=startupi-1; i>=endupi; i--)
                {
                    c = QColor::fromRgb(threshold.pixel(j, i));

                    if(c == Qt::black)
                    {
                        //we have found an empty space
                        fixleftmargin = true;
                        leftmargin    = j;
                        endleftj++;
                        travelleft--;
                        break;
                    }
                }

                endupj = startupj;
                break;
        }

        counter++;

        if ( fixbottommargin == true && 
             fixtopmargin    == true &&
             fixleftmargin   == true && 
             fixrightmargin  == true)
            break;
    }

    kDebug() << "Count     : " << count;
    kDebug() << "Endupi    : " << endupi;
    kDebug() << "Endupj    : " << endupj;
    kDebug() << "Endrighti : " << endrighti;
    kDebug() << "Endrightj : " << endrightj;
    kDebug() << "Enddowni  : " << enddowni;
    kDebug() << "Enddownj  : " << enddownj;
    kDebug() << "Endlefti  : " << endlefti;
    kDebug() << "Endleftj  : " << endleftj;
    kDebug() << "Done\n";

    kDebug() << "\n";
    kDebug() << "Left   Margin   : " << leftmargin;
    kDebug() << "Right  Margin   : " << rightmargin;
    kDebug() << "Top    Margin   : " << topmargin;
    kDebug() << "Bottom Margin   : " << bottommargin;
    kDebug() << "Bottom Margin   : " << bottommargin<<"\n";

    //----------------------releasing images

    QPoint icp1;
    icp1.setX(leftColumn+leftmargin+1);
    icp1.setY(topRow+topmargin+1);
    QPoint icp2;
    icp2.setX(leftColumn+rightmargin-1);
    icp2.setY(topRow+bottommargin-1);
    d->cropArea.setTopLeft(icp1);
    d->cropArea.setBottomRight(icp2);

    if( count == 0 )
    {
        d->cropArea = crop;
    }
    kDebug () << "Inner Crop Area : " << d->cropArea;
}

QRect AutoCrop::autoInnerCrop() const
{
    return d->cropArea;
}

} // namespace Digikam
