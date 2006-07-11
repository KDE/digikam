/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *         Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2004-06-04
 * Description : digiKam image editor plugin core
 * 
 * Copyright 2004-2005 by Renchi Raju and Gilles Caulier
 * Copyright 2006 by Gilles Caulier
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


#ifndef IMAGEPLUGIN_CORE_H
#define IMAGEPLUGIN_CORE_H

// Digikam includes.

#include "imageplugin.h"
#include "digikam_export.h"

class KAction;

class DIGIKAMIMAGEPLUGINS_EXPORT ImagePlugin_Core : public Digikam::ImagePlugin
{
    Q_OBJECT
    
public:

    ImagePlugin_Core(QObject *parent, const char* name, const QStringList &args);
    ~ImagePlugin_Core();

    void setEnabledSelectionActions(bool enable);
    void setEnabledActions(bool enable);

public slots:

    void slotColorManagement();
   
private:

    KAction *m_redeyeAction;
    KAction *m_BCGAction;
    KAction *m_HSLAction;
    KAction *m_RGBAction;
    KAction *m_autoCorrectionAction;
    KAction *m_invertAction;
    KAction *m_BWAction;
    KAction *m_aspectRatioCropAction;
    KAction *m_sharpenAction;
    KAction *m_blurAction;
    KAction *m_colorManagementAction;
    KAction *m_convertTo8Bits;
    KAction *m_convertTo16Bits;
        
private slots:

    void slotBlur();
    void slotSharpen();
    void slotBCG();
    void slotRGB();
    void slotHSL();
    void slotAutoCorrection();
    void slotInvert();
    
    void slotBW();
    
    void slotRedEye();
    void slotRatioCrop();

    void slotConvertTo8Bits();
    void slotConvertTo16Bits();

};
    
#endif /* IMAGEPLUGIN_CORE_H */
