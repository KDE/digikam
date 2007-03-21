/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at gmail dot com>
            Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Date   : 2004-12-27
 * Description : a digiKam image plugin for to reduce spherical
 *               aberration provide by lens on an image.
 * 
 * Copyright 2004-2006 by Gilles Caulier
 * Copyright 2006-2007 by Gilles Caulier and Marcel Wiesweg
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

#ifndef IMAGEEFFECT_LENSDISTORTION_H
#define IMAGEEFFECT_LENSDISTORTION_H

// Qt includes.

#include <qimage.h>

// Digikam includes.

#include "dimg.h"
#include "imageguidedlg.h"

class QLabel;

class KDoubleNumInput;

namespace DigikamLensDistortionImagesPlugin
{

class ImageEffect_LensDistortion : public Digikam::ImageGuideDlg
{
    Q_OBJECT
    
public:

    ImageEffect_LensDistortion(QWidget *parent);
    ~ImageEffect_LensDistortion();

private slots:

    void readUserSettings();

private:

    void writeUserSettings();
    void resetValues();     
    void prepareEffect();
    void prepareFinal();
    void putPreviewData();
    void putFinalData();
    void renderingFinished();

private:

    QLabel          *m_maskPreviewLabel;

    KDoubleNumInput *m_mainInput;
    KDoubleNumInput *m_edgeInput;
    KDoubleNumInput *m_rescaleInput;
    KDoubleNumInput *m_brightenInput;

    Digikam::DImg    m_previewRasterImage;
};

}  // NameSpace DigikamLensDistortionImagesPlugin

#endif /* IMAGEEFFECT_LENSDISTORTION_H */
