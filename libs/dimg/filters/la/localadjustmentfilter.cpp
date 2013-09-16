/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-07-22
 * Description : Local Adjustment Tool
 *
 * This makes a color based selection from the image, in a circular mask
 * Uses CIELAB94 as color comparison tool.
 *
 * Produces output of a Image file (the mask).
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

// local Includes

#include "localadjustmentfilter.h"

// C++ includes

#include <cmath>
#include <cfloat>
#include <QtCore/qvarlengtharray.h>

// Qt includes.

#include <QTextStream>
#include <QFile>
#include <QDebug>
#include <QApplication>
#include <QPoint>
#include <QRect>
#include <QImage>
#include <QColor>
#include <QPainter>
#include <qmath.h>
#include <QFileDialog>
#include <QMessageBox>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QPainter>
#include <QPixmap>

//KDE Includes

#include <kdebug.h>

namespace Digikam
{

LAContainer::LAContainer()
{
    red                  = 0;
    blue                 = 0;
    green                = 0;
    alpha                = 0;
    hue                  = 0;
    saturation           = 0;
    vibrance             = 0;
    lightness            = 0;
    radius               = 0;
    center.setX(0);
    center.setY(0);
    selectionCenter.setX(0);
    selectionCenter.setY(0);
//    selection            = NULL;
//    modSelection;
}

LAContainer::LAContainer(DImg part)
{
    red                  = 0;
    blue                 = 0;
    green                = 0;
    alpha                = 0;
    hue                  = 0;
    saturation           = 0;
    vibrance             = 0;
    lightness            = 0;
    radius               = 0;
    center.setX(0);
    center.setY(0);
    selectionCenter.setX(0);
    selectionCenter.setY(0);
    selection            = part;
//    modSelection;
}

LAContainer::~LAContainer()
{
}

QDebug operator<<(QDebug dbg, const LAContainer& inf)
{
    dbg.nospace() << "center:"
                  << inf.center << ", ";
    dbg.nospace() << "Selection Center:"
                  << inf.selectionCenter << ", ";
    dbg.nospace() << "Radius:"
                  << inf.radius << ", ";
    dbg.nospace() << "Red: "
                  << inf.red << ", ";
    dbg.nospace() << "Blue: "
                  << inf.blue << ", ";
    dbg.nospace() << "Green: "
                  << inf.green << ", ";
    dbg.nospace() << "Hue: "
                  << inf.hue << ", ";
    dbg.nospace() << "Saturation: "
                  << inf.saturation << ", ";
    dbg.nospace() << "Vibrance: "
                  << inf.vibrance << ", ";
    dbg.nospace() << "Lightness: "
                  << inf.lightness;
    return dbg.space();
}


class LocalAdjustments::Private
{
public:

    Private()
    {
    }
    DImg selection;
    DImg finalImage;
    QPoint selectionCenter;
    int centerX;
    int centerY;
    int radius;
    LAContainer sln[20];    //keep a maximum limit of 20 selections for work.
    int count;              //to keep a count of the next selection would be
};


LocalAdjustments::LocalAdjustments(DImg* const img, QObject* const parent)
    : DImgThreadedFilter(parent, "LocalAdjustments"), d(new Private)
{
    d->count = 0;
    setOriginalImage(*img);
}

LocalAdjustments::LocalAdjustments(DImg *const orgImage, QObject *const parent, const LAContainer &settings)
    : DImgThreadedFilter(orgImage, parent, "Local Adjustments"), d(new Private)
{
    d->sln[0] = settings;
    d->count  = 1;
    initFilter();
}

LocalAdjustments::~LocalAdjustments()
{
    delete d;
}

int LocalAdjustments::addSelection(LAContainer lac)
{
    d->sln[d->count] = lac;
    createSelection(&(d->sln[d->count]));
    d->count++;
    return (d->count - 1);
}

void LocalAdjustments::createSelection(LAContainer *lac)
{
    //function yet to be made.
}

void LocalAdjustments::createSelection(int index)
{
    /**
        d->centerX              = d->sln[index].center.x();
        d->centerY              = d->sln[index].center.x();
        d->radius               = d->sln[index].radius;
        d->selectionCenter      = centerSelection();
        */
    d->sln[index].selectionCenter           = centerSelection(d->sln[index].center.x() , d->sln[index].center.y() , d->sln[index].radius );
    d->sln[index].selection                 = getDImgSoftSelection(d->sln[index].center, d->sln[index].selectionCenter, d->sln[index].radius);
    DImg temp                               = d->sln[index].selection;
    d->sln[index].selection                 = getDImgColorSelection(temp, d->sln[index].selectionCenter );
    d->sln[index].selection.save("After Creation.png","PNG");
}

void LocalAdjustments::filterImage()
{
    postProgress(0);
    int i;
    uint x;
    uint y;
    DColor col;

    for (i = 0 ; (i < d->count) && (i < 20); i ++ )
    {
        createSelection(i);
        changeSingleRGBA(i);
    }

    postProgress(50);

    applyAllSelections();

    postProgress(100);

    for ( x = 0 ; runningFlag() && (x < m_orgImage.width()); ++x)
    {
        for ( y = 0 ; runningFlag() && (y < m_orgImage.width()); ++y)
        {
            col = d->finalImage.getPixelColor(x,y);
            m_destImage.setPixelColor(x,y,col);
        }
    }
}

FilterAction LocalAdjustments::filterAction()
{
    FilterAction action(FilterIdentifier(), CurrentVersion());
    action.setDisplayableName(DisplayableName());

    action.addParameter("centerX",    d->sln[0].center.x());
    action.addParameter("centerY",    d->sln[0].center.y());
    action.addParameter("radius",     d->sln[0].radius);
    action.addParameter("red",        d->sln[0].red);
    action.addParameter("blue",       d->sln[0].blue);
    action.addParameter("green",      d->sln[0].green);
    action.addParameter("alpha",      d->sln[0].alpha);
    action.addParameter("hue",        d->sln[0].hue);
    action.addParameter("lightness",  d->sln[0].lightness);
    action.addParameter("saturation", d->sln[0].saturation);
    action.addParameter("vibrance",   d->sln[0].vibrance);

    return action;
}

void LocalAdjustments::readParameters(const FilterAction& action)
{
    d->sln[0].center.setX(action.parameter("centerX").toDouble());
    d->sln[0].center.setY(action.parameter("centerY").toDouble());
    d->sln[0].radius       = action.parameter("radius").toDouble();
    d->sln[0].red          = action.parameter("red").toDouble();
    d->sln[0].blue         = action.parameter("blue").toDouble();
    d->sln[0].green        = action.parameter("green").toDouble();
    d->sln[0].alpha        = action.parameter("alpha").toDouble();
    d->sln[0].hue          = action.parameter("hue").toDouble();
    d->sln[0].lightness    = action.parameter("lightness").toDouble();
    d->sln[0].saturation   = action.parameter("saturation").toDouble();
    d->sln[0].vibrance     = action.parameter("vibrance").toDouble();
}

QString LocalAdjustments::filterIdentifier() const
{
    return FilterIdentifier();
}

QString LocalAdjustments::FilterIdentifier()
{
    return "digikam:LocalAdjustmentFilter";
}

QString LocalAdjustments::DisplayableName()
{
    return I18N_NOOP("Local Adjustment Filter");
}

QList<int> LocalAdjustments::SupportedVersions()
{
    return QList<int>() << 1;
}

int LocalAdjustments::CurrentVersion()
{
    return 1;
}

void LocalAdjustments::startAnalyse()
{
    //fill the d->selection and the selectionCenter

    //d->selectionCenter = centerSelection();
    int i;
    for (i = 0 ; (i < d->count) && (i < 20); i ++ )
    {
        createSelection(i);
        changeSingleRGBA(i);
    }

    //color modify the selections based on RGBA


}

QPoint LocalAdjustments::centerSelection(int x, int y, int radius)
{
    QImage img         = m_orgImage.copyQImage();
    int outerRadius    = radius;
    QPoint origCenter;
    origCenter.setX(x);
    origCenter.setY(y);
    if (outerRadius == 0)
    {
        return origCenter;
    }
    int leftlimit   = origCenter.x() - outerRadius;
    int rightlimit  = origCenter.x() + outerRadius - 1;
    int toplimit    = origCenter.y() - outerRadius;
    int bottomlimit = origCenter.y() + outerRadius - 1;
    QPoint sCenter;

    //-----Check if borders exceed image boundaries
    if (leftlimit < 0 )
    {
        leftlimit = 0;
    }
    if (rightlimit >= img.width())
    {
        rightlimit = img.width()-1;
    }
    if (toplimit < 0 )
    {
        toplimit = 0;
    }
    if ( bottomlimit >= img.height())
    {
        bottomlimit = img.height() -1;
    }

    sCenter.setX(origCenter.x()-leftlimit);
    sCenter.setY(origCenter.y()-toplimit);
    qDebug() << "scenter X = " << sCenter.x();
    qDebug() << "scenter Y = " << sCenter.y();
    return sCenter;
}

DImg LocalAdjustments::getSelection(int index)
{
    return d->sln[index].selection;
}


DImg LocalAdjustments::getDImgSoftSelection(QPoint origCenter, QPoint selectionCenter, int radius)
{
    uint i          = 0;      //loop variables
    uint j          = 0;      //loop variables
    uint x          = 0;      //loop variables
    uint y          = 0;      //loop variables
    int innerRadius = 0.7 * radius;
    int outerRadius = radius;
    int leftlimit   = origCenter.x() - outerRadius;
    int rightlimit  = origCenter.x() + outerRadius - 1;
    int toplimit    = origCenter.y() - outerRadius;
    int bottomlimit = origCenter.y() + outerRadius - 1;
    int width = m_orgImage.width();
    int height = m_orgImage.height();
    QRect crop;
    QSize size;

    //---- Fixing out of bounds for the boudaries -----------------------------

    if (leftlimit < 0 )
    {
        leftlimit = 0;
    }
    if (rightlimit >= width)
    {
        rightlimit = width - 1;
    }
    if (toplimit < 0 )
    {
        toplimit = 0;
    }
    if ( bottomlimit >= height)
    {
        bottomlimit = height - 1;
    }

    //---- Setting up Crop ----------------------------------------------------

    QPoint p1;
    p1.setX(leftlimit);
    p1.setY(toplimit);
    QPoint p2;
    p2.setX(rightlimit);
    p2.setY(bottomlimit);
    crop.setTopLeft(p1);
    crop.setBottomRight(p2);

    //---- Fixing the size of the QSize size -----------------------------------

    size.setHeight(bottomlimit-toplimit+1);
    size.setWidth(rightlimit - leftlimit + 1);
    qDebug() << "Rect : "<<crop;

//    DImg mask(crop.width(), crop.height(), m_orgImage.sixteenBit(), true, 0, true);
    DImg mask(m_orgImage.width() , m_orgImage.height(), m_orgImage.sixteenBit(), true, 0, true);
//    DImg mask = m_orgImage;
    mask.crop(crop);

    //---- Copy the m_orgImage data to mask -----------------------------------

    uint iStart = crop.topLeft().x();
    uint iStop  = crop.topRight().x();
    uint jStart = crop.topLeft().y();
    uint jStop  = crop.bottomRight().y();

    qDebug() << "crop.topLeft().x()    = " << crop.topLeft().x();
    qDebug() << "crop.topRight().x()   = " << crop.topRight().x();
    qDebug() << "crop.topLeft().y()    = " << crop.topLeft().y();
    qDebug() << "crop.bottomLeft().y() = " << crop.bottomLeft().y();

    QFile file("check data.txt");
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&file);
    out << "This file is generated by Qt\n";

    //testing the basic fill command
    DColor fill;
    fill.setRed(255);
    fill.setBlue(255);
    fill.setGreen(255);
    fill.setAlpha(100);

    mask.fill(fill);


    for ( i = iStart , x = 0  ; i <= iStop ; i++ , x++ )
    {
        for ( j = jStart , y = 0 ; j <= jStop; j++ , y++)
        {
            DColor pixel = m_orgImage.getPixelColor(i,j);
            DColor setPixel;
            setPixel.setRed(pixel.red());
            setPixel.setGreen(pixel.green());
            setPixel.setBlue(pixel.blue());
            setPixel.setAlpha(pixel.alpha());
            out << "m_orgImage (i,j) " << i << "\t" << j << "\t" << pixel.red() << "\t" << pixel.green() << "\t" << pixel.blue() << "\t" << pixel.alpha() << "\n";//<< "\t mask : " << x << "\t" << y << "\n";
            //out << "VAlue = " << pixel.red() << "\t" << pixel.green() << "\t" << pixel.blue() << "\t" << pixel.alpha() << "\n";
            mask.setPixelColor(x,y,setPixel);
        }
    }

    mask.save("Trymefirst.png","PNG");
//    return mask;

    //---- Make the circular selection ------------------------------------------

    qDebug() << "selection Center is  : " << selectionCenter;
    int centerx            = selectionCenter.x();
    int centery            = selectionCenter.y();
    uint sz                 = mask.height() * mask.width();
    float diffRadius       = outerRadius - innerRadius;
    int limit              = (m_orgImage.sixteenBit() ? 65535 : 255 );
    //int r;          //red
    //int g;          //green
    //int b;          //blue
    float a;        //alpha value temp
    DColor col;     //to store color of a pixel
    float distance; //to store the value of distance between variables
    float fimg[sz] [4];
    qDebug() << "outerRadius = " << outerRadius;
    qDebug() << "innerRadius = " << innerRadius;
    qDebug() << "Limit is " << limit;

    //--- check for proper data transfer

    qDebug() << " Working properly, all variables initialized";

    x = 0;
    for (i = 0; (i < mask.width()); i++)
    {
        for (j = 0; (j < mask.height()); j++)
        {
            col           = mask.getPixelColor(i, j);
            fimg[x][0] = col.red();
            fimg[x][1] = col.green();
            fimg[x][2] = col.blue();
            fimg[x][3] = col.alpha();
            x++;
        }
    }
    qDebug() << "X is " << x << "; Size is " << sz;

    qDebug() << "mask.height()" << mask.height();
    qDebug() << "mask.width()" << mask.width();

    x = 0;
    for (i = 0; runningFlag() && (i < mask.width()); i++)
    {
        for (j = 0; runningFlag() && (j < mask.height()); j++)
        {
            //if distance > outer radius, make alpha = zero
//            distance=(i-centery)*(i-centery)+(j-centerx)*(j-centerx);
            distance=(j-centery)*(j-centery)+(i-centerx)*(i-centerx);
            distance=qSqrt(distance);
            if (distance < outerRadius)
            {
                if (distance <= innerRadius)
                {
                    //mask.setPixel(j,i,QColor(r,g,b,255).rgba());
                    fimg[x][3] = limit;
                }
                else
                {
                    a = (diffRadius-(distance-innerRadius))/diffRadius;
                    a = a * limit;
                    //**** UNCOMMENT FOR CHECK (next line) ***
                    //                    qDebug() << "Alpha Value = " << a;
                    //mask.setPixel(j,i,QColor(r,g,b,((int)(qFloor(a)))).rgba());
                    fimg[x][3] = a;
                }
            }
            else
            {
                fimg[x][3] = 0;
            }
            out << "Distance  = " << distance << "; alpha value  = " << ((int)qFloor(fimg[x][3])) << "\n";
            x++;
        }
    }
    //Q_ASSERT(check);


    bool sixteenBit=false;
    if (limit == 65535)
    {
        col.setSixteenBit(true);
        sixteenBit = true;
    }
    x = 0;
    for ( i = 0 ; i<mask.width() ; i++ )
    {
        for ( j=0 ; j<mask.height() ; j++ )
        {
            DColor pixel(fimg[x][0],fimg[x][1],fimg[x][2],(int)(qFloor(fimg[x][3])),sixteenBit);
            out << "VAlue = " << fimg[x][0] << " " << fimg[x][1] << " " << fimg[x][2] << " " << (int)(qFloor(fimg[x][3])) << "\n";
            mask.setPixelColor( i, j, pixel);
            x++;
        }
    }
    mask.save("Circular Selection.png","PNG");
    //d->selection = mask;
    //save new file, save it as "testingDImg2.txt"
    return mask;
}

DImg LocalAdjustments::getDImgColorSelection(DImg &selection, QPoint selectionCenter)
{
    //---- Color Selection ---------------------------------------------------
    uint i,j,x;
    uint sz;
    DImg mask = selection;
    sz = mask.width() * mask.height();
    float fimg[sz][4];
    int n = (selectionCenter.y() * mask.width()) + selectionCenter.x();
    int limit = (m_orgImage.sixteenBit() ? 65535 : 255 );
    float difference[sz];

    QFile checkfile("testingColor.txt");
    checkfile.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream print(&checkfile);
    print << "This file is generated by Qt\n";

    //---- Fill the fimg array ------------------------------------------------

    mask.save("colorSelectionInit.png","PNG");
    x = 0;
    for (i = 0; (i < mask.width()); i++)
    {
        for (j = 0; (j < mask.height()); j++)
        {
            if ( (i == selectionCenter.x() ) && (j == selectionCenter.y()))
            {
                qDebug() << "found the correct n = " << x;
            }
            DColor col = mask.getPixelColor(i, j);
            fimg[x][0] = col.red();
            fimg[x][1] = col.green();
            fimg[x][2] = col.blue();
            fimg[x][3] = col.alpha();
            print << fimg[x][0] << "\t" << fimg[x][1] << "\t" << fimg[x][2] << "\t" << fimg[x][3] << "\n";
            x++;
        }
    }

    qDebug() << "========= In Color Selection Function =========";
    qDebug() << "X is " << x << "; Size is " << sz;
    qDebug() << "Limit = " << limit;

    //---- Convert the fimg[sz][4] to values from 0 to 1
    for ( i = 0 ; i < sz ; i ++ )
    {
        for ( j = 0 ; j < 4 ; j++)
        {
            fimg[i][j] = fimg[i][j] / limit;
        }
    }

    //---- set up sixteenBit for sixteenBit data
    bool sixteenBit=false;
    if (limit == 65535)
    {
        sixteenBit = true;
    }

    //-----We convert the whole image to CIELAB -------------------------------
    qDebug() << "n = " << n;
    srgb2lab(fimg,sz);

    //store the lab values in some file (say labme.txt)
    QFile fileLab("labme.txt");
    fileLab.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream outLab(&fileLab);
    for (i=0; i<sz; i++)
    {
        outLab << fimg[i][0] << "\t" << fimg[i][1] << "\t" << fimg[i][2] << "\t" << fimg[i][3]<<"\n";
    }
    fileLab.close();
    qDebug() << "After conversion:";

    //-----Comparing with Center Pixel ----------------------------------------
    //Here we consider the central pixel, i.e. center, we take the color of those pixels
    float reference[4];
    //test check of the selectionCenter values
    qDebug() << "Selection Center X = " << selectionCenter.x();
    qDebug() << "Selection Center Y = " << selectionCenter.y();
    DColor col = mask.getPixelColor(selectionCenter.x(), selectionCenter.y());
    qDebug() << "col value : " << col.red() << "\t" << col.green() << "\t" << col.blue() << "\t" << col.alpha();

    for ( i = 0 ; i < 4 ; i++)
    {
        reference[i] = fimg[n][i];
    }
    qDebug() << "New Reference Values : ";
    qDebug() << reference[0] << "\t" << reference[1] << "\t" << reference[2] << "\t" << reference[3];
    colorDifference(fimg,reference,difference,sz);

    //-----We convert the whole Image back to SRGB-----------------------------

    lab2srgb(fimg,sz);
    for ( i=0; i<sz; i++)
    {
        for (j=0 ; j<4 ; j++)
        {
            fimg[i][j] = fimg[i][j] * limit;
        }
    }
    x=0;
    for (i=0;i<mask.width(); i++)
    {
        for(j=0;j<mask.height();j++)
        {
//            c=QColor::fromRgba(selection.pixel(j,i));
//            for (k = 0; k < 3 ; k++)
//            {
//                fimg[x][k] = fimg[x][k] * limit;
//            }
            if (difference[x]<0.2)
            {
                fimg[x][3] = fimg[x][3] * (0.2 - difference[x]) * 5;
//                selection.setPixel(j,i,QColor(((int)(fimg[x][0]*limit)),((int)(fimg[x][1]*limit)),((int)(fimg[x][2]*limit)),(fimg[x][3]*limit*(0.2-difference[x])*5)).rgba());
            }
            else
            {
                fimg[x][3] = 0;
//                selection.setPixel(j,i,QColor(((int)(fimg[x][0]*limit)),((int)(fimg[x][1]*limit)),((int)(fimg[x][2]*limit)),0).rgba());
            }
            print << "difference[x] = " << difference[x] << "\t\tAlpha value multiplier" << (limit * (0.2 - difference[x]) * 5) << "\n";
            x++;
        }
    }

    // ---- convert fimg to DImg
    //we will refill mask

    x = 0;
    for ( i = 0 ; i<mask.width() ; i++ )
    {
        for ( j=0 ; j<mask.height() ; j++ )
        {
            DColor pixel((int)fimg[x][0], (int) fimg[x][1],(int) fimg[x][2],(int)(qFloor(fimg[x][3])),sixteenBit);
            print << "VAlue = " << (int) fimg[x][0] << "\t" << (int) fimg[x][1] << "\t" << (int) fimg[x][2] << "\t" << (int)(qFloor(fimg[x][3])) << "\n";
            mask.setPixelColor( i, j, pixel);
            x++;
        }
    }

    // ---- Test Save ---------------------------------------------------------
    bool istrue=mask.save("Localadj DImg Circular.png","PNG");
    qDebug() << "Check for good save : " << istrue;

    return mask;
}



DImg LocalAdjustments::createDImgLayer(DImg &selection, QPoint selectionCenter, QPoint origCenter)
{
    //this would create the DImg in the size of the m_orgImage, and keep the data
    //of the Selection in the correct location
    bool sixteenBit = m_orgImage.sixteenBit();
    DImg layer(m_orgImage.width() , m_orgImage.height(), m_orgImage.sixteenBit(), true, 0, true);
    uint i,j,x,y;
    DColor col(0,0,0,0,sixteenBit);
    //---- initialize layer to null and transparent ---------------------------
    for( x=0; x<layer.width(); x++)
    {
        for ( y=0; y<layer.height(); y++)
        {
            layer.setPixelColor(x,y,col);
        }
    }

    //---- Placing the selection in the correct position of the layer ---------
    uint leftlimit;
    uint rightlimit;
    uint toplimit;
    uint bottomlimit;
    leftlimit = origCenter.x() - selectionCenter.x();
    toplimit  = origCenter.y() - selectionCenter.y();
    rightlimit = leftlimit + d->selection.width();
    bottomlimit = toplimit + d->selection.height();

    for(x=leftlimit, i=0 ; x < rightlimit ; x++, i++ )
    {
        for (y=toplimit, j=0 ; y < bottomlimit ; y++, j++)
        {
            layer.setPixelColor(x,y,selection.getPixelColor(i,j));
        }
    }
    return layer;
}

DImg LocalAdjustments::createDImgLayer(int index)
{
    //this would create the DImg in the size of the m_orgImage, and data of selection
    bool sixteenBit = m_orgImage.sixteenBit();
    DImg layer(m_orgImage.width() , m_orgImage.height(), m_orgImage.sixteenBit(), true, 0, true);
    uint i,j,x,y;
    DColor col(0,0,0,0,sixteenBit);
    //---- initialize layer to null and transparent ---------------------------
    for( x=0; x<layer.width(); x++)
    {
        for ( y=0; y<layer.height(); y++)
        {
            layer.setPixelColor(x,y,col);
        }
    }

    //---- Placing the selection in the correct position of the layer ---------
    uint leftlimit;
    uint rightlimit;
    uint toplimit;
    uint bottomlimit;
    leftlimit = d->sln[index].center.x() - d->sln[index].selectionCenter.x();
    toplimit  = d->sln[index].center.y() - d->sln[index].selectionCenter.y();
    rightlimit = leftlimit + d->sln[index].selection.width();
    bottomlimit = toplimit + d->sln[index].selection.height();

    for(x=leftlimit, i=0 ; x < rightlimit ; x++, i++ )
    {
        for (y=toplimit, j=0 ; y < bottomlimit ; y++, j++)
        {
            layer.setPixelColor(x,y,d->sln[index].modSelection.getPixelColor(i,j));
        }
    }
    return layer;
}


QImage LocalAdjustments::createLayer(QImage selection)
{
    QImage img             = m_orgImage.copyQImage();
    QPoint origCenter;
    QPoint selectionCenter = d->selectionCenter; 
    origCenter.setX(d->centerX);
    origCenter.setY(d->centerY);
    QImage layer;
    layer=QImage(img.size(),QImage::Format_ARGB32);
    int i;
    int j;
    int x;
    int y;
    //---- initialize layer to null and transparent ---------------------------
    for( x=0; x<layer.width(); x++)
    {
        for ( y=0; y<layer.height(); y++)
        {
            layer.setPixel(x,y,qRgba(0,0,0,0));
        }
    }
    //---- Placing the selection in the correct position of the layer ---------
    int leftlimit;
    int rightlimit;
    int toplimit;
    int bottomlimit;
    leftlimit = origCenter.x() - selectionCenter.x();
    toplimit  = origCenter.y() - selectionCenter.y();
    rightlimit = leftlimit + selection.width();
    bottomlimit = toplimit + selection.height();
    
    for(x=leftlimit, i=0 ; x < rightlimit ; x++, i++ )
    {
        for (y=toplimit, j=0 ; y < bottomlimit ; y++, j++)
        {
            layer.setPixel(x,y,selection.pixel(i,j));
        }
    }
    return layer;
}


void LocalAdjustments::srgb2lch(float fimg[][4], int size)
{
    float c, h;
    int i;
    srgb2lab(fimg,size);
    
    QFile file("/home/sayantan/WORK/samples/lab.txt");
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&file);
    for (i=0; i<size; i++)
    {
        out << fimg[i][0] << "\t" << fimg[i][1] << "\t" << fimg[i][2] << "\t" << fimg[i][3]<<"\n";
    }
    file.close();
    for(int i=0; i<size; i++)
    {
        c=qSqrt((fimg[i][1]*fimg[i][1]) + (fimg[i][2]*fimg[i][2])); //chroma
        h=qAtan(fimg[i][2]/fimg[i][1]);                             //hue
        fimg[i][1]=c;
        fimg[i][2]=h;
    }
}

void LocalAdjustments::srgb2xyz(float fimg[][4], int size)
{
    // fimg in [0:1], sRGB
    float x, y, z;
    
    for (int i = 0; i < size; ++i)
    {
        // scaling and gamma correction (approximate)
        fimg[i][0] = pow(fimg[i][0], (float)2.2);
        fimg[i][1] = pow(fimg[i][1], (float)2.2);
        fimg[i][2] = pow(fimg[i][2], (float)2.2);
        
        // matrix RGB -> XYZ, with D65 reference white (www.brucelindbloom.com)
        x = 0.412424  * fimg[i][0] + 0.357579 * fimg[i][1] + 0.180464  * fimg[i][2];
        y = 0.212656  * fimg[i][0] + 0.715158 * fimg[i][1] + 0.0721856 * fimg[i][2];
        z = 0.0193324 * fimg[i][0] + 0.119193 * fimg[i][1] + 0.950444  * fimg[i][2];
        
        //      x = 0.412424 * fimg[i][0] + 0.212656  * fimg[i][1] + 0.0193324 * fimg[i][2];
        //      y = 0.357579 * fimg[i][0] + 0.715158  * fimg[i][1] + 0.119193  * fimg[i][2];
        //      z = 0.180464 * fimg[i][0] + 0.0721856 * fimg[i][1] + 0.950444  * fimg[i][2];
        
        fimg[i][0] = x;
        fimg[i][1] = y;
        fimg[i][2] = z;
    }
}

void LocalAdjustments::srgb2lab(float fimg[][4], int size)
{
    float l, a, b;
    
    srgb2xyz(fimg, size);
    
    for (int i = 0; i < size; ++i)
    {
        // reference white
        fimg[i][0] /= 0.95047F;
        
        //fimg[i][1] /= 1.00000;          // (just for completeness)
        
        fimg[i][2] /= 1.08883F;
        
        // scale
        if (fimg[i][0] > 216.0 / 24389.0)
        {
            fimg[i][0] = pow(fimg[i][0], (float)(1.0 / 3.0));
        }
        else
        {
            fimg[i][0] = (24389.0 * fimg[i][0] / 27.0 + 16.0) / 116.0;
        }
        
        if (fimg[i][1] > 216.0 / 24389.0)
        {
            fimg[i][1] = pow(fimg[i][1], (float)(1.0 / 3.0));
        }
        else
        {
            fimg[i][1] = (24389 * fimg[i][1] / 27.0 + 16.0) / 116.0;
        }
        
        if (fimg[i][2] > 216.0 / 24389.0)
        {
            fimg[i][2] = (float)pow(fimg[i][2], (float)(1.0 / 3.0));
        }
        else
        {
            fimg[i][2] = (24389.0 * fimg[i][2] / 27.0 + 16.0) / 116.0;
        }
        
        l          = 116 * fimg[i][1]  - 16;
        a          = 500 * (fimg[i][0] - fimg[i][1]);
        b          = 200 * (fimg[i][1] - fimg[i][2]);
        fimg[i][0] = l / 116.0; // + 16 * 27 / 24389.0;
        fimg[i][1] = a / 500.0 / 2.0 + 0.5;
        fimg[i][2] = b / 200.0 / 2.2 + 0.5;
        
        if (fimg[i][0] < 0)
        {
            fimg[i][0] = 0;
        }
    }
}

void LocalAdjustments::xyz2srgb(float fimg[][4], int size)
{
    float r, g, b;
    
    for (int i = 0; i < size; ++i)
    {
        // matrix RGB -> XYZ, with D65 reference white (www.brucelindbloom.com)
        r = 3.24071   * fimg[i][0] - 1.53726  * fimg[i][1] - 0.498571  * fimg[i][2];
        g = -0.969258 * fimg[i][0] + 1.87599  * fimg[i][1] + 0.0415557 * fimg[i][2];
        b = 0.0556352 * fimg[i][0] - 0.203996 * fimg[i][1] + 1.05707   * fimg[i][2];
        
        
        //      r =  3.24071  * fimg[i][0] - 0.969258  * fimg[i][1]
        //           + 0.0556352 * fimg[i][2];
        //      g = -1.53726  * fimg[i][0] + 1.87599   * fimg[i][1]
        //           - 0.203996  * fimg[i][2];
        //      b = -0.498571 * fimg[i][0] + 0.0415557 * fimg[i][1]
        //           + 1.05707   * fimg[i][2];
        
        // scaling and gamma correction (approximate)
        r = r < 0 ? 0 : pow(r, (float)(1.0 / 2.2));
        g = g < 0 ? 0 : pow(g, (float)(1.0 / 2.2));
        b = b < 0 ? 0 : pow(b, (float)(1.0 / 2.2));
        
        fimg[i][0] = r;
        fimg[i][1] = g;
        fimg[i][2] = b;
    }
}

void LocalAdjustments::lab2srgb(float fimg[][4], int size)
{
    float x, y, z;
    
    for (int i = 0; i < size; ++i)
    {
        // convert back to normal LAB
        fimg[i][0] = (fimg[i][0] - 0 * 16 * 27 / 24389.0) * 116;
        fimg[i][1] = (fimg[i][1] - 0.5) * 500 * 2;
        fimg[i][2] = (fimg[i][2] - 0.5) * 200 * 2.2;
        
        // matrix
        y = (fimg[i][0] + 16) / 116;
        z = y - fimg[i][2] / 200.0;
        x = fimg[i][1] / 500.0 + y;
        
        // scale
        if (x * x * x > 216 / 24389.0)
        {
            x = x * x * x;
        }
        else
        {
            x = (116 * x - 16) * 27 / 24389.0;
        }
        
        if (fimg[i][0] > 216 / 27.0)
        {
            y = y * y * y;
        }
        else
        {
            //y = fimg[i][0] * 27 / 24389.0;
            y = (116 * y - 16) * 27 / 24389.0;
        }
        
        if (z * z * z > 216 / 24389.0)
        {
            z = z * z * z;
        }
        else
        {
            z = (116 * z - 16) * 27 / 24389.0;
        }
        
        // white reference
        fimg[i][0] = x * 0.95047;
        fimg[i][1] = y;
        fimg[i][2] = z * 1.08883;
    }
    
    xyz2srgb(fimg, size);
}

void LocalAdjustments::colorDifference(float fimg[][4], float reference[4], float* difference, int size)
{
    //This code will be on the function of CIE94
    //We will test it on the basis that
    //wiki link: http://en.wikipedia.org/wiki/Color_difference#CIE94
    //stackoverflow link: http://stackoverflow.com/questions/6630599/are-there-known-implementations-of-the-ciede2000-or-cie94-delta-e-color-differen
    
    //fimg contains all the pixels, so for each of the pixel, we consider that the color of the pixel is Lab and not Lch, we will calculate the rest of the data from this Lab. :) If so required, we would also convert from Lab to Lch
    float dl       = 0;   //delta L (Lightness)
    float cab       = 0;
    float c1       = 0;   //chroma 1
    float c2       = 0;   //chroma 2
    float hab      = 0;  //Hypotenuse
//    float dab      = 0;  //delta Lab
    float deltaE   = 0;
    const float kl = 1;
    const float k1 = 0.045;
    const float k2 = 0.015;
    float sl       = 1;
    float sc       = 1;
    float sh       = 1;
    int i;
    for(i=0; i<size; i++)
    {
        dl = reference[0] - fimg[i][0];
        c2 = qSqrt(fimg[i][1]*fimg[i][1] + fimg[i][2]*fimg[i][2]);
        c1 = qSqrt(reference[1]*reference[1]+reference[2]*reference[2]);
        cab = c1 - c2;
        //dab = qSqrt(qPow((reference[0] - fimg[i][0]),2) + qPow((reference[1] - fimg[i][1]),2) + qPow((reference[2] - fimg[i][2]),2));
        hab = qSqrt(qPow((reference[1] - fimg[i][1]),2) + qPow((reference[2] - fimg[i][2]),2) - qPow(cab,2));
        sc = 1 +k1*c1;
        sh = 1 +k2*c1;
        deltaE = qSqrt(qPow( ( dl / (kl*sl ) ), 2 ) + qPow((cab / sc), 2) + qPow((hab / sh), 2));
        difference[i]=deltaE;
    }
    
}

void  LocalAdjustments::apply8bitSelection(DImg  &destination, DImg &source)
{
    //now we create a layer from the selection, and paste the layer on the image
    //works for 8 bit only
    qDebug() << "In apply8bitSelection Function";
    QImage mask  = destination.copyQImage();
    QImage img   = source.copyQImage();

    QPainter painter(&img);
    QPointF point;
    point.setX(0);
    point.setY(0);
    painter.drawImage(point,mask);

    DImg result(img);
    source = result;
    qDebug() << "Done with filling source with the required output";
    //return result;
}

void LocalAdjustments::apply16bitSelection(DImg &destination, DImg &source)
{

    //---- Make a new result DIMG, with the one on another one ----------------

    DColorComposer* composer = DColorComposer::getComposer(DColorComposer::PorterDuffNone);
//    if ( m_orgImage.sixteenBit() )
//    {
//        destination.convertToSixteenBit();
//    }

    //---- Make the src destination premultiplied ----------------------------------

    uint i,j;

    for (i = 0 ; i < destination.width() ; j++)
    {
        for ( j = 0 ; j< destination.width() ; j++)
        {
            DColor pixel = destination.getPixelColor(i,j);
            pixel.premultiply();
            destination.setPixelColor(i,j,pixel);
        }
    }

    //DImg output = m_orgImage();
    //---- make a deep copy of m_orgImage to DImg output
    DImg output(m_orgImage.width() , m_orgImage.height(), m_orgImage.sixteenBit(), true, 0, true);

    for (i = 0; i < m_orgImage.width() ; i++ )
    {
        for ( j = 0 ; j < m_orgImage.height() ; j++)
        {
            DColor pixel = m_orgImage.getPixelColor(i,j);
            DColor setPixel;
            setPixel.setRed(pixel.red());
            setPixel.setGreen(pixel.green());
            setPixel.setBlue(pixel.blue());
            setPixel.setAlpha(pixel.alpha());
            output.setPixelColor(i,j,setPixel);
        }
    }

    output.bitBlendImage(composer, &destination, 0, 0, destination.width(), destination.height(),0,0 );
    source = output;
}


//For test purposes only, do delete when tests are done.
DImg LocalAdjustments::applyDImgSelection(int index)
{
    d->finalImage = m_orgImage;
    DImg layer = createDImgLayer(index);
    //apply the selection one by one to the main image
    if (m_orgImage.sixteenBit())
    {
        qDebug() << "Sorry 16bit application is incomplete";
        kDebug() << "Sorry 16bit application is incomplete";
    }
    else
    {
        apply8bitSelection(layer,d->finalImage);
    }
    return d->finalImage;
}

//make private when tests are done.
void LocalAdjustments::applyAllSelections()
{
    int i;
    qDebug() << "In applyAllSelections() functions";
    d->finalImage = m_orgImage;
    for (i = 0 ; i < d->count ; i ++)
    {
        //apply the selection one by one to the main image
        if (m_orgImage.sixteenBit())
        {
            qDebug() << "Sorry 16bit application is incomplete";
            kDebug() << "Sorry 16bit application is incomplete";
            DImg layer = createDImgLayer(i);
            apply16bitSelection( layer ,d->finalImage);
        }
        else
        {
            qDebug() << "creating layer for index = " << i;
            DImg layer = createDImgLayer(i);
            qDebug() << "Calling 8bitSelection for index = " << i;
            apply8bitSelection( layer ,d->finalImage);
            qDebug() << "Done with 8bit Selection of index = " << i << "\n\n";
        }
    }
}

DImg LocalAdjustments::getFinalImage()
{
    //apply all selections
    //return the final image
    qDebug() << "In the getFinalImage Function";
    applyAllSelections();
    qDebug() << "Done with all calculations, d->final image is filled sucessfully";
    return (d->finalImage);
}

void LocalAdjustments::changeAllRGBA()
{
    int i;
    for( i = 0 ; i < d->count ; i++)
    {
        changeSingleRGBA(i);
    }
}

DImg LocalAdjustments::returnOrigImage()
{
    return m_orgImage;
}

void LocalAdjustments::changeSingleRGBA(int index)
{
    //changes the selection to the given parameters.

    //-- Check for 8bit image or 16bit image, the rest of the code depends on that
    int intR;
    int intG;
    int intB;
    int intA;
    int tempR;
    int tempG;
    int tempB;
    int tempA;
    int limit;
    uint i;
    uint j;
    DColor col;
    float sixteenBit = m_orgImage.sixteenBit();
//    qDebug() << "In actual function : " << r << "\t" <<  g << "\t" <<  b << "\t" << a;

    d->sln[index].selection.save("Selectionb4Mod.png","PNG");

    //---- alpha check --------------------------------------------------------

    if (d->sln[index].alpha > 0)
    {
        d->sln[index].alpha = 0;
    }

    if (sixteenBit)
    {
        limit = 65535;
        intR  = (int) (( d->sln[index].red   / 100.0 ) * limit);
        intB  = (int) (( d->sln[index].blue  / 100.0 ) * limit);
        intG  = (int) (( d->sln[index].green / 100.0 ) * limit);
        intA  = (int) (( d->sln[index].alpha / 100.0 ) * limit);
    }
    else
    {
        limit = 255;
        intR  = (int) (( d->sln[index].red   / 100.0 ) * limit);
        intB  = (int) (( d->sln[index].blue  / 100.0 ) * limit);
        intG  = (int) (( d->sln[index].green / 100.0 ) * limit);
        intA  = (int) (( d->sln[index].alpha / 100.0 ) * limit);
    }

//    qDebug() << "intR = " << intR;
//    qDebug() << "intG = " << intG;
//    qDebug() << "intB = " << intB;
//    qDebug() << "intA = " << intA;


    QFile file("check color mod.txt");
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&file);
    out << "This file is generated by Qt\n";

    //d->sln[index].modSelection = d->sln[index].selection;

    //---- Create a deep copy to create a copy of selection to modselection

    DImg layer(d->sln[index].selection.width() , d->sln[index].selection.height(), m_orgImage.sixteenBit(), true, 0, true);

    for ( i = 0 ; i < layer.width() ; i++)
    {
        for ( j = 0 ; j < layer.height() ; j++)
        {
            col = d->sln[index].selection.getPixelColor(i,j);
            layer.setPixelColor(i,j,col);
        }
    }

    //-- Apply the values on every pixel with a boundary check

    for ( i = 0 ; i < layer.width() ; i++)
    {
        for ( j = 0 ; j < layer.height() ; j++)
        {
            col = layer.getPixelColor(i,j);
            tempR = col.red();
            tempG = col.green();
            tempB = col.blue();
            tempA = col.alpha();

            out << "Previous \t\t " << tempR << "\t" << tempG << "\t" << tempB << "\t" << tempA << "\t\t";

            tempR = tempR + intR;
            tempG = tempG + intG;
            tempB = tempB + intB;
            tempA = tempA + intA;

            out << "Current \t\t " <<  tempR << "\t" << tempG << "\t" << tempB << "\t" << tempA << "\t\t\n";

            if (tempR > limit)
            {
                tempR = limit;
            }
            if (tempB > limit)
            {
                tempB = limit;
            }
            if (tempG > limit)
            {
                tempG = limit;
            }
            if (tempA > limit)
            {
                tempA = limit;
            }
            if (tempR < 0)
            {
                tempR = 0;
            }
            if (tempG < 0)
            {
                tempG = 0;
            }
            if (tempB < 0)
            {
                tempB = 0;
            }
            if (tempA < 0)
            {
                tempA = 0;
            }
            col.setRed(tempR);
            col.setBlue(tempB);
            col.setGreen(tempG);
            col.setAlpha(tempA);
            layer.setPixelColor(i,j,col);
        }
    }
    file.close();
    d->sln[index].selection.save("SelectionAfterMod.png","PNG");
    d->sln[index].modSelection = layer;

}

DImg LocalAdjustments::getModifiedSelection(int index)
{
    changeSingleRGBA(index);
    return d->sln[index].modSelection;
}

} //namespace Digikam
