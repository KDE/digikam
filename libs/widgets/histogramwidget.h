/* ============================================================
 * File  : histogramwidget.h
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
    
public:

    HistogramWidget(int w, int h,                              // Widget size.
                    uint *i_data, uint i_w, uint i_h,          // Full image info.
                    QWidget *parent=0, bool selectMode=true);
    
    HistogramWidget(int w, int h,                              // Widget size.
                    uint *i_data, uint i_w, uint i_h,          // Full image info.
                    uint *s_data, uint s_w, uint s_h,          // Image selection info.
                    QWidget *parent=0, bool selectMode=true);
    
    ~HistogramWidget();

    int   m_channelType;     // Channel type to draw.
    int   m_scaleType;       // Scale to use for drawing.
    int   m_colorType;       // Color to use for drawing in All Colors Channel mode.
    int   m_renderingType;   // Using full image or image selection for histogram rendering.

    class ImageHistogram *m_imageHistogram;            // Full image.
    class ImageHistogram *m_selectionHistogram;        // Image selection.

signals:
    
    void signalMousePressed( int );
    void signalMouseReleased( int );

public slots:

    void slotMinValueChanged( int min );
    void slotMaxValueChanged( int max );
        
protected:

    void paintEvent( QPaintEvent * );
    void mousePressEvent ( QMouseEvent * e );
    void mouseReleaseEvent ( QMouseEvent * e );
    void mouseMoveEvent ( QMouseEvent * e );
    
private:
    
    // Current selection informations.
    int  m_xmin;
    int  m_xminOrg; 
    int  m_xmax;
    bool m_inSelected;
    bool m_selectMode;         // If true, a part of the histogram can be selected !
};

}

#endif /* HISTOGRAMWIDGET_H */
