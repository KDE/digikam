/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-07-21
 * Description : image histogram manipulation methods.
 *
 * Copyright 2004-2005 by Gilles Caulier
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
       }

    bool starting;
    bool success;
};

private:

    // Using a structure instead a class is more fast 
    // (access with memset() and bytes manipulation).

    struct double_packet
    {
        double value;
        double red;
        double green;
        double blue;
        double alpha;
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

    /** The histogram data.*/
    struct double_packet *m_histogram;

    /** Image informations.*/
    uchar                *m_imageData;
    uint                  m_imageWidth;
    uint                  m_imageHeight;

    /** Numbers of histogram segments dependaing of image bytes depth*/
    int                   m_histoSegments;

    /** To post event from thread to parent.*/
    QObject              *m_parent;

    /** Used to stop thread during calculations.*/
    bool m_runningFlag;

private:

    void calcHistogramValues();
    void postProgress(bool starting, bool success);

protected:

    virtual void run();

};

}  // NameSpace Digikam

#endif /* IMAGEHISTOGRAM_H */
