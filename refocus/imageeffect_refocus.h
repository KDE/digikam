/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date   : 2005-04-29
 * Description : a digiKam image editor plugin to refocus 
 *               an image.
 * 
 * Copyright 2005-2007 by Gilles Caulier
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

#ifndef IMAGEEFFECT_REFOCUS_H
#define IMAGEEFFECT_REFOCUS_H

// Local includes.

#include <digikamheaders.h>

class KIntNumInput;
class KDoubleNumInput;

namespace DigikamRefocusImagesPlugin
{

class ImageEffect_Refocus : public Digikam::CtrlPanelDlg
{
    Q_OBJECT

public:

    ImageEffect_Refocus(QWidget* parent, QString title, QFrame* banner);
    ~ImageEffect_Refocus();

private slots:

    void slotUser2();
    void slotUser3();
    void readUserSettings();

private:

    void writeUserSettings();
    void resetValues();        
    void prepareEffect();
    void prepareFinal();
    void abortPreview();
    void putPreviewData();
    void putFinalData();
    void renderingFinished();

private:
    
    Digikam::DImg    m_img;
    
    KIntNumInput    *m_matrixSize;
    
    KDoubleNumInput *m_radius;
    KDoubleNumInput *m_gauss;
    KDoubleNumInput *m_correlation;
    KDoubleNumInput *m_noise;
};

}  // NameSpace DigikamRefocusImagesPlugin

#endif /* IMAGEEFFECT_REFOCUS_H */
