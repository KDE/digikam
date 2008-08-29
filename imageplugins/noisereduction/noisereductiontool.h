/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-08-24
 * Description : a plugin to reduce CCD noise.
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

#ifndef NOISEREDUCTIONTOOL_H
#define NOISEREDUCTIONTOOL_H

// Local includes.

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

namespace DigikamNoiseReductionImagesPlugin
{

class NoiseReductionTool : public Digikam::EditorToolThreaded
{
    Q_OBJECT

public:

    NoiseReductionTool(QObject* parent);
    ~NoiseReductionTool();

private:

    void readSettings();
    void writeSettings();
    void prepareEffect();
    void prepareFinal();
    void putPreviewData();
    void putFinalData();
    void renderingFinished();

private slots:

    void slotSaveAsSettings();
    void slotLoadSettings();
    void slotResetSettings();

private:

    KDcrawIface::RDoubleNumInput *m_radiusInput;
    KDcrawIface::RDoubleNumInput *m_lumToleranceInput;
    KDcrawIface::RDoubleNumInput *m_thresholdInput;
    KDcrawIface::RDoubleNumInput *m_textureInput;
    KDcrawIface::RDoubleNumInput *m_sharpnessInput;

    KDcrawIface::RDoubleNumInput *m_csmoothInput;
    KDcrawIface::RDoubleNumInput *m_lookaheadInput;
    KDcrawIface::RDoubleNumInput *m_gammaInput;
    KDcrawIface::RDoubleNumInput *m_dampingInput;
    KDcrawIface::RDoubleNumInput *m_phaseInput;

    Digikam::ImagePanelWidget    *m_previewWidget;

    Digikam::EditorToolSettings  *m_gboxSettings;
};

}  // NameSpace DigikamNoiseReductionImagesPlugin

#endif /* NOISEREDUCTIONTOOL_H */
