/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-25
 * Description : a digiKam image plugin to reduce
 *               vignetting on an image.
 *
 * Copyright (C) 2005-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef IMAGEEFFECT_ANTIVIGNETTING_H
#define IMAGEEFFECT_ANTIVIGNETTING_H

// Digikam includes.

#include "imageguidedlg.h"

class QLabel;

namespace KDcrawIface
{
class RIntNumInput;
class RDoubleNumInput;
}

namespace DigikamAntiVignettingImagesPlugin
{

class ImageEffect_AntiVignetting : public Digikam::ImageGuideDlg
{
    Q_OBJECT

public:

    ImageEffect_AntiVignetting(QWidget *parent);
    ~ImageEffect_AntiVignetting();

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

    QLabel                       *m_maskPreviewLabel;

    KDcrawIface::RIntNumInput    *m_brightnessInput;
    KDcrawIface::RIntNumInput    *m_contrastInput;

    KDcrawIface::RDoubleNumInput *m_gammaInput;
    KDcrawIface::RDoubleNumInput *m_densityInput;
    KDcrawIface::RDoubleNumInput *m_powerInput;
    KDcrawIface::RDoubleNumInput *m_radiusInput;
};

}  // NameSpace DigikamAntiVignettingImagesPlugin

#endif /* IMAGEEFFECT_ANTIVIGNETTING_H */
