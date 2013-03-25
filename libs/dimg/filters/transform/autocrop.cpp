/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-10-18
 * Description : Auto Crop Tool for panaroma images generated from hugin
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
        for(int c =0 ; c < 3 ; c++ )
        {
            fimg[c] = 0;
        }
    }

    QRect  cropArea;
    float* fimg[3];
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
    QImage img= m_orgImage.copyQImage();
    int i,j;
    int breakflag=0;
    //    float channel0,channel1, channel2;
    int topRow=-1, topColumn=-1;
    int bottomRow=-1, bottomColumn=-1;
    int leftRow=-1,leftColumn=-1;
    int rightRow=-1,rightColumn=-1;
    QColor c;
    postProgress(5);

    //----------------------Finding the outer boundaries of the image (i.e. with black portions)
    /*
          This would be done in 4 steps
          1. Search column wise:
          (a) From the left to the right, this is to get the left boundary
          (b) From the right to the left, this is to get the right boundary
          2. Search row wise :
          (a) From the top to the bottom, this is to get the top boundary
          (b) From the bottom to the top, this is to get the bottom boundary
          */



    //1(a) Traversing the image from top to bottom, left to right, to get left boundary
    breakflag=0;
    int width=img.width();
    int height=img.height();
    QPoint p;

    //    for(i=210; i<211; i++)
    //    {
    //        for(j=400; j<410; j++)
    //        {
    //            p.setX(j);
    //            p.setY(i);
    //            c = QColor::fromRgb(img.pixel(p));
    //            qDebug() << c;
    //            if(c == Qt::white)
    //                qDebug() << "WHITE! yay!";
    //        }
    //    }
    //    qDebug() << "I'm a genius!";

    for(i=0; i<width; i++)
    {
        for(j=0; j<height; j++)
        {
            c = QColor::fromRgb (img.pixel(i,j) );
            if( c == Qt::black || !c.isValid())
                ;
            else
            {
                //we have found our pixel
                leftRow=j;
                leftColumn=i;
                breakflag=1;
                break;
            }
        }
        if(breakflag == 1)
            break;
    }
    qDebug("Done Till step 1(a)");
    //    postProgress(30,"Left extracted");

    //1(b) Traversing the image from top to bottom, right to left, to get right boundary
    breakflag=0;
    for(i=0; i<width; i++)
    {
        for(j=0; j<height; j++)
        {
            c = QColor::fromRgb (img.pixel(i,j) );
            if(c == Qt::black || !c.isValid())
                ;
            else
            {
                //we have found our pixel
                rightRow=j;
                rightColumn=i;
                break;
            }
        }
    }
    qDebug()<<"Done Till step 1(b)";
    //    postProgress(50,"Right Extracted");

    //2(a) Traversing the image left to right, top to down, to get top boundary
    breakflag=0;
    for(i=0;i<height;i++)
    {
        for(j=0;j<width;j++)
        {
            c = QColor::fromRgb (img.pixel(j,i) );
            if(c == Qt::black || !c.isValid())
                ;
            else
            {
                //we have found our pixel
                topRow=i;
                topColumn=j;
                breakflag=1;
                break;
            }
        }
        if(breakflag==1)
            break;
    }
    qDebug()<<"Done Till step 2(a)";
    //    postProgress(70,"Top Extracted");

    //2(b) Traversing the image from left to right, bottom up, to get lower boundary
    breakflag=0;
    for(i=height-1;i>=0;i--)
    {
        for(j=0;j<width;j++)
        {
            c = QColor::fromRgb (img.pixel(j,i) );
            if(c == Qt::black || !c.isValid())
                ;
            else
            {
                //we have found our pixel
                bottomRow=i;
                bottomColumn=j;
                breakflag=1;
                break;
            }
        }
        if(breakflag==1)
            break;
    }

    qDebug()<<"Done Till step 2(b)";
    //    postProgress(90,"Bottom Extracted");

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
    //    postProgress(91,"Verbose Output done");
    QPoint p1;
    p1.setX(leftColumn);
    p1.setY(topRow);
    QPoint p2;
    p2.setX(rightColumn);
    p2.setY(bottomRow);
    QRect crop;
    crop.setTopLeft(p1);
    crop.setBottomRight(p2);
    qDebug() << "Outer Crop Area : "<<crop;

    qDebug() << "Crop Area : "<<crop;

    //crop Image to outerCrop
    QImage result;
    QSize resultsize = QSize(crop.width(),crop.height());
    result = QImage(resultsize, QImage::Format_RGB888);
    int ni,nj;
    qDebug() << "From "<<crop.top()<<" to "<<crop.bottom()<<" & "<<crop.left()<<" to "<<crop.right();
    for(i=crop.top(),ni=0;i<=crop.bottom();i++,ni++)
    {
        for(j=crop.left(),nj=0;j<=crop.right();j++,nj++)
        {
            result.setPixel(nj,ni,img.pixel(j,i));
        }
    }

    //---------------------threshold the image

    QImage threshold = QImage(resultsize,QImage::Format_RGB888);
    int toggleflag1=0,toggleflag2=0;

    //-----initialize

    for(i=0;i<result.height();i++)
        for(j=0;j<result.width();j++)
            threshold.setPixel(j,i,Qt::black);

    //----------mark points on horizontal scan
    for(i=0;i<result.height();i++)
    {
        toggleflag1=-1;
        toggleflag2=-1;
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
        if(toggleflag1>=0)
            for(j=toggleflag1;j<=toggleflag2;j++)
                threshold.setPixel(j,i,qRgb(255,255,255));
    }

    //----------fill black points on vertical scan
    for(j=0;j<result.width();j++)
    {
        toggleflag1=-1;
        toggleflag2=-2;
        for(i=0;i<result.height();i++)
        {
            c = QColor::fromRgb (result.pixel(j,i) );
            if(c != Qt::black)
            {
                toggleflag1=i;
                break;
            }
        }
        for(i=(result.height()-1);i>=0;i--)
        {
            c = QColor::fromRgb (result.pixel(j,i) );
            if(c != Qt::black)
            {
                toggleflag2=i;
                break;
            }
        }
        if(toggleflag1>=0)
            for(i=0;i<toggleflag1;i++)
                threshold.setPixel(j,i,qRgb(0,0,0));

        if(toggleflag2>=0)
            for(i=(toggleflag2+1);i<(result.height());i++)
                threshold.setPixel(j,i,qRgb(0,0,0));
    }
    qDebug() << "Thresholding Complete\n";

    //---------------------inner crop
    int limitcolumn = threshold.width();
    int limitrow    = threshold.height();
    int centeri=((limitrow/2)-1);
    int centerj=((limitcolumn/2)-1);
    int startrighti=0,    startrightj=0,    endrighti=0,      endrightj=0;
    int startlefti=0,     startleftj=0,     endlefti=0,       endleftj=0;
    int startupi=0,       startupj=0,       endupi=0,         endupj=0;
    int startdowni=0,     startdownj=0,     enddowni=0,       enddownj=0;
    int travelright=0,    travelleft=0,     travelup=0,       traveldown=0;
    int rightmargin=0,    leftmargin=0,     bottommargin=0,   topmargin=0;
    int counter=0;
    bool fixtopmargin    =       false;
    bool fixrightmargin  =       false;
    bool fixleftmargin   =       false;
    bool fixbottommargin =       false;
    //    int count=0;
    endupi = centeri;
    endupj = centerj;
    travelright = traveldown = -1;
    travelleft  = travelup   = 0;

    while(true)
    {
        switch((counter%4))
        {
        case 0 :    //travelling right
            travelright += 2;
            if(fixrightmargin==true)
                travelright--;
            if(fixleftmargin==true)
                travelright--;
            if(fixtopmargin==true)
            {
                if(fixrightmargin==false)
                    endrightj++;
                break;
            }
            startrighti=endupi;
            startrightj=endupj;
            endrightj=startrightj+travelright;
            if(endrightj >= limitcolumn)
            {
                endrightj = limitcolumn-1;
                fixrightmargin=true;
                rightmargin=limitcolumn-1;
                counter++;
                travelright--;
            }
            i=startrighti;
            j=startrightj;
            for(j=startrightj+1;j<=endrightj;j++)
            {
                c=QColor::fromRgb(threshold.pixel(j,i));
                if(c == Qt::black)
                {
                    //we have found an empty space
                    fixtopmargin=true;
                    topmargin=i;
                    endupi++;
                    travelup--;
                    break;
                }
            }
            endrighti=startrighti;

            break;
        case 1 :    //travelling down
            traveldown += 2;
            if(fixbottommargin==true)
                traveldown--;
            if(fixtopmargin==true)
                traveldown--;
            if(fixrightmargin==true)
            {
                if(fixbottommargin==false)
                    enddowni++;
                //endrightj--;
                // qDebug() << "Travelling down : Case Skipped\n";
                break;
            }
            startdowni=endrighti;
            startdownj=endrightj;
            enddowni = startdowni + traveldown;
            if(enddowni >= limitrow)
            {
                enddowni = limitrow-1;
                counter++;
                bottommargin=limitrow-1;
                fixbottommargin=true;
                traveldown--;
            }
            i=startdowni;
            j=startdownj;
            for(i=startdowni+1; i<=enddowni; i++)
            {
                c=QColor::fromRgb(threshold.pixel(j,i));
                if(c == Qt::black)
                {
                    //we have found an empty space
                    fixrightmargin=true;
                    rightmargin=j;
                    endrightj--;
                    travelright--;
                    break;
                }
            }
            enddownj=startdownj;

            break;
        case 2 :    //travelling left
            travelleft += 2;
            if(fixleftmargin==true)
                travelleft--;
            if(fixrightmargin==true)
                travelleft--;
            if(fixbottommargin==true)
            {
                if(fixleftmargin==false)
                    endleftj--;
                break;
            }
            startlefti=enddowni;
            startleftj=enddownj;
            endleftj = startleftj - travelleft;
            if(endleftj < 0)
            {
                endleftj = 0;
                counter++;
                leftmargin=0;
                fixleftmargin=true;
                travelleft--;
            }
            i=startlefti;
            j=startleftj;
            for(j=startleftj-1;j>=endleftj;j--)
            {
                c=QColor::fromRgb(threshold.pixel(j,i));
                if(c == Qt::black)
                {
                    //we have found an empty space
                    fixbottommargin=true;
                    bottommargin=i;
                    enddowni--;
                    traveldown--;
                    break;
                }
            }
            endlefti=startlefti;

            break;
        case 3 :    //travelling up
            travelup += 2;
            if(fixbottommargin==true)
                travelup--;
            if(fixtopmargin==true)
                travelup--;
            if(fixleftmargin==true)
            {
                if(fixtopmargin==false)
                    endupi--;
                break;
            }
            startupi=endlefti;
            startupj=endleftj;
            endupi = startupi - travelup;
            if(endupi < 0)
            {
                endupi = 0;
                counter++;
                fixtopmargin=true;
                topmargin=0;
                travelup--;
            }
            i=startupi;
            j=startupj;
            for(i=startupi-1; i>=endupi; i--)
            {
                c=QColor::fromRgb(threshold.pixel(j,i));
                if(c == Qt::black)
                {
                    //we have found an empty space
                    fixleftmargin=true;
                    leftmargin=j;
                    endleftj++;
                    travelleft--;
                    break;
                }
            }
            endupj=startupj;

            break;
        }
        counter++;
        if ( fixbottommargin==true && fixtopmargin==true && fixleftmargin==true && fixrightmargin==true)
            break;
    }
    kDebug() << "Endupi    : "<<endupi;
    kDebug() << "Endupj    : "<<endupj;
    kDebug() << "Endrighti : "<<endrighti;
    kDebug() << "Endrightj : "<<endrightj;
    kDebug() << "Enddowni  : "<<enddowni;
    kDebug() << "Enddownj  : "<<enddownj;
    kDebug() << "Endlefti  : "<<endlefti;
    kDebug() << "Endleftj  : "<<endleftj;
    kDebug() << "Done\n";

    kDebug() << "\n";
    kDebug() << "Left   Margin   : "<<leftmargin;
    kDebug() << "Right  Margin   : "<<rightmargin;
    kDebug() << "Top    Margin   : "<<topmargin;
    kDebug() << "Bottom Margin   : "<<bottommargin;
    qDebug() << "Bottom Margin   : "<<bottommargin<<"\n";

    //----------------------releasing images
    QPoint icp1;
    icp1.setX(leftColumn+leftmargin+1);
    icp1.setY(topRow+topmargin+1);
    QPoint icp2;
    icp2.setX(leftColumn+rightmargin-1);
    icp2.setY(topRow+bottommargin-1);
    d->cropArea.setTopLeft(icp1);
    d->cropArea.setBottomRight(icp2);
    qDebug () << "Inner Crop Area : " << d->cropArea;
}

//void AutoCrop::readImage() const
//{
//    DColor col;

//    for (int c = 0;   (c < 3); c++)
//    {
//        d->fimg[c] = new float[m_orgImage.numPixels()];
//    }

//    int j = 0;

//    for (uint y = 0;   (y < m_orgImage.height()); y++)
//    {
//        for (uint x = 0;   (x < m_orgImage.width()); x++)
//        {
//            col           = m_orgImage.getPixelColor(x, y);
//            d->fimg[0][j] = col.red();
//            d->fimg[1][j] = col.green();
//            d->fimg[2][j] = col.blue();
//            j++;
//        }
//    }
//}

QRect AutoCrop::autoOuterCrop() const
{
    return d->cropArea;
}

} // namespace Digikam
