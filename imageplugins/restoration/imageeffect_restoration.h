/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at gmail dot com>
 * Date   : 2005-03-26
 * Description : a digiKam image editor plugin to restore 
 *               a photograph
 * 
 * Copyright 2005-2007 by Gilles Caulier
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

#ifndef IMAGEEFFECT_RESTORATION_H
#define IMAGEEFFECT_RESTORATION_H

// Qt include.

#include <qstring.h>

// Digikam includes.

#include "ctrlpaneldlg.h"

class QComboBox;
class QTabWidget;

namespace Digikam
{
class GreycstorationWidget;
}

namespace DigikamRestorationImagesPlugin
{

class ImageEffect_Restoration : public Digikam::CtrlPanelDlg
{
    Q_OBJECT

public:

    ImageEffect_Restoration(QWidget* parent);
    ~ImageEffect_Restoration();
    
private slots:

    void slotUser2();
    void slotUser3();
    void processCImgURL(const QString&);
    void readUserSettings();
    void slotResetValues(int);

private:
    
    void writeUserSettings();
    void prepareEffect(void);
    void prepareFinal(void);
    void putPreviewData(void);
    void putFinalData(void);
    void resetValues(void);   
    void renderingFinished(void);

private:

    enum RestorationFilteringPreset
    {
        NoPreset=0,
        ReduceUniformNoise,
        ReduceJPEGArtefacts,
        ReduceTexturing,
    };

    QTabWidget                    *m_mainTab;        
            
    QComboBox                     *m_restorationTypeCB;  
    
    Digikam::GreycstorationWidget *m_settingsWidget;
};
    
}  // NameSpace DigikamRestorationImagesPlugin

#endif /* IMAGEEFFECT_RESTORATION_H */
