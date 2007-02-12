/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date   : 2004-11-28
 * Description : a digiKam image editor plugin to process image 
 *               free rotation.
 * 
 * Copyright 2004-2007 by Gilles Caulier
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

#ifndef IMAGEEFFECT_FREEROTATION_H
#define IMAGEEFFECT_FREEROTATION_H

// Local includes.

#include <digikamheaders.h>

class QFrame;
class QLabel;
class QCheckBox;
class QComboBox;

class KIntNumInput;
class KDoubleNumInput;

namespace DigikamFreeRotationImagesPlugin
{

class ImageEffect_FreeRotation : public Digikam::ImageGuideDlg
{
    Q_OBJECT
    
public:

    ImageEffect_FreeRotation(QWidget *parent, QString title, QFrame* banner);
    ~ImageEffect_FreeRotation();

private slots:
    
    void readUserSettings(void);

protected:

    void writeUserSettings();    
    void resetValues();   
    void prepareEffect();
    void prepareFinal();
    void putPreviewData();
    void putFinalData();
    void renderingFinished();

private:

    QLabel           *m_newWidthLabel;
    QLabel           *m_newHeightLabel;

    QCheckBox        *m_antialiasInput;
    
    QComboBox        *m_autoCropCB;
    
    KIntNumInput     *m_angleInput;

    KDoubleNumInput  *m_fineAngleInput;
};

}  // NameSpace DigikamFreeRotationImagesPlugin

#endif /* IMAGEEFFECT_FREEROTATION_H */
