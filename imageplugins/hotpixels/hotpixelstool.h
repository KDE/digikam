/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-03-27
 * Description : a digiKam image plugin for fixing dots produced by
 *               hot/stuck/dead pixels from a CCD.
 *
 * Copyright (C) 2005-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef HOTPIXELSTOOL_H
#define HOTPIXELSTOOL_H

#define MAX_PIXEL_DEPTH 4

// Qt includes

#include <QList>

// Local includes

#include "editortool.h"
#include "hotpixel.h"

class KUrl;

namespace DigikamHotPixelsImagesPlugin
{

class HotPixelsToolPriv;

class HotPixelsTool : public Digikam::EditorToolThreaded
{
    Q_OBJECT

public:

    HotPixelsTool(QObject *parent);
    ~HotPixelsTool();

private Q_SLOTS:

    void slotLoadingProgress(float v);
    void slotLoadingComplete();
    void slotBlackFrame(QList<HotPixel> hpList, const KUrl& blackFrameURL);
    void slotAddBlackFrame();
    void slotResetSettings();

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

    HotPixelsToolPriv* const d;
};

}  // namespace DigikamHotPixelsImagesPlugin

#endif /* HOTPIXELSTOOL_H */
