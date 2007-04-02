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

// Digikam includes.

#include "dimg.h"
#include "imageguidedlg.h"

class QTabWidget;
class QComboBox;

namespace Digikam
{
class GreycstorationWidget;
}

namespace DigikamInPaintingImagesPlugin
{

class ImageEffect_InPainting
{
public:

    static void inPainting(QWidget *parent);
};

//-----------------------------------------------------------

class ImageEffect_InPainting_Dialog : public Digikam::ImageGuideDlg
{
    Q_OBJECT

public:

    ImageEffect_InPainting_Dialog(QWidget* parent);
    ~ImageEffect_InPainting_Dialog();
       
private slots:

    void slotUser2();
    void slotUser3();
    void readUserSettings();
    void processCImgURL(const QString&);
    void slotResetValues(int);

private:

    void writeUserSettings();
    void resetValues();     
    void prepareEffect();
    void prepareFinal();
    void putPreviewData();
    void putFinalData();
    void renderingFinished();

private:

    enum InPaintingFilteringPreset
    {
        NoPreset=0,
        RemoveSmallArtefact,
        RemoveMediumArtefact,
        RemoveLargeArtefact
    };
    
    bool                           m_isComputed;

    QRect                          m_maskRect;
    
    QImage                         m_maskImage;
    
    QComboBox                     *m_inpaintingTypeCB;  
    
    QTabWidget                    *m_mainTab;
    
    Digikam::DImg                  m_originalImage;
    Digikam::DImg                  m_cropImage;

    Digikam::GreycstorationWidget *m_settingsWidget;
};
    
}  // NameSpace DigikamInPaintingImagesPlugin

#endif /* IMAGEEFFECT_INPAINTING_H */
