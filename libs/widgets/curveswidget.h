/* ============================================================
 * File  : curveswidget.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
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
#include <qcolor.h>
#include "digikam_export.h"

class QCustomEvent;


namespace Digikam
{
class ImageHistogram;
class ImageCurves;

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
    LinScaleHistogram,        // Linear scale.
    LogScaleHistogram         // Logarithmic scale.
};
    
private:

enum RepaintType
{
    HistogramNone = 0,        // No current histogram values calculation.
    HistogramStarted,         // Histogram values calculation started.
    HistogramCompleted,       // Histogram values calculation completed.
    HistogramFailed           // Histogram values calculation failed.
};    

public:

    CurvesWidget(int w, int h,                                      // Widget size.
                 uint *i_data, uint i_w, uint i_h,                  // Full image info.
                 Digikam::ImageCurves *curves,                      // Curves data instance to use.
                 QWidget *parent=0,                                 // Parent widget instance.
                 bool readOnly=false);                              // If true : widget with full edition mode capabilities.
                                                                    // If false : display curve data only without edition.
                 
    ~CurvesWidget();

    // Stop current histogram computations.
    void stopHistogramComputation(void);
    
    void reset(void);
    void curveTypeChanged(void);
    void setCurveGuide(QColor color);
    
    int m_channelType;     // Channel type to draw.
    int m_scaleType;       // Scale to use for drawing.

    class Digikam::ImageHistogram *m_imageHistogram;          

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

    int                   m_clearFlag;          // Clear drawing zone with message.
    int                   m_leftmost;
    int                   m_rightmost;
    int                   m_grab_point;
    int                   m_last;
    
    bool                  m_blinkFlag;         
    bool                  m_readOnlyMode;
    bool                  m_guideVisible;
    
    QColor                m_colorGuide;
    
    QTimer               *m_blinkTimer;
    
    Digikam::ImageCurves *m_curves;             // Curves data instance.
    
    void customEvent(QCustomEvent *event);
};

}  // NameSpace Digikam

#endif /* CURVESWIDGET_H */
