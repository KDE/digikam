/* ============================================================
 * File  : imageeffect_refocus.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-04-29
 * Description : a digiKam image editor plugin to refocus 
 *               an image.
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

private:
    
    Digikam::DImg    m_img;
    
    KIntNumInput    *m_matrixSize;
    
    KDoubleNumInput *m_radius;
    KDoubleNumInput *m_gauss;
    KDoubleNumInput *m_correlation;
    KDoubleNumInput *m_noise;
    
private slots:

    void slotUser2();
    void slotUser3();

protected:
    
    void prepareEffect(void);
    void prepareFinal(void);
    void abortPreview(void);
    void putPreviewData(void);
    void putFinalData(void);
    void resetValues(void);   
    void renderingFinished(void);
};

}  // NameSpace DigikamRefocusImagesPlugin

#endif /* IMAGEEFFECT_REFOCUS_H */
