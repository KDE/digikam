/* ============================================================
 * File  : imageeffect_unsharp.h
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2004-08-27
 * Description : Unsharp mask image filter for digiKam Image Editor
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

#ifndef IMAGEEFFECT_UNSHARP_H
#define IMAGEEFFECT_UNSHARP_H

// Local includes.

#include <digikamheaders.h>

class KDoubleNumInput;
class KIntNumInput;

namespace DigikamUnsharpMaskImagesPlugin
{

class ImageEffect_Unsharp : public Digikam::CtrlPanelDlg
{
    Q_OBJECT

public:

    ImageEffect_Unsharp(QWidget* parent, QString title, QFrame* banner);
    ~ImageEffect_Unsharp();

private:
    
    KIntNumInput    *m_radiusInput;

    KDoubleNumInput *m_amountInput;
    KDoubleNumInput *m_thresholdInput;
    
protected:
    
    void prepareEffect(void);
    void prepareFinal(void);
    void putPreviewData(void);
    void putFinalData(void);
    void resetValues(void);   
    void renderingFinished(void);
};

}  // NameSpace DigikamUnsharpMaskImagesPlugin

#endif /* IMAGEEFFECT_UNSHARP_H */
