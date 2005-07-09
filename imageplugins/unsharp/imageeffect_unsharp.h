/* ============================================================
 * File  : imageeffect_unsharp.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-08-27
 * Description : Unsharp mask image filter for digiKam Image Editor
 * 
 * Copyright 2004-2005 by Gilles Caulier
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

#include "ctrlpaneldialog.h"

class KIntNumInput;
class KDoubleNumInput;

namespace DigikamUnsharpMaskImagesPlugin
{

class ImageEffect_Unsharp : public DigikamImagePlugins::CtrlPanelDialog
{
    Q_OBJECT

public:

    ImageEffect_Unsharp(QWidget* parent);
    ~ImageEffect_Unsharp();

private:
    
    KDoubleNumInput *m_radiusInput;
    KDoubleNumInput *m_amountInput;
    
    KIntNumInput    *m_thresholdInput;
    
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
