/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-07-09
 * Description : a tool to blur an image
 *
 * Copyright (C) 2004-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef IMAGEEFFECT_BLUR_H
#define IMAGEEFFECT_BLUR_H

// Digikam includes.

#include "editortool.h"

namespace KDcrawIface
{
class RIntNumInput;
}

namespace Digikam
{
class EditorToolSettings;
class ImagePanelWidget;
}

namespace DigikamImagesPluginCore
{

class BlurTool : public Digikam::EditorToolThreaded
{
    Q_OBJECT

public:

    BlurTool(QObject *parent);
    ~BlurTool();

private slots:

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

    KDcrawIface::RIntNumInput   *m_radiusInput;

    Digikam::ImagePanelWidget   *m_previewWidget;

    Digikam::EditorToolSettings *m_gboxSettings;
};

}  // NameSpace DigikamImagesPluginCore

#endif /* IMAGEEFFECT_BLUR_H */
