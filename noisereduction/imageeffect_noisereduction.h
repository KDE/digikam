/* ============================================================
 * File   : imageeffect_noisereduction.h
 * Authors: Gilles Caulier <caulier dot gilles at kdemail dot net>
 *          Peter Heckert <peter dot heckert at arcor dot de>
 * Date   : 2004-08-24
 * Description : noise reduction image filter for digiKam 
 *               image editor.
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
 * 
 * ============================================================ */

#ifndef IMAGEEFFECT_NOISEREDUCTION_H
#define IMAGEEFFECT_NOISEREDUCTION_H

// Local includes.

#include <digikamheaders.h>

class KDoubleNumInput;

namespace DigikamNoiseReductionImagesPlugin
{

class ImageEffect_NoiseReduction : public Digikam::CtrlPanelDlg
{
    Q_OBJECT

public:

    ImageEffect_NoiseReduction(QWidget* parent, QString title, QFrame* banner);
    ~ImageEffect_NoiseReduction();
       
private:

    KDoubleNumInput *m_radiusInput;
    KDoubleNumInput *m_lumToleranceInput;
    KDoubleNumInput *m_thresholdInput;
    KDoubleNumInput *m_textureInput;
    KDoubleNumInput *m_sharpnessInput;

    KDoubleNumInput *m_csmoothInput;
    KDoubleNumInput *m_lookaheadInput;
    KDoubleNumInput *m_gammaInput;
    KDoubleNumInput *m_dampingInput;
    KDoubleNumInput *m_phaseInput;

private slots:

    void slotUser2();
    void slotUser3();

protected:
    
    void prepareEffect(void);
    void prepareFinal(void);
    void putPreviewData(void);
    void putFinalData(void);
    void resetValues(void);   
    void renderingFinished(void);        
};

}  // NameSpace DigikamNoiseReductionImagesPlugin

#endif /* IMAGEEFFECT_NOISEREDUCTION_H */
