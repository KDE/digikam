/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-07-21
 * Description : a widget for to display an image histogram.
 * 
 * Copyright 2004 by Gilles Caulier
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

// Qt includes.

#include <qwidget.h>

class QCustomEvent;

namespace Digikam
{

class ImageHistogram;

class HistogramWidget : public QWidget
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
    ColorChannelsHistogram,   // All color channels.
    LinScaleHistogram,        // Linear scale.
    LogScaleHistogram,        // Logarithmic scale.
    RedColor,                 // Red color to foreground in All Colors Channel mode.
    GreenColor,               // Green color to foreground in All Colors Channel mode.
    BlueColor,                // Blue color to foreground in All Colors Channel mode.
    FullImageHistogram,       // Full image histogram rendering.
    ImageSelectionHistogram   // Image selection histogram rendering.
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

    // Constructor without image data (needed to use updateData() method after instance created).
    
    HistogramWidget(int w, int h,                              // Widget size.
                    QWidget *parent=0, bool selectMode=true);

    // Constructor with image data and without image selection data.
    
    HistogramWidget(int w, int h,                              // Widget size.
                    uint *i_data, uint i_w, uint i_h,          // Full image info.
                    QWidget *parent=0, bool selectMode=true);
    
    // Constructor with image data and image selection data.
    
    HistogramWidget(int w, int h,                              // Widget size.
                    uint *i_data, uint i_w, uint i_h,          // Full image info.
                    uint *s_data, uint s_w, uint s_h,          // Image selection info.
                    QWidget *parent=0, bool selectMode=true);
    
    ~HistogramWidget();

    // Stop current histogram computations.
    void stopHistogramComputation(void);
    
    // Update histogram data method.
    void updateData(uint *i_data, uint i_w, uint i_h, 
                    uint *s_data=0, uint s_w=0, uint s_h=0);
    
    int   m_channelType;     // Channel type to draw.
    int   m_scaleType;       // Scale to use for drawing.
    int   m_colorType;       // Color to use for drawing in All Colors Channel mode.
    int   m_renderingType;   // Using full image or image selection for histogram rendering.

    class ImageHistogram *m_imageHistogram;            // Full image.
    class ImageHistogram *m_selectionHistogram;        // Image selection.

signals:
    
    void signalMousePressed( int );
    void signalMouseReleased( int );
    void signalHistogramComputationDone(void);
    void signalHistogramComputationFailed(void);

public slots:

    void slotMinValueChanged( int min );
    void slotMaxValueChanged( int max );

protected slots:
    
    void slotBlinkTimerDone( void );
            
protected:

    void paintEvent( QPaintEvent * );
    void mousePressEvent ( QMouseEvent * e );
    void mouseReleaseEvent ( QMouseEvent * e );
    void mouseMoveEvent ( QMouseEvent * e );
    
private:

    // Current selection informations.
    int     m_xmin;
    int     m_xminOrg; 
    int     m_xmax;
    int     m_clearFlag;          // Clear drawing zone with message.
    
    bool    m_inSelected;
    bool    m_selectMode;         // If true, a part of the histogram can be selected !
    bool    m_blinkFlag;         
    
    QTimer *m_blinkTimer;
    
    void customEvent(QCustomEvent *event);
};

}

#endif /* HISTOGRAMWIDGET_H */
