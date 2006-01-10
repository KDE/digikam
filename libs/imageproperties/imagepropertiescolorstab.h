/* ============================================================
 * Author: Caulier Gilles <caulier dot gilles at free.fr>
 * Date  : 2004-11-17
 * Description :
 *
 * Copyright 2004-2006 by Gilles Caulier
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

#ifndef IMAGEPROPERTIESCOLORSTAB_H
#define IMAGEPROPERTIESCOLORSTAB_H

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

class KSqueezedTextLabel;

namespace Digikam
{
class DImg;
class HistogramWidget;
class ColorGradientWidget;
class NavigateBarWidget;
class ManagedLoadSaveThread;

class DIGIKAM_EXPORT ImagePropertiesColorsTab : public QWidget
{
    Q_OBJECT

public:

    ImagePropertiesColorsTab(QWidget* parent, QRect* selectionArea, bool navBar=true);
    ~ImagePropertiesColorsTab();

    void setData(const KURL& url=KURL::KURL(), QRect *selectionArea=0, 
                 DImg *img=0, int itemType=0);

    void setSelection(QRect *selectionArea);

signals:
    
    void signalFirstItem(void);    
    void signalPrevItem(void);    
    void signalNextItem(void);    
    void signalLastItem(void); 

private:

    void loadImageFromUrl(const KURL& url);
    void updateInformations();
    void updateStatistiques();
    void getICCData();
    
private slots:

    void slotRefreshOptions(bool sixteenBit);
    void slotHistogramComputationFailed(void);
    void slotChannelChanged(int channel);
    void slotScaleChanged(int scale);
    void slotColorsChanged(int color);
    void slotRenderingChanged(int rendering);
    void slotMinValueChanged(int);
    void slotMaxValueChanged(int);

    void slotUpdateInterval(int min, int max);
    void slotUpdateIntervRange(int range);

    void slotLoadImageFromUrlComplete(const QString&, const DImg& img);
    void slotProgressInfo(const QString& filePath, float progress);

private:

    QComboBox             *m_channelCB;    
    QComboBox             *m_colorsCB;    
    QComboBox             *m_renderingCB;    

    QHButtonGroup         *m_scaleBG;
    QHButtonGroup         *m_regionBG;
    
    QSpinBox              *m_minInterv;
    QSpinBox              *m_maxInterv;
    
    QLabel                *m_labelMeanValue;
    QLabel                *m_labelPixelsValue;
    QLabel                *m_labelStdDevValue;
    QLabel                *m_labelCountValue;
    QLabel                *m_labelMedianValue;
    QLabel                *m_labelPercentileValue;
    QLabel                *m_labelColorDepth;
    QLabel                *m_labelAlphaChannel;
    QLabel                *m_infoHeader;
    
    KSqueezedTextLabel    *m_labelICCName;
    KSqueezedTextLabel    *m_labelICCDescription;
    KSqueezedTextLabel    *m_labelICCCopyright;
    KSqueezedTextLabel    *m_labelICCIntent;
    KSqueezedTextLabel    *m_labelICCColorSpace;
    
    DImg                   m_image;
    DImg                   m_imageSelection;
    
    QRect                 *m_selectionArea;
    
    ColorGradientWidget   *m_hGradient;
    HistogramWidget       *m_histogramWidget;
    NavigateBarWidget     *m_navigateBar;
    ManagedLoadSaveThread *m_imageLoaderThreaded;
    QString                m_currentFilePath;

    QByteArray             m_embedded_profile;
    
};

}  // NameSpace Digikam

#endif /* IMAGEPROPERTIESCOLORSTAB_H */
