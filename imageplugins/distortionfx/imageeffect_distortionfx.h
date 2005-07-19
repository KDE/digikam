/* ============================================================
 * File  : imageeffect_distortionfx.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-02-11
 * Description : 
 * 
 * Copyright 2005 by Gilles Caulier
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

#ifndef IMAGEEFFECT_DISTORTIONFX_H
#define IMAGEEFFECT_DISTORTIONFX_H

// Local includes.

#include "imageguidedialog.h"

class QPushButton;
class QComboBox;
class QLabel;

class KIntNumInput;

namespace DigikamDistortionFXImagesPlugin
{

class ImageEffect_DistortionFX : public DigikamImagePlugins::ImageGuideDialog
{
    Q_OBJECT
    
public:

    ImageEffect_DistortionFX(QWidget *parent);
    ~ImageEffect_DistortionFX();

private:
    
    QComboBox            *m_effectType;

    QLabel               *m_effectTypeLabel;
    QLabel               *m_levelLabel;
    QLabel               *m_iterationLabel;
    
    KIntNumInput         *m_levelInput;
    KIntNumInput         *m_iterationInput;

protected:
    
    void prepareEffect(void);
    void prepareFinal(void);
    void putPreviewData(void);
    void putFinalData(void);
    void resetValues(void);   
    void renderingFinished(void);    
    
private slots:

    void slotEffectTypeChanged(int type);
};

}  // NameSpace DigikamDistortionFXImagesPlugin

#endif /* IMAGEEFFECT_DISTORTIONFX_H */
