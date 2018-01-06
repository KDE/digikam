/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-07-21
 * Description : image histogram manipulation methods.
 *
 * Copyright (C) 2004-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes

#include <QEvent>
#include <QThread>

// Local includes

#include "digikam_export.h"
#include "dynamicthread.h"

class QObject;

namespace Digikam
{

class DImg;

class DIGIKAM_EXPORT ImageHistogram : public DynamicThread
{
    Q_OBJECT

public:

    explicit ImageHistogram(const DImg& img, QObject* const parent = 0);
    ~ImageHistogram();

    /** Started computation: synchronous or threaded */
    void calculate();
    void calculateInThread();

    /** Stop threaded computation. */
    void stopCalculation();
    bool isCalculating()  const;

    /** Methods to access the histogram data.*/
    bool   isSixteenBit() const;
    bool   isValid()      const;

    double getCount(int channel, int start, int end)   const;
    double getMean(int channel, int start, int end)    const;
    double getPixels()                                 const;
    double getStdDev(int channel, int start, int end)  const;
    double getValue(int channel, int bin)              const;
    double getMaximum(int channel, int start, int end) const;

    int    getHistogramSegments()                      const;
    int    getMaxSegmentIndex()                        const;
    int    getMedian(int channel, int start, int end)  const;

Q_SIGNALS:

    void calculationFinished(bool success);
    /// when calculation in thread is initiated, from other thread
    void calculationAboutToStart();
    /// emitted from calculation thread
    void calculationStarted();

protected:

    virtual void run();

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* IMAGEHISTOGRAM_H */
