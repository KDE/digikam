/* ============================================================
 * File  : adjustcurves.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-12-01
 * Description : image histogram adjust curves. 
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

#ifndef ADJUSTCURVES_H
#define ADJUSTCURVES_H

// KDE includes.

#include <kdialogbase.h>

class QComboBox;
class QPushButton;

namespace Digikam
{
class HistogramWidget;
class ImageCurves;
class ImageWidget;
class ColorGradientWidget;
}

namespace DigikamAdjustCurvesImagesPlugin
{

class AdjustCurveDialog : public KDialogBase
{
    Q_OBJECT

public:

    AdjustCurveDialog(QWidget *parent, uint *imageData, uint width, uint height);
    ~AdjustCurveDialog();

protected:

    void closeEvent(QCloseEvent *e);
    
private:
    
    QComboBox                    *m_channelCB;    
    QComboBox                    *m_scaleCB;    
    
    QPushButton                  *m_loadButton;
    QPushButton                  *m_saveButton;
    QPushButton                  *m_helpButton;
    QPushButton                  *m_resetButton;
    
    Digikam::HistogramWidget     *m_histogramWidget;
    
    Digikam::ColorGradientWidget *m_hGradient;
    Digikam::ColorGradientWidget *m_vGradient;
            
    Digikam::ImageWidget         *m_previewOriginalWidget;
    Digikam::ImageWidget         *m_previewTargetWidget;
    
    Digikam::ImageCurves         *m_curves;

private:

    bool loadCurvesFromFile(KURL fileUrl);
    bool saveCurvesToFile(KURL fileUrl);
    
private slots:

    void slotUser1();
    void slotEffect();
    void slotOk();
    void slotHelp();
    void slotResetAllChannels();
    void slotLoadCurves();
    void slotSaveCurves();
    void slotChannelChanged(int channel);
    void slotScaleChanged(int scale);
};

}  // NameSpace DigikamAdjustCurvesImagesPlugin

#endif /* ADJUSTCURVES_H */
