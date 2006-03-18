/* ============================================================
 * File  : imageeffect_emboss.h
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
           Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Date  : 2004-08-26
 * Description : a digiKam image editor plugin to emboss 
 *               an image.
 * 
 * Copyright 2004-2005 by Gilles Caulier
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

#ifndef IMAGEEFFECT_EMBOSS_H
#define IMAGEEFFECT_EMBOSS_H

// Digikam includes.

#include <digikamheaders.h>

class KIntNumInput;

namespace DigikamEmbossImagesPlugin
{

class ImageEffect_Emboss : public Digikam::CtrlPanelDlg
{
    Q_OBJECT

public:

    ImageEffect_Emboss(QWidget* parent, QString title, QFrame* banner);
    ~ImageEffect_Emboss();

private:

    KIntNumInput *m_depthInput;

protected:

    void prepareEffect(void);
    void prepareFinal(void);
    void putPreviewData(void);
    void putFinalData(void);
    void resetValues(void);   
    void renderingFinished(void);
};

}  // NameSpace DigikamEmbossImagesPlugin

#endif /* IMAGEEFFECT_EMBOSS_H */
