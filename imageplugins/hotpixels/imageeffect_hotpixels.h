/* ============================================================
* File  : imageeffect_hotpixels.h
* Author: Unai Garro <ugarro at users dot sourceforge dot net>
*         Gilles Caulier <caulier dot gilles at free dot fr>
* Date  : 2005-03-27
* Description : a digiKam image plugin for fixing dots produced by
*               hot/stuck/dead pixels from a CCD
* 
* Copyright 2005-2006 by Unai Garro and Gilles Caulier
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

#ifndef IMAGEEFFECT_HOTPIXELS_H
#define IMAGEEFFECT_HOTPIXELS_H

#define MAX_PIXEL_DEPTH    4

// Qt includes.

#include <qimage.h>

// KDE includes.

#include <kurl.h>

// Digikam includes.

#include <digikamheaders.h>

// Local includes.

#include "hotpixelfixer.h"


class QComboBox;
class QPushButton;

namespace DigikamHotPixelsImagesPlugin
{

class BlackFrameListView;

class ImageEffect_HotPixels : public Digikam::CtrlPanelDlg
{
    Q_OBJECT

public:

    ImageEffect_HotPixels(QWidget *parent,QString title, QFrame* banner);
    ~ImageEffect_HotPixels();

private:

    QComboBox            *m_filterMethodCombo;
    
    QPushButton          *m_blackFrameButton;
    
    QValueList<HotPixel>  m_hotPixelsList;
    
    KURL                  m_blackFrameURL;
    
    BlackFrameListView   *m_blackFrameListView;
    
protected:
    
    void prepareEffect(void);
    void prepareFinal(void);
    void abortPreview(void);
    void putPreviewData(void);
    void putFinalData(void);
    void resetValues(void);   
    void renderingFinished(void);
    
private slots:
        
    void slotBlackFrame(QValueList<HotPixel> hpList, const KURL& blackFrameURL);
    void slotAddBlackFrame(void);
    
private:

    void readSettings(void);
    void writeSettings(void);
};

}  // NameSpace DigikamHotPixelsImagesPlugin

#endif /* IMAGEEFFECT_HOTPIXELS_H */
