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

#ifndef IMAGEPROPERTIESHISTOGRAMTAB_H
#define IMAGEPROPERTIESHISTOGRAMTAB_H

// Qt includes.

#include <qwidget.h>

// KDE includes.

#include <kurl.h>

// Local includes.

#include "dimg.h"
#include "digikam_export.h"

class QComboBox;
class QSpinBox;
class QPixmap;
class QLabel;
class QHButtonGroup;

class KFileItem;

namespace Digikam
{
class HistogramWidget;
class ColorGradientWidget;
class NavigateBarWidget;

class DIGIKAM_EXPORT ImagePropertiesHistogramTab : public QWidget
{
    Q_OBJECT

public:

    ImagePropertiesHistogramTab(QWidget* parent, QRect* selectionArea, bool navBar=true);
    ~ImagePropertiesHistogramTab();

    void setData(const KURL& url=KURL::KURL(), QRect *selectionArea=0, 
                 uchar* imageData=0, int imageWidth=0, int imageHeight=0, 
                 bool sixteenBit=false, int itemType=0);

    void setSelection(QRect *selectionArea);

signals:
    
    void signalFirstItem(void);    
    void signalPrevItem(void);    
    void signalNextItem(void);    
    void signalLastItem(void); 

private:

    void updateInformation();
    
private slots:

    void slotRefreshOptions();
    void slotChannelChanged(int channel);
    void slotScaleChanged(int scale);
    void slotColorsChanged(int color);
    void slotRenderingChanged(int rendering);
    void slotIntervChanged(int);

    void slotUpdateMinInterv(int min);
    void slotUpdateMaxInterv(int max);

private:

    QComboBox           *m_channelCB;    
    QComboBox           *m_colorsCB;    
    QComboBox           *m_renderingCB;    

    QHButtonGroup       *m_scaleBG;
    QHButtonGroup       *m_regionBG;
    
    QSpinBox            *m_minInterv;
    QSpinBox            *m_maxInterv;
    
    QLabel              *m_labelMeanValue;
    QLabel              *m_labelPixelsValue;
    QLabel              *m_labelStdDevValue;
    QLabel              *m_labelCountValue;
    QLabel              *m_labelMedianValue;
    QLabel              *m_labelPercentileValue;
    
    DImg                 m_image;
    DImg                 m_imageSelection;
    
    QRect               *m_selectionArea;
    
    ColorGradientWidget *m_hGradient;
    HistogramWidget     *m_histogramWidget;
    NavigateBarWidget   *m_navigateBar;   
};

}  // NameSpace Digikam

#endif /* IMAGEPROPERTIESHISTOGRAMTAB_H */
