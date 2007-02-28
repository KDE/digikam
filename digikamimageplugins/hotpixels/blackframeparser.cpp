/* ============================================================
 * Authors: Unai Garro <ugarro at users dot sourceforge dot net>
 * Date   : 2005-03-27
 * Description : 
 * 
 * Copyright 2005-2006 by Unai Garro
 *
 * Part of the algorithm for finding the hot pixels was based on
 * the code of jpegpixi, which was released under the GPL license,
 * and is Copyright (C) 2003, 2004 Martin Dickopp
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
 * ============================================================*/

// Denominator for relative quantities. 
#define DENOM (DENOM_SQRT * DENOM_SQRT)

// Square root of denominator for relative quantities. 
#define DENOM_SQRT 10000

// Convert relative to absolute numbers. Care must be taken not to overflow integers. 
#define REL_TO_ABS(n,m) \
    ((((n) / DENOM_SQRT) * (m) + ((n) % DENOM_SQRT) * (m) / DENOM_SQRT) / DENOM_SQRT)

// QT includes.

#include <qimage.h>
#include <qstringlist.h>

// Local includes.

#include "blackframeparser.h"
#include "blackframeparser.moc"

namespace DigikamHotPixelsImagesPlugin
{

BlackFrameParser::BlackFrameParser()
                : QObject()
{
}

BlackFrameParser::~BlackFrameParser()
{
}

void BlackFrameParser::parseHotPixels(QString file)
{
    parseBlackFrame(file);
    
    /*
    mOutputString ="";
    
    //First, lets run jpeghotp on the blackframe file
    KProcess *proc = new KProcess;
    connect(proc, SIGNAL(processExited(KProcess*)),this, SLOT(processed(KProcess*)));
    connect(proc, SIGNAL(receivedStdout(KProcess*, char*,int)),this,SLOT(HotPData(KProcess*, char*,int)));
    connect(proc, SIGNAL(receivedStderr(KProcess*, char*,int)),this,SLOT(HotPData(KProcess*, char*,int)));
    *proc << "jpeghotp";
    *proc << file;
    proc->start( KProcess::NotifyOnExit, KProcess::Stdout);    
    */
}

void BlackFrameParser::parseBlackFrame(KURL url)
{
    //Initialize the data buffer
    mData.resize(0);

    //And open the file
    
    KIO::TransferJob *job = KIO::get(url, false, false);
    connect(job, SIGNAL(data( KIO::Job*, const QByteArray&)),
            this, SLOT( blackFrameDataArrived( KIO::Job *, const QByteArray& )));
    
    connect(job, SIGNAL(result(KIO::Job* )), 
            this, SLOT(slotResult(KIO::Job*)));
}

void BlackFrameParser::parseBlackFrame(QImage& img)
{
    mImage=img;
    blackFrameParsing();
}

void BlackFrameParser::blackFrameDataArrived(KIO::Job*,const QByteArray& data)
{
    uint size=mData.size();
    uint dataSize=data.size();
    mData.resize(size+dataSize);
    memcpy(mData.data()+size,data.data(),dataSize);
}

void BlackFrameParser::slotResult(KIO::Job*)
{
    blackFrameParsing(true);
}

// Parses black frames

void BlackFrameParser::blackFrameParsing(bool useData)
{
    //First we create a QImage out of the file data if we are using it
    if (useData) 
    {
        mImage.loadFromData(mData);
    }
    // Now find the hot pixels and store them in a list
    QValueList<HotPixel> hpList;
    
    for (int y=0 ; y < mImage.height() ; ++y)
    {
        for (int x=0 ; x < mImage.width() ; ++x)
        {
            //Get each point in the image
            QRgb pixrgb=mImage.pixel(x,y);
            QColor color; color.setRgb(pixrgb);
            
            // Find maximum component value.
            int maxValue;
            int threshold=DENOM/10;
            const int threshold_value = REL_TO_ABS(threshold,255);
            maxValue=(color.red()>color.blue()) ? color.red() : color.blue();
            if (color.green()>maxValue) maxValue=color.green();

            // If the component is bigger than the threshold, add the point
            if (maxValue > threshold_value)
            {
                HotPixel point;
                point.rect=QRect (x,y,1,1);
                //TODO:check this
                point.luminosity = ((2 * DENOM) / 255 ) * maxValue / 2;
    
                hpList.append(point);
            }
        }
    }
    
    //Now join points together into groups
    consolidatePixels (hpList);
    
    //And notify
    emit parsed(hpList);
}

// Consolidate adjacent points into larger points.

void BlackFrameParser::consolidatePixels (QValueList<HotPixel>& list)
{
    if (list.isEmpty()) 
        return;

    /* Consolidate horizontally.  */
    
    QValueList<HotPixel>::iterator it, prevPointIt;

    prevPointIt=list.begin();
    it=list.begin();
    ++it;
    
    HotPixel tmp;
    HotPixel point;
    HotPixel point_below;
    QValueList<HotPixel>::iterator end(list.end()); 
    for (; it != end; ++it )
    {
        while (1)
        {
            point = (*it);
            tmp   = point;
    
            QValueList<HotPixel>::Iterator point_below_it;
            point_below_it = list.find (tmp); //find any intersecting hotp below tmp
            if (point_below_it != list.end())
            {
                point_below =* point_below_it;
                validateAndConsolidate (&point, &point_below);
                
                point.rect.setX(MIN(point.x(), point_below.x()));
                point.rect.setWidth(MAX(point.x() + point.width(),
                                    point_below.x() + point_below.width()) - point.x());
                point.rect.setHeight(MAX(point.y() + point.height(),
                                     point_below.y() + point_below.height()) - point.y());
                *it=point;
                list.remove (point_below_it); //TODO: Check! this could remove it++?
            }
            else    
                break;
        }
    }
}

void BlackFrameParser::validateAndConsolidate (HotPixel *a, HotPixel *b)
{
    a->luminosity = MAX (a->luminosity, b->luminosity);
}

}  // NameSpace DigikamHotPixelsImagesPlugin
