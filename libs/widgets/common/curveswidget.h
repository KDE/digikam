/* ============================================================
 * File  : curveswidget.h
 * Author: Gilles Caulier <caulier dot gilles at gmail dot com>
 * Date  : 2004-12-01
 * Description : 
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

#ifndef CURVESWIDGET_H
#define CURVESWIDGET_H

// Qt includes.

#include <qwidget.h>

// Local includes

#include "dcolor.h"
#include "digikam_export.h"

class QCustomEvent;

namespace Digikam
{

class ImageHistogram;
class ImageCurves;
class CurvesWidgetPriv;

class DIGIKAM_EXPORT CurvesWidget : public QWidget
{
Q_OBJECT

public:

    enum HistogramType
    {
        ValueHistogram = 0,       // Luminosity.
        RedChannelHistogram,      // Red channel.
        GreenChannelHistogram,    // Green channel.
        BlueChannelHistogram,     // Blue channel.
        AlphaChannelHistogram,    // Alpha channel.
    };

    enum HistogramScale
    {
        LinScaleHistogram=0,      // Linear scale.
        LogScaleHistogram         // Logarithmic scale.
    };
    
public:

    CurvesWidget(int w, int h,                         // Widget size.
                 uchar *i_data, uint i_w, uint i_h,    // Full image info.
                 bool i_sixteenBits,                   // 8 or 16 bits image.
                 ImageCurves *curves,                  // Curves data instance to use.
                 QWidget *parent=0,                    // Parent widget instance.
                 bool readOnly=false);                 // If true : widget with full edition mode capabilities.
                                                       // If false : display curve data only without edition.
                 
    ~CurvesWidget();

    // Stop current histogram computations.
    void stopHistogramComputation(void);
    
    void reset(void);
    void curveTypeChanged(void);
    void setCurveGuide(DColor color);
    
public:
    
    int             m_channelType;     // Channel type to draw.
    int             m_scaleType;       // Scale to use for drawing.

    ImageHistogram *m_imageHistogram;

signals:
    
    void signalMouseMoved( int x, int y );
    void signalCurvesChanged(void);
    void signalHistogramComputationDone(void);
    void signalHistogramComputationFailed(void);

protected slots:
    
    void slotBlinkTimerDone( void );
            
protected:

    void paintEvent( QPaintEvent * );
    void mousePressEvent ( QMouseEvent * e );
    void mouseReleaseEvent ( QMouseEvent * e );
    void mouseMoveEvent ( QMouseEvent * e );
    void leaveEvent ( QEvent * );
    
private:
        
    void customEvent(QCustomEvent *event);

private:

    CurvesWidgetPriv* d;
    
};

}  // NameSpace Digikam

#endif /* CURVESWIDGET_H */
