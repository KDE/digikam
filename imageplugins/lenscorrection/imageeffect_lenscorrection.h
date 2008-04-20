/* ============================================================
 *
 * Date        : 2008-02-10
 * Description : a plugin to fix lens errors
 * 
 * Copyright (C) 2008 Adrian Schroeter <adrian@suse.de>
 * Copyright (C) 2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef IMAGEEFFECT_LENSCORRECTION_H
#define IMAGEEFFECT_LENSCORRECTION_H

// local includes.

#include "dimg.h"
#include "imageguidedlg.h"

class QCheckBox;
class QLabel;
class QWidget;

class KLFDeviceSelector;

namespace DigikamLensCorrectionImagesPlugin
{

class ImageEffect_LensCorrection : public Digikam::ImageGuideDlg
{
    Q_OBJECT

public:

    ImageEffect_LensCorrection(QWidget *parent);
    ~ImageEffect_LensCorrection();

private slots:

    void readUserSettings();
    void setFilters();
    void slotLensChanged();

private:

    void writeUserSettings();
    void resetValues();
    void prepareEffect();
    void prepareFinal();
    void putPreviewData();
    void putFinalData();
    void renderingFinished();

private:

    QLabel            *m_maskPreviewLabel;

    QWidget           *m_settingsWidget;

    QCheckBox         *m_filterCCA;
    QCheckBox         *m_filterVig;
    QCheckBox         *m_filterCCI;
    QCheckBox         *m_filterDist;
    QCheckBox         *m_filterGeom;

    KLFDeviceSelector *m_cameraSelector;

//    double focalDistance;
//    double aperature;

    Digikam::DImg      m_previewRasterImage;

private slots:

//    virtual void slotDefault(){};
//    virtual void slotCancel(){};
//    virtual void slotUser1(){};
//    void readUserSettings();
    void slotInit();

};

}  // NameSpace DigikamLensCorrectionImagesPlugin

#endif /* IMAGEEFFECT_LENSCORRECTION_H */
