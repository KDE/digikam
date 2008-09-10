/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-12-27
 * Description : a plugin to reduce lens distorsions to an image.
 *
 * Copyright (C) 2004-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef LENSDISTORTIONTOOL_H
#define LENSDISTORTIONTOOL_H

// Digikam includes.

#include "dimg.h"
#include "editortool.h"

class QLabel;

namespace KDcrawIface
{
class RDoubleNumInput;
}

namespace Digikam
{
class EditorToolSettings;
class ImageWidget;
}

namespace DigikamLensDistortionImagesPlugin
{

class LensDistortionTool : public Digikam::EditorToolThreaded
{
    Q_OBJECT

public:

    LensDistortionTool(QObject *parent);
    ~LensDistortionTool();

private slots:

    void slotResetSettings();
    void slotColorGuideChanged();

private:

    void readSettings();
    void writeSettings();
    void prepareEffect();
    void prepareFinal();
    void putPreviewData();
    void putFinalData();
    void renderingFinished();

private:

    QLabel                       *m_maskPreviewLabel;

    KDcrawIface::RDoubleNumInput *m_mainInput;
    KDcrawIface::RDoubleNumInput *m_edgeInput;
    KDcrawIface::RDoubleNumInput *m_rescaleInput;
    KDcrawIface::RDoubleNumInput *m_brightenInput;

    Digikam::DImg                 m_previewRasterImage;

    Digikam::ImageWidget         *m_previewWidget;

    Digikam::EditorToolSettings  *m_gboxSettings;
};

}  // NameSpace DigikamLensDistortionImagesPlugin

#endif /* LENSDISTORTIONTOOL_H */
