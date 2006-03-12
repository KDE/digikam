/* ============================================================
 * File  : imageeffect_blurfx.h
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
           Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Date  : 2005-02-09
 * Description : 
 * 
 * Copyright 2005 by Gilles Caulier
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

#ifndef IMAGEEFFECT_BLURFX_H
#define IMAGEEFFECT_BLURFX_H

// Digikam includes.

#include <digikamheaders.h>

class QComboBox;
class QLabel;

class KIntNumInput;

namespace DigikamBlurFXImagesPlugin
{

class ImageEffect_BlurFX : public Digikam::CtrlPanelDlg
{
    Q_OBJECT
    
public:

    ImageEffect_BlurFX(QWidget *parent, QString title, QFrame* banner);
    ~ImageEffect_BlurFX();

private:
    
    QComboBox                  *m_effectType;
    
    QLabel                     *m_effectTypeLabel;
    QLabel                     *m_distanceLabel;
    QLabel                     *m_levelLabel;
    
    KIntNumInput               *m_distanceInput;
    KIntNumInput               *m_levelInput;
    
protected:
    
    void prepareEffect(void);
    void prepareFinal(void);
    void abortPreview(void);
    void putPreviewData(void);
    void putFinalData(void);
    void resetValues(void);   
    void renderingFinished(void);
            
private slots:

    void slotEffectTypeChanged(int type);
};

}  // NameSpace DigikamBlurFXImagesPlugin

#endif /* IMAGEEFFECT_BLURFX_H */
