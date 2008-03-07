/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-12-23
 * Description : a plugin to shear an image
 * 
 * Copyright (C) 2004-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "imageguidedlg.h"

class QFrame;
class QPushButton;
class QCheckBox;
class QLabel;

class KIntNumInput;
class KDoubleNumInput;

namespace DigikamShearToolImagesPlugin
{

class ImageEffect_ShearTool : public Digikam::ImageGuideDlg
{
    Q_OBJECT

public:

    ImageEffect_ShearTool(QWidget* parent);
    ~ImageEffect_ShearTool();
    
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

private:

    QLabel          *m_newWidthLabel;
    QLabel          *m_newHeightLabel;
   
    QCheckBox       *m_antialiasInput;
    
    KIntNumInput    *m_mainHAngleInput;
    KIntNumInput    *m_mainVAngleInput;

    KDoubleNumInput *m_fineHAngleInput;
    KDoubleNumInput *m_fineVAngleInput;
};

}  // NameSpace DigikamShearToolImagesPlugin

#endif /* IMAGEEFFECT_SHEARTOOL_H */
