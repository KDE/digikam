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
    LogScaleHistogram         // Logarithmic scale.
};
    
public:

    HistogramWidget(int w, int h, uint *i_data, uint i_w, uint i_h,
                    QWidget *parent=0, bool selectMode=true);
    ~HistogramWidget();

    class ImageHistogram *m_imageHistogram;
    
    int    m_channelType;     // Channel type to draw.
    int    m_scaleType;       // Scale to use for drawing.

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
