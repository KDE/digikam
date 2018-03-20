/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-07-21
 * Description : a widget to display an image histogram.
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

#ifndef HISTOGRAMWIDGET_H
#define HISTOGRAMWIDGET_H

// Qt includes

#include <QWidget>
#include <QEvent>
#include <QMouseEvent>
#include <QPaintEvent>

// Local includes

#include "dimg.h"
#include "dcolor.h"
#include "digikam_debug.h"
#include "digikam_export.h"
#include "digikam_globals.h"

namespace Digikam
{

class ImageHistogram;

class DIGIKAM_EXPORT HistogramWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int animationState READ animationState WRITE setAnimationState)

public:

    /** Standard constructor. Needed to use updateData() methods after to create valid instance.
     */
    HistogramWidget(int w, int h,                              // Widget size.
                    QWidget* const parent=0, bool selectMode=true,
                    bool showProgress=true,
                    bool statisticsVisible=false);

    ~HistogramWidget();

    /** Stop current histogram computations.
     */
    void stopHistogramComputation();

    /** Update full image histogram data methods.
     */
    void updateData(const DImg& img,                        // full image data.
                    const DImg& sel=DImg(),                 // selection image data
                    bool showProgress=true);

    /** Update image selection histogram data methods.
     */
    void updateSelectionData(const DImg& sel, bool showProgress=true);

    void setDataLoading();
    void setLoadingFailed();

    void setHistogramGuideByColor(const DColor& color);
    void setStatisticsVisible(bool b);

    void reset();

    HistogramScale scaleType()   const;
    ChannelType    channelType() const;

    int  animationState() const;
    void setAnimationState(int animationState);

    void setRenderingType(HistogramRenderingType type);
    HistogramRenderingType renderingType() const;

    /** Currently rendered histogram, depending on current rendering type.
     */
    ImageHistogram* currentHistogram() const;

Q_SIGNALS:

    void signalIntervalChanged(int min, int max);
    void signalMaximumValueChanged(int);
    void signalHistogramComputationDone(bool);
    void signalHistogramComputationFailed();

public Q_SLOTS:

    void slotMinValueChanged(int min);
    void slotMaxValueChanged(int max);
    void setChannelType(ChannelType channel);
    void setScaleType(HistogramScale scale);

protected Q_SLOTS:

    void slotCalculationAboutToStart();
    void slotCalculationFinished(bool success);

protected:

    void paintEvent(QPaintEvent*);
    void mousePressEvent(QMouseEvent*);
    void mouseReleaseEvent(QMouseEvent*);
    void mouseMoveEvent(QMouseEvent*);

private:

    void notifyValuesChanged();
    void connectHistogram(const ImageHistogram* const histogram);
    void setup(int w, int h, bool selectMode, bool statisticsVisible);
    void setState(int state);
    void startWaitingAnimation();
    void stopWaitingAnimation();

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* HISTOGRAMWIDGET_H */
