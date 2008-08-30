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

#define MAX_PIXEL_DEPTH    4

// Qt includes.

#include <qvaluelist.h>

// KDE includes.

#include <kurl.h>

// Digikam includes.

#include "editortool.h"

// Local includes.

#include "hotpixelfixer.h"

class QPushButton;

namespace KDcrawIface
{
class RComboBox;
}

namespace Digikam
{
class EditorToolSettings;
class ImagePanelWidget;
}

namespace DigikamHotPixelsImagesPlugin
{

class BlackFrameListView;

class HotPixelsTool : public Digikam::EditorToolThreaded
{
    Q_OBJECT

public:

    HotPixelsTool(QObject *parent);
    ~HotPixelsTool();

private slots:

    void slotBlackFrame(QValueList<HotPixel> hpList, const KURL& blackFrameURL);
    void slotResetSettings();
    void slotAddBlackFrame();
    void slotLoadingProgress(float);
    void slotLoadingComplete();

private:

    void readSettings();
    void writeSettings();
    void prepareEffect();
    void prepareFinal();
    void abortPreview();
    void putPreviewData();
    void putFinalData();
    void renderingFinished();

private:

    QPushButton                 *m_blackFrameButton;

    QValueList<HotPixel>         m_hotPixelsList;

    KURL                         m_blackFrameURL;

    BlackFrameListView          *m_blackFrameListView;

    KDcrawIface::RComboBox      *m_filterMethodCombo;

    Digikam::ImagePanelWidget   *m_previewWidget;

    Digikam::EditorToolSettings *m_gboxSettings;
};

}  // NameSpace DigikamHotPixelsImagesPlugin

#endif /* IMAGEEFFECT_HOTPIXELS_H */
