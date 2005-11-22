/* ============================================================
 * File  : imageeffect_antivignetting.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-12-25
 * Description : a digiKam image plugin to reduce 
 *               vignetting on an image.
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
 * 
 * ============================================================ */

#ifndef IMAGEEFFECT_ANTIVIGNETTING_H
#define IMAGEEFFECT_ANTIVIGNETTING_H

// Local includes.

#include "imageguidedialog.h"

class QLabel;
class QTabWidget;

class KIntNumInput;
class KDoubleNumInput;

namespace DigikamAntiVignettingImagesPlugin
{

class ImageEffect_AntiVignetting : public DigikamImagePlugins::ImageGuideDialog
{
    Q_OBJECT
    
public:

    ImageEffect_AntiVignetting(QWidget *parent);
    ~ImageEffect_AntiVignetting();

private:

    QLabel               *m_maskPreviewLabel;
    
    KIntNumInput         *m_brightnessInput;
    KIntNumInput         *m_contrastInput;
    KIntNumInput         *m_gammaInput;
    
    KDoubleNumInput      *m_densityInput;
    KDoubleNumInput      *m_powerInput;
    KDoubleNumInput      *m_radiusInput;
    
protected:
    
    void prepareEffect(void);
    void prepareFinal(void);
    void putPreviewData(void);
    void putFinalData(void);
    void resetValues(void);   
    void renderingFinished(void);
};

}  // NameSpace DigikamAntiVignettingImagesPlugin

#endif /* IMAGEEFFECT_ANTIVIGNETTING_H */
