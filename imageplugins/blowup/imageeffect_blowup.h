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
class QTimer;
class QCustomEvent;
class QComboBox;
class QTabWidget;

class KIntNumInput;
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
    
private slots:

    void slotHelp();
    void slotOk();
    void slotCancel();
    void slotDefault();
    void slotUser2();
    void slotUser3();
    void processCImgURL(const QString&);
    void slotAdjustRatioFromWidth(int w);
    void slotAdjustRatioFromHeight(int h);

private:

    enum RunningMode
    {
        NoneRendering=0,
        FinalRendering
    };

    int              m_currentRenderingMode;
    
    double           m_aspectRatio;

    QWidget         *m_parent;
    
    KIntNumInput    *m_newWidth;
    KIntNumInput    *m_newHeight;
    
    QCheckBox       *m_preserveRatioBox;
    
    QTabWidget      *m_mainTab;
    
    KProgress       *m_progressBar;
    
    KAboutData      *m_about;
    
    Digikam::ImageIface                       *m_iface;

    DigikamImagePlugins::GreycstorationIface  *m_cimgInterface;
    DigikamImagePlugins::GreycstorationWidget *m_settingsWidget;
};
    
}  // NameSpace DigikamBlowUpImagesPlugin

#endif /* IMAGEEFFECT_BLOWUP_H */
