/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-03-27
 * Description : a digiKam image plugin for fixing dots produced by
 *               hot/stuck/dead pixels from a CCD.
 * 
 * Copyright (C) 2005-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2005-2006 by Unai Garro <ugarro at users dot sourceforge dot net>
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

#define MAX_PIXEL_DEPTH 4

// Qt includes.

#include <Q3ValueList>

// KDE includes.

#include <kurl.h>

// Digikam includes.

#include "ctrlpaneldlg.h"

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

    ImageEffect_HotPixels(QWidget *parent);
    ~ImageEffect_HotPixels();

private slots:
        
    void slotBlackFrame(Q3ValueList<HotPixel> hpList, const KUrl& blackFrameURL);
    void slotAddBlackFrame();
    void readUserSettings();

private:

    void writeUserSettings();
    void resetValues();    
    void prepareEffect();
    void prepareFinal();
    void abortPreview();
    void putPreviewData();
    void putFinalData();
    void renderingFinished();

private:

    QComboBox             *m_filterMethodCombo;
    
    QPushButton           *m_blackFrameButton;
    
    Q3ValueList<HotPixel>  m_hotPixelsList;
    
    KUrl                   m_blackFrameURL;
    
    BlackFrameListView    *m_blackFrameListView;
};

}  // NameSpace DigikamHotPixelsImagesPlugin

#endif /* IMAGEEFFECT_HOTPIXELS_H */
