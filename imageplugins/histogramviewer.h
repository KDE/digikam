/* ============================================================
 * File  : histogramviewer.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-07-21
 * Description : an image histogram viewer dialog.
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

#ifndef HISTOGRAMVIEWER_H
#define HISTOGRAMVIEWER_H

// Qt includes.

#include <qstring.h>
#include <qimage.h>

// KDE includes.

#include <kdialogbase.h>

class QComboBox;
class QSpinBox;
class QLabel;

namespace Digikam
{
class HistogramWidget;
class ColorGradientWidget;
}

class HistogramViewer : public KDialogBase
{
    Q_OBJECT
    
public:

    HistogramViewer(QWidget *parent, QString imageFile);
    HistogramViewer(QWidget *parent, QImage image);
    HistogramViewer(QWidget *parent, uint *imageData, uint width, uint height);
    HistogramViewer(QWidget *parent, 
                    uint *imageData, uint width, uint height,
                    uint *selectionData, uint selectionWidth, uint selectionheight);

    ~HistogramViewer();

public slots:    
    
    void slotUpdateMinInterv(int min);
    void slotUpdateMaxInterv(int max);
        
protected:

    void closeEvent(QCloseEvent *e);
    
private:
    
    QComboBox                    *m_channelCB;    
    QComboBox                    *m_scaleCB;    
    QComboBox                    *m_colorsCB;  
    QComboBox                    *m_renderingCB;
    
    QSpinBox                     *m_minInterv;
    QSpinBox                     *m_maxInterv;
    
    QLabel                       *m_labelMeanValue;
    QLabel                       *m_labelPixelsValue;
    QLabel                       *m_labelStdDevValue;
    QLabel                       *m_labelCountValue;
    QLabel                       *m_labelMedianValue;
    QLabel                       *m_labelPercentileValue;

    QImage                        m_image;
                    
    Digikam::ColorGradientWidget *m_hGradient;
    Digikam::HistogramWidget     *m_histogramWidget;
    
    void setupGui(uint *imageData, uint imageWidth, uint imageHeight,
                  uint *selectionData=0L, uint selectionWidth=0, uint selectionheight=0);
    void updateInformations();

private slots:

    void slotChannelChanged(int channel);
    void slotScaleChanged(int scale);
    void slotColorsChanged(int color);
    void slotRenderingChanged(int rendering);
    void slotIntervChanged(int);
};

#endif /* HISTOGRAMVIEWER_H */
