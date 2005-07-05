/* ============================================================
 * File  : imageeffect_filmgrain.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-08-26
 * Description : a digiKam image editor plugin for add film 
 *               grain on an image.
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

#ifndef IMAGEEFFECT_FILMGRAIN_H
#define IMAGEEFFECT_FILMGRAIN_H

// Local includes.

#include "ctrlpaneldialog.h"

class QSlider;
class QLCDNumber;

namespace DigikamFilmGrainImagesPlugin
{

class ImageEffect_FilmGrain : public DigikamImagePlugins::CtrlPanelDialog
{
    Q_OBJECT

public:

    ImageEffect_FilmGrain(QWidget* parent);
    ~ImageEffect_FilmGrain();

        
private:

    QSlider     *m_sensibilitySlider;
    
    QLCDNumber  *m_sensibilityLCDValue;

private slots:

    void slotSensibilityChanged(int);

protected:
    
    void prepareEffect(void);
    void prepareFinal(void);
    void putPreviewData(void);
    void putFinalData(void);
    void resetValues(void);   
    void renderingFinished(void);    
};

}  // NameSpace DigikamFilmGrainImagesPlugin

#endif /* IMAGEEFFECT_FILMGRAIN_H */
