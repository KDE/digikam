/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at gmail dot com>
 * Date   : 2005-03-30
 * Description : a digiKam image editor plugin to inpaint 
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

#ifndef IMAGEEFFECT_INPAINTING_H
#define IMAGEEFFECT_INPAINTING_H

// Qt include.

#include <qimage.h>
#include <qrect.h>
#include <qstring.h>

// KDE include.

#include <kdialogbase.h>

// Digikam includes.

#include <digikamheaders.h>

class QPushButton;
class QTimer;
class QCustomEvent;
class QTabWidget;
class QFrame;

class KProgress;

namespace DigikamImagePlugins
{
class GreycstorationWidget;
class GreycstorationIface;
}

namespace Digikam
{
class ImageIface;
}

namespace DigikamInPaintingImagesPlugin
{

class ImageEffect_InPainting
{
public:

    static void inPainting(QWidget *parent);
};

//-----------------------------------------------------------

class ImageEffect_InPainting_Dialog : public KDialogBase
{
    Q_OBJECT

public:

    ImageEffect_InPainting_Dialog(QWidget* parent);
    ~ImageEffect_InPainting_Dialog();
       
protected:

    void closeEvent(QCloseEvent *e);
    
private:

    enum InPaintingFilteringPreset
    {
        NoPreset=0,
        RemoveSmallArtefact,
        RemoveMediumArtefact,
        RemoveLargeArtefact
    };
    
    enum RunningMode
    {
        NoneRendering=0,
        FinalRendering
    };

    int              m_currentRenderingMode;
    
    QRect            m_maskRect;
    
    QImage           m_maskImage;
        
    QWidget         *m_parent;
    
    QPushButton     *m_helpButton;
    
    // Preset Settings.
    QComboBox       *m_inpaintingTypeCB;  
    
    QTabWidget      *m_mainTab;
    
    KProgress       *m_progressBar;

    KAboutData      *m_about;
    
    DigikamImagePlugins::GreycstorationWidget *m_settingsWidget;

    DigikamImagePlugins::GreycstorationIface  *m_greycstorationIface;
    
    Digikam::ImageIface                       *m_iface;    

    Digikam::DImg                              m_originalImage;
    Digikam::DImg                              m_cropImage;
    Digikam::DImg                              m_previewImage;
    
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
};
    
}  // NameSpace DigikamInPaintingImagesPlugin

#endif /* IMAGEEFFECT_INPAINTING_H */
