/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-03-27
 * Description : Threaded image filter to fix hot pixels
 *
 * Copyright (C) 2005-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2005-2006 by Unai Garro <ugarro at users dot sourceforge dot net>
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

#include "hotpixelfixer.h"

// C++ includes

#include <cmath>
#include <cstdlib>

// Qt includes

#include <QList>
#include <QColor>
#include <QRegExp>
#include <QStringList>

// Local includes

#include "dimg.h"

#ifdef HAVE_FLOAT_H
#if HAVE_FLOAT_H
# include <float.h>
#endif
#endif

#ifndef DBL_MIN
# define DBL_MIN 1e-37
#endif
#ifndef DBL_MAX
# define DBL_MAX 1e37
#endif

namespace Digikam
{

HotPixelFixer::HotPixelFixer(QObject* const parent)
    : DImgThreadedFilter(parent)
{
    m_interpolationMethod = TWODIM_DIRECTION;
    initFilter();
}

HotPixelFixer::HotPixelFixer(Digikam::DImg* const orgImage, QObject* const parent, const QList<HotPixel>& hpList,
                             int interpolationMethod)
    : Digikam::DImgThreadedFilter(orgImage, parent, QLatin1String("HotPixels"))
{
    m_hpList              = hpList;
    m_interpolationMethod = interpolationMethod;

    initFilter();
}

HotPixelFixer::~HotPixelFixer()
{
    cancelFilter();
}

Digikam::FilterAction HotPixelFixer::filterAction()
{
    DefaultFilterAction<HotPixelFixer> action;
    action.addParameter(QLatin1String("interpolationMethod"), m_interpolationMethod);

    foreach(const HotPixel& hp, m_hpList)
    {
        QString hpString = QString::fromUtf8("%1-%2x%3-%4x%5").arg(hp.luminosity)
                                                              .arg(hp.rect.x()).arg(hp.rect.y())
                                                              .arg(hp.rect.width()).arg(hp.rect.height());
        action.addParameter(QLatin1String("hotPixel"), hpString);
    }

    return action;
}

void HotPixelFixer::readParameters(const Digikam::FilterAction& action)
{
    m_interpolationMethod = action.parameter(QLatin1String("interpolationMethod")).toInt();
    QRegExp exp(QLatin1String("(\\d+)-(\\d+)x(\\d+)-(\\d+)x(\\d+)"));

    foreach(const QVariant& var, action.parameters().values(QLatin1String("hotPixel")))
    {
        if (exp.exactMatch(var.toString()))
        {
            HotPixel hp;
            hp.luminosity = exp.cap(1).toInt();
            hp.rect       = QRect(exp.cap(2).toInt(), exp.cap(3).toInt(),
                                  exp.cap(4).toInt(), exp.cap(5).toInt());
            m_hpList << hp;
        }
    }
}

void HotPixelFixer::filterImage()
{
    QList <HotPixel>::ConstIterator it;
    QList <HotPixel>::ConstIterator end(m_hpList.constEnd());

    for (it = m_hpList.constBegin() ; it != end ; ++it)
    {
        HotPixel hp = *it;
        interpolate(m_orgImage, hp, m_interpolationMethod);
    }

    m_destImage = m_orgImage;
}

// Interpolates a pixel block
void HotPixelFixer::interpolate(Digikam::DImg& img, HotPixel& hp, int method)
{
    const int xPos = hp.x();
    const int yPos = hp.y();
    bool sixtBits  = img.sixteenBit();

    // Interpolate pixel.
    switch (method)
    {
        case AVERAGE_INTERPOLATION:
        {
            // We implement the bidimendional one first.
            // TODO: implement the rest of directions (V & H) here

            //case twodim:
            // {
            int sum_weight = 0;
            double vr      = 0.0, vg = 0.0, vb = 0.0;
            int x, y;
            Digikam::DColor col;

            for (x = xPos; x < xPos+hp.width(); ++x)
            {
                if (validPoint(img,QPoint(x,yPos-1)))
                {
                    col=img.getPixelColor(x,yPos-1);
                    vr += col.red();
                    vg += col.green();
                    vb += col.blue();
                    ++sum_weight;
                }

                if (validPoint(img,QPoint(x,yPos+hp.height())))
                {
                    col=img.getPixelColor(x,yPos+hp.height());
                    vr += col.red();
                    vg += col.green();
                    vb += col.blue();
                    ++sum_weight;
                }
            }

            for (y = yPos; y < hp.height(); ++y)
            {
                if (validPoint(img,QPoint(xPos-1,y)))
                {
                    col=img.getPixelColor(xPos,y);
                    vr += col.red();
                    vg += col.green();
                    vb += col.blue();
                    ++sum_weight;
                }

                if (validPoint(img,QPoint(xPos+hp.width(),y)))
                {
                    col=img.getPixelColor(xPos+hp.width(),y);
                    vr += col.red();
                    vg += col.green();
                    vb += col.blue();
                    ++sum_weight;
                }
            }

            if (sum_weight > 0)
            {
                vr /= (double)sum_weight;
                vg /= (double)sum_weight;
                vb /= (double)sum_weight;

                for (x = 0; x < hp.width(); ++x)
                {
                    for (y = 0; y < hp.height(); ++y)
                    {
                        if (validPoint(img,QPoint(xPos+x,yPos+y)))
                        {
                            int alpha = sixtBits ? 65535 : 255;
                            int ir = (int )round(vr), ig = (int) round(vg), ib = (int) round(vb);
                            img.setPixelColor(xPos+x,yPos+y,Digikam::DColor(ir,ig,ib,alpha,sixtBits));
                        }
                    }
                }
            }

            break;
        }

        case LINEAR_INTERPOLATION:
            weightPixels(img, hp, LINEAR_INTERPOLATION, TWODIM_DIRECTION, sixtBits ? 65535: 255);
            break;

        case QUADRATIC_INTERPOLATION:
            weightPixels(img, hp, QUADRATIC_INTERPOLATION, TWODIM_DIRECTION, sixtBits ? 65535 : 255);
            break;

        case CUBIC_INTERPOLATION:
            weightPixels(img, hp, CUBIC_INTERPOLATION, TWODIM_DIRECTION, sixtBits ? 65535 : 255);
            break;
    }
}

void HotPixelFixer::weightPixels(Digikam::DImg& img, HotPixel& px, int method, Direction dir, int maxComponent)
{
    //TODO: implement direction here too

    for (int iComp = 0; iComp < 3; ++iComp)
    {
        // Obtain weight data block.

        Weights w;
        int polynomeOrder=-1;

        switch (method)
        {
            case AVERAGE_INTERPOLATION:  // Gilles: to prevent warnings from compiler.
                break;
            case LINEAR_INTERPOLATION:
                polynomeOrder=1;
                break;
            case QUADRATIC_INTERPOLATION:
                polynomeOrder=2;
                break;
            case CUBIC_INTERPOLATION:
                polynomeOrder=3;
                break;
        }

        if (polynomeOrder<0)
        {
            return;
        }

        // In the one-dimensional case, the width must be 1,
        // and the size must be stored in height

        w.setWidth(dir == TWODIM_DIRECTION ? px.width() : 1);
        w.setHeight(dir == HORIZONTAL_DIRECTION ? px.width() : px.height());
        w.setPolynomeOrder(polynomeOrder);
        w.setTwoDim(dir == TWODIM_DIRECTION);

        //TODO: check this, it must not recalculate existing calculated weights
        //for now I don't think it is finding the duplicates fine, so it uses
        //the previous one always...

        //if (mWeightList.find(w)==mWeightList.end())
        //{
        w.calculateWeights();

        //    mWeightList.append(w);

        //}

        // Calculate weighted pixel sum.
        for (int y = 0; y<px.height(); ++y)
        {
            for (int x = 0; x < px.width(); ++x)
            {
                if (validPoint (img, QPoint(px.x()+x,px.y()+y)))
                {
                    double sum_weight = 0.0, v = 0.0;
                    size_t i;

                    for (i = 0; i < (size_t)w.positions().count(); ++i)
                    {
                        // In the one-dimensional case, only the y coordinate is used.
                        const int xx = px.x()+(dir == VERTICAL_DIRECTION ? x :
                                               dir== HORIZONTAL_DIRECTION ? w.positions().at(i).y() : w.positions().at(i).x());
                        const int yy = px.y()+(dir == HORIZONTAL_DIRECTION ? y :
                                               w.positions().at(i).y());

                        if (validPoint (img,QPoint(xx, yy)))
                        {
                            //TODO: check this. I think it is broken
                            double weight;

                            if (dir==VERTICAL_DIRECTION)
                            {
                                weight = w[i][y][0];
                            }
                            else if (dir==HORIZONTAL_DIRECTION)
                            {
                                weight=w[i][0][x];
                            }
                            else
                            {
                                weight=w[i][y][x];
                            }

                            if (iComp==0)
                            {
                                v += weight * img.getPixelColor(xx, yy).red();
                            }
                            else if (iComp==1)
                            {
                                v += weight * img.getPixelColor(xx, yy).green();
                            }
                            else
                            {
                                v += weight * img.getPixelColor(xx, yy).blue();
                            }

                            sum_weight += weight;
                        }
                    }

                    Digikam::DColor color=img.getPixelColor(px.x()+x,px.y()+y);
                    int component;

                    if (fabs (v) <= DBL_MIN)
                    {
                        component=0;
                    }
                    else if (sum_weight >= DBL_MIN)
                    {
                        component=(int) (v/sum_weight);

                        //Clamp value
                        if (component<0)
                        {
                            component=0;
                        }

                        if (component>maxComponent)
                        {
                            component=maxComponent;
                        }
                    }
                    else if (v >= 0.0)
                    {
                        component=maxComponent;
                    }
                    else
                    {
                        component=0;
                    }

                    if (iComp==0)
                    {
                        color.setRed(component);
                    }
                    else if (iComp==1)
                    {
                        color.setGreen(component);
                    }
                    else
                    {
                        color.setBlue(component);
                    }

                    img.setPixelColor(px.x()+x,px.y()+y,color);
                }
            }
        }
    }
}

}  // namespace Digikam
