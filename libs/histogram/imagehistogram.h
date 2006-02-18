/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2004-07-21
 * Description : image histogram manipulation methods.
 *
 * Copyright 2004-2006 by Gilles Caulier
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


#ifndef IMAGEHISTOGRAM_H
#define IMAGEHISTOGRAM_H

// Qt includes.

#include <qthread.h>

// Local includes.

#include "digikam_export.h"

class QObject;

namespace Digikam
{

class ImageHistogramPriv;
class DImg;

class DIGIKAM_EXPORT ImageHistogram : public QThread
{

public:

enum HistogramChannelType
{
    ValueChannel = 0,
    RedChannel,
    GreenChannel,
    BlueChannel,
    AlphaChannel
};

class EventData
{
public:

    EventData() 
    {
       starting = false;
       success = false;
       histogram = 0;
    }

    bool starting;
    bool success;
    ImageHistogram *histogram;
};

public:

    ImageHistogram(uchar *i_data, uint i_w, uint i_h, bool i_sixteenBits, QObject *parent=0);

    ImageHistogram(const DImg& image, QObject *parent=0);
    ~ImageHistogram();

    void setup(uchar *i_data, uint i_w, uint i_h, bool i_sixteenBits, QObject *parent);

    /** Method to stop threaded computations.*/
    void stopCalcHistogramValues(void);

    /** Methods for to manipulate the histogram data.*/
    double getCount(int channel, int start, int end);
    double getMean(int channel, int start, int end);
    double getPixels();
    double getStdDev(int channel, int start, int end);
    double getValue(int channel, int bin);
    double getMaximum(int channel);

    int    getHistogramSegment(void);
    int    getMedian(int channel, int start, int end);

private:

    ImageHistogramPriv* d;
    
private:

    void calcHistogramValues();
    void postProgress(bool starting, bool success);

protected:

    virtual void run();

};

}  // NameSpace Digikam

#endif /* IMAGEHISTOGRAM_H */
