/* ============================================================
 * File  : imageeffect_lensdistortion.h
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
           Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Date  : 2004-12-27
 * Description : a digiKam image plugin for to reduce spherical
 *               aberration provide by lens on an image.
 * 
 * Copyright 2004-2006 by Gilles Caulier
 * Copyright 2006 by Gilles Caulier and Marcel Wiesweg
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

#include <digikamheaders.h>

class QLabel;

class KDoubleNumInput;

namespace DigikamLensDistortionImagesPlugin
{

class ImageEffect_LensDistortion : public Digikam::ImageGuideDlg
{
    Q_OBJECT
    
public:

    ImageEffect_LensDistortion(QWidget *parent, QString title, QFrame* banner);
    ~ImageEffect_LensDistortion();

private:

    QLabel               *m_maskPreviewLabel;

    KDoubleNumInput      *m_mainInput;
    KDoubleNumInput      *m_edgeInput;
    KDoubleNumInput      *m_rescaleInput;
    KDoubleNumInput      *m_brightenInput;

    Digikam::DImg         m_previewRasterImage;

private slots:

    void readUserSettings(void);

protected:

    void writeUserSettings(void);
    void prepareEffect(void);
    void prepareFinal(void);
    void putPreviewData(void);
    void putFinalData(void);
    void resetValues(void);
    void renderingFinished(void);
};

}  // NameSpace DigikamLensDistortionImagesPlugin

#endif /* IMAGEEFFECT_LENSDISTORTION_H */
