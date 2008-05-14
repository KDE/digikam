/* ============================================================
 *
 * Date        : 2008-02-10
 * Description : a plugin to fix automaticaly camera lens aberrations
 *
 * Copyright (C) 2008 by Adrian Schroeter <adrian at suse dot de>
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

#ifndef IMAGEEFFECT_AUTOCORRECTION_H
#define IMAGEEFFECT_AUTOCORRECTION_H

// local includes.

#include "dimg.h"
#include "imageguidedlg.h"

class QCheckBox;
class QLabel;
class QWidget;

class KLFDeviceSelector;

namespace DigikamAutoCorrectionImagesPlugin
{

class ImageEffect_AutoCorrection : public Digikam::ImageGuideDlg
{
    Q_OBJECT

public:

    ImageEffect_AutoCorrection(QWidget *parent);
    ~ImageEffect_AutoCorrection();

private slots:

    void readUserSettings();
    void slotSetFilters();
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

    QCheckBox         *m_showGrid;
    QCheckBox         *m_filterCCA;
    QCheckBox         *m_filterVig;
    QCheckBox         *m_filterCCI;
    QCheckBox         *m_filterDist;
    QCheckBox         *m_filterGeom;

    KLFDeviceSelector *m_cameraSelector;

private slots:

//    virtual void slotDefault(){};
//    virtual void slotCancel(){};
//    virtual void slotUser1(){};
//    void readUserSettings();
    void slotInit();

};

}  // NameSpace DigikamAutoCorrectionImagesPlugin

#endif /* IMAGEEFFECT_AUTOCORRECTION_H */
