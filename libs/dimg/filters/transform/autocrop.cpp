/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-10-18
 * Description : Auto Crop analyzer
 *
 * Algorithm based on black point detection on the basis of spiral
 * traversal
 *
 * Copyright (C) 2013 by Sayantan Datta <sayantan dot knz at gmail dot com>
 * Copyright (C) 2013-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "autocrop.h"

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

// Local includes

#include "digikam_debug.h"

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
    : DImgThreadedAnalyser(parent, QLatin1String("AutoCrop")),
      d(new Private)
{
    setOriginalImage(*img);
}

AutoCrop::~AutoCrop()
{
    delete d;
}

QRect AutoCrop::spiralClockwiseTraversal(const QImage& source, int topCrop, int bottomCrop)
{
    int i, j, ni;

    if(topCrop == -1)
    {
        topCrop=0;
    }

    if(bottomCrop == -1)
    {
        bottomCrop=source.height();
    }

    QSize resultsize = QSize(source.width(),(source.height()-topCrop-(source.height()-bottomCrop)));
    QImage threshold = QImage(resultsize, QImage::Format_RGB888);

    for(i=topCrop, ni=0; i<bottomCrop ; i++, ni++)
    {
        for(j=0; j<source.width(); j++)
        {
            threshold.setPixel(j,ni,source.pixel(j,i));
        }
    }

    QColor c;
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
//    int count            = limitcolumn + limitrow -1;
    bool rightEdge       = false;
    bool leftEdge        = false;
    bool topEdge         = false;
    bool bottomEdge      = false;

    endupi      = centeri;
    endupj      = centerj;
    travelright = traveldown = -1;
    travelleft  = travelup   = 0;

    qCDebug(DIGIKAM_DIMG_LOG) << "Center pixel : "<<centerj<<" , "<< centeri;

    while(true)
    {
//        qCDebug(DIGIKAM_DIMG_LOG) << "count = "<<count;

        switch((counter%4))
        {
            case 0 :    //traveling right
            {
                if(fixtopmargin == true)
                {
                    if(fixrightmargin == false)
                    {
                        endrightj++;

                        if(endrightj >= threshold.width())
                        {
                            qCDebug(DIGIKAM_DIMG_LOG) << "We cannot go right anymore";
                            fixrightmargin = true;
                            rightmargin    = limitcolumn - 1;
                            rightEdge      = true;
                        }
                    }

                    break;
                }

                travelright += 2;

                if(fixrightmargin == true)
                    travelright--;

                if(fixleftmargin == true)
                    travelright--;

    //            qCDebug(DIGIKAM_DIMG_LOG) << "TRAVELLING RIGHT";
    //            qCDebug(DIGIKAM_DIMG_LOG) << "Endupi" << endupi;
                startrighti = endupi;
                startrightj = endupj;
                endrightj   = startrightj+travelright;
                //            qCDebug(DIGIKAM_DIMG_LOG) << "Moving Right EndRight = " << endrightj;

                if(endrightj >= limitcolumn)
                {
                    qCDebug(DIGIKAM_DIMG_LOG) << "We have reached limitcolumn, i.e. width";
                    endrightj      = limitcolumn-1;
                    fixrightmargin = true;
                    rightmargin    = limitcolumn-1;
                    counter++;
                    travelright--;
                    rightEdge      = true;
                }

                i = startrighti;
                j = startrightj;

                for(j=startrightj+1;j<=endrightj;j++)
                {
    //                qCDebug(DIGIKAM_DIMG_LOG) << "At pixel "<< j << " , " << i;
                    c = QColor::fromRgb(threshold.pixel(j,i));

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
            }
            case 1 :    //traveling down
            {
                if(fixrightmargin == true)
                {
                    if(fixbottommargin == false)
                    {
                        enddowni++;

                        if(enddowni >= limitrow)
                        {
                            fixbottommargin = true;
                            bottommargin    = limitrow-1;
                            bottomEdge      = true;
                        }
                    }

                    //endrightj--;
                    // qCDebug(DIGIKAM_DIMG_LOG) << "Traveling down : Case Skipped\n";
                    break;
                }
                traveldown += 2;

                if(fixbottommargin==true)
                    traveldown--;

                if(fixtopmargin==true)
                    traveldown--;

                startdowni = endrighti;
                startdownj = endrightj;
                enddowni   = startdowni + traveldown;
                //            qCDebug(DIGIKAM_DIMG_LOG) << "Moving Down EndDown = " << enddowni;

                if(enddowni >= limitrow)
                {
                    qCDebug(DIGIKAM_DIMG_LOG) << "We have reached limitrow, i.e. Height";
                    enddowni        = limitrow-1;
                    counter++;
                    bottommargin    = limitrow-1;
                    fixbottommargin = true;
                    traveldown--;
                    bottomEdge      = true;
                }

                i = startdowni;
                j = startdownj;

                for(i=startdowni+1; i<=enddowni; i++)
                {
    //                qCDebug(DIGIKAM_DIMG_LOG) << "At pixel "<< j << " , " << i;
                    c = QColor::fromRgb(threshold.pixel(j,i));

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
            }
            case 2 :    //traveling left
            {
                if(fixbottommargin == true)
                {
                    if(fixleftmargin == false)
                    {
                        endleftj--;

                        if(endleftj < 0)
                        {
                            fixleftmargin = true;
                            leftmargin    = 0;
                            leftEdge      = true;
                        }
                    }

                    break;
                }

                travelleft += 2;

                if(fixleftmargin == true)
                    travelleft--;

                if(fixrightmargin == true)
                    travelleft--;

                startlefti = enddowni;
                startleftj = enddownj;
                endleftj   = startleftj - travelleft;
                //            qCDebug(DIGIKAM_DIMG_LOG) << "Moving Left Endleft = " << endleftj;

                if(endleftj < 0)
                {
                    qCDebug(DIGIKAM_DIMG_LOG) << "We have gone too left";
                    endleftj      = 0;
                    counter++;
                    leftmargin    = 0;
                    fixleftmargin = true;
                    travelleft--;
                    leftEdge      = true;
                }

                i = startlefti;
                j = startleftj;

                for(j=startleftj-1;j>=endleftj;j--)
                {
    //                qCDebug(DIGIKAM_DIMG_LOG) << "At pixel "<< j << " , " << i;
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
            }
            case 3 :    //traveling up
            {
                if(fixleftmargin == true)
                {
                    if(fixtopmargin == false)
                    {
                        endupi--;
                        endupj = leftmargin;

                        if(endupi < 0)
                        {
                            fixtopmargin = true;
                            topmargin    = 0;
                            topEdge      = true;
                        }
                    }

                    break;
                }

                travelup += 2;

                if(fixbottommargin == true)
                    travelup--;

                if(fixtopmargin == true)
                    travelup--;

                startupi = endlefti;
                startupj = endleftj;
                endupi   = startupi - travelup;
                //            qCDebug(DIGIKAM_DIMG_LOG) << "Moving Up Endup = " << endupi;

                if(endupi < 0)
                {
                    qCDebug(DIGIKAM_DIMG_LOG) << "We have gone too right";
                    endupi       = 0;
                    topEdge      = true;
                    counter++;
                    fixtopmargin = true;
                    topmargin    = 0;
                    travelup--;
                }

                i = startupi;
                j = startupj;

                for(i=startupi-1; i>=endupi; i--)
                {
    //                qCDebug(DIGIKAM_DIMG_LOG) << "At pixel "<< j << " , " << i;
                    c = QColor::fromRgb(threshold.pixel(j,i));

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

                endupj=startupj;

                break;
            }
        }

        counter++;

        if ( fixbottommargin == true && fixtopmargin == true && fixleftmargin == true && fixrightmargin == true)
            break;
    }

//    qCDebug(DIGIKAM_DIMG_LOG) << "Count     : " << count;
    qCDebug(DIGIKAM_DIMG_LOG) << "Endupi    : " << endupi;
    qCDebug(DIGIKAM_DIMG_LOG) << "Endupj    : " << endupj;
    qCDebug(DIGIKAM_DIMG_LOG) << "Endrighti : " << endrighti;
    qCDebug(DIGIKAM_DIMG_LOG) << "Endrightj : " << endrightj;
    qCDebug(DIGIKAM_DIMG_LOG) << "Enddowni  : " << enddowni;
    qCDebug(DIGIKAM_DIMG_LOG) << "Enddownj  : " << enddownj;
    qCDebug(DIGIKAM_DIMG_LOG) << "Endlefti  : " << endlefti;
    qCDebug(DIGIKAM_DIMG_LOG) << "Endleftj  : " << endleftj;
    qCDebug(DIGIKAM_DIMG_LOG) << "Done\n";

    qCDebug(DIGIKAM_DIMG_LOG) << "Left   Margin   : " << leftmargin;
    qCDebug(DIGIKAM_DIMG_LOG) << "Right  Margin   : " << rightmargin;
    qCDebug(DIGIKAM_DIMG_LOG) << "Top    Margin   : " << topmargin;
    qCDebug(DIGIKAM_DIMG_LOG) << "Bottom Margin   : " << bottommargin;
    qCDebug(DIGIKAM_DIMG_LOG) << "Done\n";

    qCDebug(DIGIKAM_DIMG_LOG) << "Left Edge   : " << leftEdge;
    qCDebug(DIGIKAM_DIMG_LOG) << "Right Edge  : " << rightEdge;
    qCDebug(DIGIKAM_DIMG_LOG) << "Top Edge    : " << topEdge;
    qCDebug(DIGIKAM_DIMG_LOG) << "Bottom Edge : " << bottomEdge;
    qCDebug(DIGIKAM_DIMG_LOG) << "Done\n";

    if(bottomEdge)
    {
        bottommargin++;
    }

    if(topEdge)
    {
        topmargin--;
    }

    if(leftEdge)
    {
        leftmargin--;
    }

    if(rightEdge)
    {
        rightmargin++;
    }

    //----------------------releasing images
    QPoint icp1;
    icp1.setX(leftmargin+1);
    icp1.setY(topCrop+topmargin+1);
    QPoint icp2;
    icp2.setX(rightmargin-1);
    icp2.setY(topCrop+bottommargin-1);
    QRect cropArea;
    cropArea.setTopLeft(icp1);
    cropArea.setBottomRight(icp2);
    return cropArea;
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

    qCDebug(DIGIKAM_DIMG_LOG) << "Done Till step 1(a)";
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

    qCDebug(DIGIKAM_DIMG_LOG) << "Done Till step 1(b)";
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

    qCDebug(DIGIKAM_DIMG_LOG) << "Done Till step 2(a)";
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

        if(breakflag == 1)
            break;
    }

    qCDebug(DIGIKAM_DIMG_LOG) << "Done Till step 2(b)";
    postProgress(90);

    //------making the required output--------------------

    QString outercropParameters;
    outercropParameters.append(QLatin1String("TopMost Pixel : ( "));
    outercropParameters.append(QString::number(topRow));
    outercropParameters.append(QLatin1String(", "));
    outercropParameters.append(QString::number(topColumn));
    outercropParameters.append(QLatin1String(")\nBottomMost Pixel : ( "));
    outercropParameters.append(QString::number(bottomRow));
    outercropParameters.append(QLatin1String(", "));
    outercropParameters.append(QString::number(bottomColumn));
    outercropParameters.append(QLatin1String(")\nLeftMost Pixel : ( "));
    outercropParameters.append(QString::number(leftRow));
    outercropParameters.append(QLatin1String(", "));
    outercropParameters.append(QString::number(leftColumn));
    outercropParameters.append(QLatin1String(")\nRightMost Pixel : ( "));
    outercropParameters.append(QString::number(rightRow));
    outercropParameters.append(QLatin1String(", "));
    outercropParameters.append(QString::number(rightColumn));
    outercropParameters.append(QLatin1String(")\nDONE"));
    qCDebug(DIGIKAM_DIMG_LOG) << outercropParameters;
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

    qCDebug(DIGIKAM_DIMG_LOG) << "Outer Crop area:";
    qCDebug(DIGIKAM_DIMG_LOG) << "From "<< crop.top()  << " to " << crop.bottom()
             << " & "  << crop.left() << " to " << crop.right();

    for(i=crop.top(), ni=0; i <= crop.bottom(); i++, ni++)
    {
        for(j=crop.left(), nj=0; j <= crop.right(); j++, nj++)
        {
            result.setPixel(nj, ni, img.pixel(j, i));
        }
    }

    //---------------------threshold the image

    QImage threshold    = QImage(resultsize,QImage::Format_RGB888);
    int toggleflag1     = 0,toggleflag2 = 0;
    int whitepixelCount = 0;

    //-----initialize

    for(i=0;i<result.height();i++)
    {
        for(j=0;j<result.width();j++)
        {
            threshold.setPixel(j,i,Qt::black);
        }
    }

    //----------mark points on horizontal scan
    for(i=0;i<result.height();i++)
    {
        toggleflag1 = -1;
        toggleflag2 = -1;

        for(j=0;j<result.width();j++)
        {
            c = QColor::fromRgb (result.pixel(j,i) );

            if(c != Qt::black)
            {
                toggleflag1=j;
                break;
            }
        }

        for(j=(result.width()-1);j>=0;j--)
        {
            c = QColor::fromRgb (result.pixel(j,i) );

            if(c != Qt::black)
            {
                toggleflag2=j;
                break;
            }
        }

        if(toggleflag1 >= 0)
        {
            for(j=toggleflag1;j<=toggleflag2;j++)
            {
                threshold.setPixel(j,i,qRgb(255,255,255));
            }
        }
    }

    //----------fill black points on vertical scan

    for(j=0;j<result.width();j++)
    {
        toggleflag1 = -1;
        toggleflag2 = -2;

        for(i=0;i<result.height();i++)
        {
            c = QColor::fromRgb (result.pixel(j,i) );

            if(c != Qt::black)
            {
                toggleflag1 = i;
                break;
            }
        }

        for(i=(result.height()-1);i>=0;i--)
        {
            c = QColor::fromRgb (result.pixel(j,i) );

            if(c != Qt::black)
            {
                toggleflag2 = i;
                break;
            }
        }

        if(toggleflag1>=0)
        {
            for(i=0;i<toggleflag1;i++)
            {
                threshold.setPixel(j,i,qRgb(0,0,0));
            }
        }

        if(toggleflag2>=0)
        {
            for(i=(toggleflag2+1);i<(result.height());i++)
            {
                threshold.setPixel(j,i,qRgb(0,0,0));
            }
        }
    }

    // ---count of white pixel in threshold

    for(i = 0; i <threshold.height(); i++)
    {
        for (j = 0; j<threshold.width(); j++)
        {
            c = QColor::fromRgb(threshold.pixel(j,i));

            if( c == Qt::white)
            {
                whitepixelCount++;
            }
        }
    }

    qCDebug(DIGIKAM_DIMG_LOG) << "White pixel count in thresholded image = " << whitepixelCount;
    qCDebug(DIGIKAM_DIMG_LOG) << "Thresholding Complete\n";

    //---------------------inner crop

    QRect InrCrop = spiralClockwiseTraversal(threshold,-1,-1);
    QPoint icp1;
    icp1.setX(InrCrop.topLeft().x()+leftColumn);
    icp1.setY(InrCrop.topLeft().y()+topRow);
    QPoint icp2;
    icp2.setX(InrCrop.bottomRight().x()+leftColumn);
    icp2.setY(InrCrop.bottomRight().y()+topRow);
    QRect cropArea;
    cropArea.setTopLeft(icp1);
    cropArea.setBottomRight(icp2);

    qCDebug(DIGIKAM_DIMG_LOG) << "cropArea : "<<cropArea;

    //Step 1. check for extra small crop
    //Step 2. Find out first minima from left and right, crop accordingly

    //-----Step 1 -- Check for extra small crop

    int area = cropArea.height() * cropArea.width();

    if(area > (whitepixelCount/1.43))
    {
        d->cropArea.setTopLeft(icp1);
        d->cropArea.setBottomRight(icp2);
        qCDebug(DIGIKAM_DIMG_LOG) << "Inner Crop Area : " << d->cropArea;
        return;
    }
    else
    {
//         threshold.save("ThresholdedImage.jpg",0,100);
        qCDebug(DIGIKAM_DIMG_LOG) << "Area not adequate!";
        qCDebug(DIGIKAM_DIMG_LOG) << "Extra Cropping Required";
        // --- Step 2 -- Search between local minima
        qCDebug(DIGIKAM_DIMG_LOG) << "In local minima function";
        //We need to find the maxima between the first two local minima from either side
        int* const blackpointCount = new int[threshold.width()];
        int leftminima     = 0;
        int rightminima    = (threshold.width()-1);
        int topCropLine    = 0;
        int bottomCropLine = threshold.height()-1;
        int temp;
        int temppos;
        int count;

        //initialize black point count
        for(i=0; i<threshold.width(); i++)
        {
            blackpointCount[i] = 0;
        }

        for(j=0; j<threshold.width(); j++)
        {
            count = 0;

            for(i=0; i<threshold.height(); i++)
            {
                c = QColor::fromRgb(threshold.pixel(j,i));

                if ( c == Qt::black)
                {
                    count++;
                }
                else
                {
                    break;
                }
            }

            blackpointCount[j] = count;
        }

        qCDebug(DIGIKAM_DIMG_LOG) << "Top black element count Data Entry Completed";

        // --- Searching left minima

        for(j=1;j<threshold.width();j++)
        {
            if((blackpointCount[j]>blackpointCount[j-1]) && (blackpointCount[j]<(0.2*threshold.height())))
            {
                leftminima = j-1;
                break;
            }
        }

        for(j=(threshold.width()-2); j>=0; j--)
        {
            if((blackpointCount[j]>blackpointCount[j+1]) && (blackpointCount[j]<(0.2*threshold.height())))
            {
                rightminima = j+1;
                break;
            }
        }

        qCDebug(DIGIKAM_DIMG_LOG) << "Top Part right minima : " << rightminima << " Left Minima : " << leftminima;

        // --- find the maximum among these minima

        temp    = blackpointCount[leftminima];
        temppos = leftminima;

        for(j=leftminima+1; j<=rightminima; j++)
        {
            if(temp < blackpointCount[j])
            {
                temp    = blackpointCount[j];
                temppos = j;
            }
        }

        topCropLine = temp;
        qCDebug(DIGIKAM_DIMG_LOG) << "Found top crop line";
        qCDebug(DIGIKAM_DIMG_LOG) << "Found in column = " << temppos << "and the topCropLine is "<< topCropLine;
        qCDebug(DIGIKAM_DIMG_LOG) << "Searching for bottom crop line";

        //-----For the bottom of the image

        //initialize black point count
        for(i=0; i<threshold.width(); i++)
        {
            blackpointCount[i] = 0;
        }

        for(j=0; j<threshold.width(); j++)
        {
            count = 0;

            for(i=(threshold.height()-1); i>=0; i--)
            {
                c = QColor::fromRgb(result.pixel(j,i));

                if ( c == Qt::black)
                {
                    count++;
                }
                else
                {
                    break;
                }
            }

            blackpointCount[j] = count;
        }

        qCDebug(DIGIKAM_DIMG_LOG) << "Bottom black element count Data Entry Completed";

        // --- Searching left minima

        for(j=1;j<threshold.width();j++)
        {
            if((blackpointCount[j]>blackpointCount[j-1]) && (blackpointCount[j]<(0.2*threshold.height())))
            {
                leftminima = j-1;
                break;
            }
        }

        for(j=(threshold.width()-2); j>=0; j--)
        {
            if((blackpointCount[j]>blackpointCount[j+1]) && (blackpointCount[j]<(0.2*threshold.height())))
            {
                rightminima = j+1;
                break;
            }
        }

        // --- find the maximum among these minima

        temp    = blackpointCount[leftminima];
        temppos = leftminima;

        for(j=leftminima+1; j<=rightminima; j++)
        {
            if(temp < blackpointCount[j])
            {
                temp    = blackpointCount[j];
                temppos = j;
            }
        }

        bottomCropLine = temp;
        qCDebug(DIGIKAM_DIMG_LOG) << "Found top crop line";
        qCDebug(DIGIKAM_DIMG_LOG) << "Found in column = " << temppos;
        QRect newCrop = spiralClockwiseTraversal(threshold,topCropLine,(threshold.height()-bottomCropLine));

        if(newCrop != crop)
        {
            icp1.setX(newCrop.topLeft().x()+leftColumn);
            icp1.setY(newCrop.topLeft().y()+topRow);
            icp2.setX(newCrop.bottomRight().x()+leftColumn);
            icp2.setY(newCrop.bottomRight().y()+topRow);
            d->cropArea.setTopLeft(icp1);
            d->cropArea.setBottomRight(icp2);
        }

        delete [] blackpointCount;
    }

    qCDebug(DIGIKAM_DIMG_LOG) << "Inner Crop Area : " << cropArea;
//    return(cropArea);
//    resultsize = QSize (cropArea.width(), cropArea.height());
//    QImage ic = QImage(resultsize,img.format());
//    for(i=cropArea.top(), ni=0 ; i<=cropArea.bottom() ; i++, ni++ )
//    {
//        for (j=cropArea.left(), nj=0 ; j<=cropArea.right();j++,nj++ )
//        {
//            ic.setPixel(nj,ni,img.pixel(j,i));
//        }
//    }
//    qCDebug(DIGIKAM_DIMG_LOG) << "From "<<cropArea.top()<<" to "<<cropArea.bottom()<<" & "<<cropArea.left()<<" to "<<cropArea.right();
//    if(ic.save("InnerCrop.jpg",0,100))
//        qCDebug(DIGIKAM_DIMG_LOG) << "Inner Crop Function Saves the day!";
//    else
//        qCDebug(DIGIKAM_DIMG_LOG) << "Inner Crop Functions fails";

    qCDebug(DIGIKAM_DIMG_LOG) << "Inner Crop Area : " << d->cropArea;
}

QRect AutoCrop::autoInnerCrop() const
{
    return d->cropArea;
}

} // namespace Digikam
