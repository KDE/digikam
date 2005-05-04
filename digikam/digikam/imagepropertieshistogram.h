/* ============================================================
 * Author: Caulier Gilles <caulier dot gilles at free.fr>
 * Date  : 2004-11-17
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
 * ============================================================ */

#ifndef IMAGEPROPERTIESHISTOGRAM_H
#define IMAGEPROPERTIESHISTOGRAM_H

// Qt includes.

#include <qobject.h>
#include <qguardedptr.h>
#include <qimage.h>

class QComboBox;
class QSpinBox;
class QPixmap;
class QLabel;

class KURL;
class KFileMetaInfo;

class ThumbnailJob;

namespace Digikam
{
class HistogramWidget;
class ColorGradientWidget;
}

class ImagePropertiesHistogram : public QObject
{
    Q_OBJECT

public:

    ImagePropertiesHistogram(QWidget* page, QRect* selectionArea);
    ~ImagePropertiesHistogram();

    void setData(const KURL& url, uint* imageData=0, int imageWidth=0, int imageHeight=0);

private:

    void updateInformation();
    
private slots:

    void slotRefreshOptions();
    void slotChannelChanged(int channel);
    void slotScaleChanged(int scale);
    void slotColorsChanged(int color);
    void slotRenderingChanged(int rendering);
    void slotIntervChanged(int);
    void slotGotThumbnail(const KURL&, const QPixmap& pix,
                          const KFileMetaInfo*);  
    void slotFailedThumbnail(const KURL&);

    void slotUpdateMinInterv(int min);
    void slotUpdateMaxInterv(int max);

private:

    QComboBox                    *m_channelCB;    
    QComboBox                    *m_scaleCB;    
    QComboBox                    *m_colorsCB;    
    QComboBox                    *m_renderingCB;    
        
    QSpinBox                     *m_minInterv;
    QSpinBox                     *m_maxInterv;
    
    QLabel                       *m_labelThumb;
    QLabel                       *m_labelMeanValue;
    QLabel                       *m_labelPixelsValue;
    QLabel                       *m_labelStdDevValue;
    QLabel                       *m_labelCountValue;
    QLabel                       *m_labelMedianValue;
    QLabel                       *m_labelPercentileValue;
    QLabel                       *m_labelRendering;
    
    QImage                        m_image;
    QImage                        m_imageSelection;
    
    QRect                        *m_selectionArea;
    
    QGuardedPtr<ThumbnailJob>     m_thumbJob;
    
    Digikam::ColorGradientWidget *m_hGradient;
    Digikam::HistogramWidget     *m_histogramWidget;
};

#endif /* IMAGEPROPERTIESHISTOGRAM_H */
