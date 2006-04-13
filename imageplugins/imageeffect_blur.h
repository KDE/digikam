/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2004-07-09
 * Description : Blur image filter for ImageEditor
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

#ifndef IMAGEEFFECT_BLUR_H
#define IMAGEEFFECT_BLUR_H

// Digikam include.

#include "ctrlpaneldlg.h"

class KIntNumInput;

namespace DigikamImagesPluginCore
{

class ImageEffect_Blur : public Digikam::CtrlPanelDlg
{
    Q_OBJECT

public:

    ImageEffect_Blur(QWidget *parent);
    ~ImageEffect_Blur();

private:

    KIntNumInput *m_radiusInput;
    
protected:
    
    void prepareEffect(void);
    void prepareFinal(void);
    void abortPreview(void);
    void putPreviewData(void);
    void putFinalData(void);
    void resetValues(void);   
    void renderingFinished(void);
};

}  // NameSpace DigikamImagesPluginCore

#endif /* IMAGEEFFECT_BLUR_H */
