/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at gmail dot com>
 * Date   : 2004-07-09
 * Description : a tool to blur an image
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

#endif /* IMAGEEFFECT_BLUR_H */
