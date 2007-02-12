/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at kdemail dot net>
 *          Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Date  : 2005-02-11
 * Description : 
 * 
 * Copyright 2005 by Gilles Caulier
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

#ifndef IMAGEEFFECT_DISTORTIONFX_H
#define IMAGEEFFECT_DISTORTIONFX_H

// Digikam includes.

#include <digikamheaders.h>

class QPushButton;
class QComboBox;
class QLabel;

class KIntNumInput;

namespace DigikamDistortionFXImagesPlugin
{

class ImageEffect_DistortionFX : public Digikam::ImageGuideDlg
{
    Q_OBJECT
    
public:

    ImageEffect_DistortionFX(QWidget *parent, QString title, QFrame* banner);
    ~ImageEffect_DistortionFX();

private slots:

    void slotEffectTypeChanged(int type);
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
    
    QComboBox            *m_effectType;

    QLabel               *m_effectTypeLabel;
    QLabel               *m_levelLabel;
    QLabel               *m_iterationLabel;
    
    KIntNumInput         *m_levelInput;
    KIntNumInput         *m_iterationInput;
};

}  // NameSpace DigikamDistortionFXImagesPlugin

#endif /* IMAGEEFFECT_DISTORTIONFX_H */
