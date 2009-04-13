/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-07-09
 * Description : a tool to blur an image
 *
 * Copyright (C) 2004-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009      by Andi Clemens <andi dot clemens at gmx dot net>
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

#ifndef BLURTOOL_H
#define BLURTOOL_H

// Local includes

#include "editortool.h"

namespace KDcrawIface
{
class RDoubleNumInput;
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

private Q_SLOTS:

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

    KDcrawIface::RDoubleNumInput   *m_radiusInput;

    Digikam::ImagePanelWidget      *m_previewWidget;

    Digikam::EditorToolSettings    *m_gboxSettings;
};

}  // namespace DigikamImagesPluginCore

#endif /* BLURTOOL_H */
