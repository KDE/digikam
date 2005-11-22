/* ============================================================
 * File  : imageeffect_sheartool.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
  * Date  : 2004-12-23
 * Description : a digiKam image editor plugin to process 
 *               shearing image.
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

#ifndef IMAGEEFFECT_SHEARTOOL_H
#define IMAGEEFFECT_SHEARTOOL_H

// Local includes.

#include "imageguidedialog.h"

class QPushButton;
class QCheckBox;
class QLabel;

class KDoubleNumInput;

namespace DigikamShearToolImagesPlugin
{

class ImageEffect_ShearTool : public DigikamImagePlugins::ImageGuideDialog
{
    Q_OBJECT

public:

    ImageEffect_ShearTool(QWidget* parent);
    ~ImageEffect_ShearTool();

private:

    QLabel          *m_newWidthLabel;
    QLabel          *m_newHeightLabel;
   
    QCheckBox       *m_antialiasInput;
    
    KDoubleNumInput *m_magnitudeX;
    KDoubleNumInput *m_magnitudeY;
    
private slots:
    
    void readUserSettings(void);

protected:

    void writeUserSettings(void);    
    void prepareEffect(void);
    void prepareFinal(void);
    void putPreviewData(void);
    void putFinalData(void);
    void resetValues(void);   
    void renderingFinished(void);
   
};

}  // NameSpace DigikamShearToolImagesPlugin

#endif /* IMAGEEFFECT_SHEARTOOL_H */
