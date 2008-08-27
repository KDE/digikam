/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-11-28
 * Description : a digiKam image editor plugin to process image
 *               free rotation.
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

#ifndef FREEROTATIONTOOL_H
#define FREEROTATIONTOOL_H

// Local includes.

#include "editortool.h"

class QFrame;
class QLabel;
class QCheckBox;

namespace KDcrawIface
{
class RIntNumInput;
class RDoubleNumInput;
class RComboBox;
}

namespace Digikam
{
class EditorToolSettings;
class ImageWidget;
}

namespace DigikamFreeRotationImagesPlugin
{

class FreeRotationTool : public Digikam::EditorToolThreaded
{
    Q_OBJECT

public:

    FreeRotationTool(QObject *parent);
    ~FreeRotationTool();

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

    QLabel                       *m_newWidthLabel;
    QLabel                       *m_newHeightLabel;

    QCheckBox                    *m_antialiasInput;

    KDcrawIface::RComboBox       *m_autoCropCB;

    KDcrawIface::RIntNumInput    *m_angleInput;

    KDcrawIface::RDoubleNumInput *m_fineAngleInput;

    Digikam::ImageWidget         *m_previewWidget;

    Digikam::EditorToolSettings  *m_gboxSettings;
};

}  // NameSpace DigikamFreeRotationImagesPlugin

#endif /* FREEROTATIONTOOL_H */
