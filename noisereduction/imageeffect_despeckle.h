/* ============================================================
 * File  : imageeffect_despeckle.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-08-24
 * Description : noise reduction image filter for digiKam 
 *               image editor.
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

#ifndef IMAGEEFFECT_DESPECKLE_H
#define IMAGEEFFECT_DESPECKLE_H

// Local includes.

#include "ctrlpaneldialog.h"

class QCheckBox;

class KIntNumInput;

namespace DigikamNoiseReductionImagesPlugin
{

class ImageEffect_Despeckle : public DigikamImagePlugins::CtrlPanelDialog
{
    Q_OBJECT

public:

    ImageEffect_Despeckle(QWidget* parent);
    ~ImageEffect_Despeckle();
       
private:

    KIntNumInput *m_radiusInput;
    KIntNumInput *m_blackLevelInput;
    KIntNumInput *m_whiteLevelInput;
        
    QCheckBox    *m_useAdaptativeMethod;
    QCheckBox    *m_useRecursiveMethod;
    
protected:
    
    void prepareEffect(void);
    void prepareFinal(void);
    void putPreviewData(void);
    void putFinalData(void);
    void resetValues(void);   
    void renderingFinished(void);        
};

}  // NameSpace DigikamNoiseReductionImagesPlugin

#endif /* IMAGEEFFECT_DESPECKLE_H */
