/* ============================================================
 * File  : imageeffect_raindrop.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-09-30
 * Description : a digiKam image plugin to add
 *               raindrops on an image.
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

#ifndef IMAGEEFFECT_RAINDROP_H
#define IMAGEEFFECT_RAINDROP_H

// Local includes.

#include "imageguidedialog.h"

class KIntNumInput;

namespace DigikamRainDropImagesPlugin
{

class ImageEffect_RainDrop : public DigikamImagePlugins::ImageGuideDialog
{
    Q_OBJECT
    
public:

    ImageEffect_RainDrop(QWidget *parent);
    ~ImageEffect_RainDrop();

private:
            
    KIntNumInput *m_dropInput;
    KIntNumInput *m_amountInput;
    KIntNumInput *m_coeffInput;    
    
protected:
    
    void prepareEffect(void);
    void prepareFinal(void);
    void putPreviewData(void);
    void putFinalData(void);
    void resetValues(void);   
    void renderingFinished(void);
};

}  // NameSpace DigikamRainDropImagesPlugin

#endif /* IMAGEEFFECT_RAINDROP_H */
