/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at gmail dot com>
 * Date   : 2005-04-07
 * Description : a digiKam image editor plugin to blowup 
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

#ifndef IMAGEEFFECT_BLOWUP_H
#define IMAGEEFFECT_BLOWUP_H

// Qt include.

#include <qstring.h>

// KDE include.

#include <kdialogbase.h>

class QCheckBox;
class QCustomEvent;
class QTabWidget;

class KIntNumInput;
class KDoubleNumInput;
class KProgress;
class KAboutData;

namespace DigikamImagePlugins
{
class GreycstorationIface;
class GreycstorationWidget;
}

namespace Digikam
{
class ImageIface;
}

namespace DigikamBlowUpImagesPlugin
{

class ImageEffect_BlowUp : public KDialogBase
{
    Q_OBJECT

public:

    ImageEffect_BlowUp(QWidget* parent);
    ~ImageEffect_BlowUp();
       
protected:

    void closeEvent(QCloseEvent *e);
    
private:

    void customEvent(QCustomEvent *event);
    void writeUserSettings();

private slots:

    void slotHelp();
    void slotOk();
    void slotCancel();
    void slotDefault();
    void slotUser2();
    void slotUser3();
    void processCImgURL(const QString&);
    void slotValuesChanged();
    void readUserSettings();            

private:

    enum RunningMode
    {
        NoneRendering=0,
        FinalRendering
    };

    int              m_currentRenderingMode;
    int              m_orgWidth;    
    int              m_orgHeight;   
    int              m_prevW; 
    int              m_prevH; 

    double           m_prevWP;    
    double           m_prevHP;    
    double           m_aspectRatio;

    QWidget         *m_parent;
    
    QCheckBox       *m_preserveRatioBox;
    
    QTabWidget      *m_mainTab;

    KIntNumInput    *m_wInput;
    KIntNumInput    *m_hInput;

    KDoubleNumInput *m_wpInput;
    KDoubleNumInput *m_hpInput;
    
    KProgress       *m_progressBar;
    
    KAboutData      *m_about;
    
    Digikam::ImageIface                       *m_iface;

    DigikamImagePlugins::GreycstorationIface  *m_cimgInterface;
    DigikamImagePlugins::GreycstorationWidget *m_settingsWidget;
};
    
}  // NameSpace DigikamBlowUpImagesPlugin

#endif /* IMAGEEFFECT_BLOWUP_H */
