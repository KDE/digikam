/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date   : 2004-07-09
 * Description : a tool to sharp an image
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

#ifndef IMAGEEFFECT_SHARPEN_H
#define IMAGEEFFECT_SHARPEN_H

// Digikam include.

#include "ctrlpaneldlg.h"

class KIntNumInput;

namespace DigikamImagesPluginCore
{

class ImageEffect_Sharpen : public Digikam::CtrlPanelDlg
{
    Q_OBJECT

public:

    ImageEffect_Sharpen(QWidget *parent);
    ~ImageEffect_Sharpen();

private slots:

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
    
    KIntNumInput *m_radiusInput;
};

}  // NameSpace DigikamImagesPluginCore

#endif /* IMAGEEFFECT_SHARPEN_H */
