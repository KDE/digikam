/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-02-11
 * Description : a plugin to apply Distortion FX to an image.
 *
 * Copyright (C) 2005-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * Original Distortion algorithms copyrighted 2004-2005 by
 * Pieter Z. Voloshyn <pieter dot voloshyn at gmail dot com>.
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

#ifndef DISTORTIONFXTOOL_H
#define DISTORTIONFXTOOL_H

// Digikam includes.

#include "editortool.h"

class QLabel;

namespace KDcrawIface
{
class RIntNumInput;
class RComboBox;
}

namespace Digikam
{
class ImageWidget;
}

namespace DigikamDistortionFXImagesPlugin
{

class DistortionFXTool : public Digikam::EditorToolThreaded
{
    Q_OBJECT

public:

    DistortionFXTool(QObject *parent);
    ~DistortionFXTool();

private slots:

    void slotEffectTypeChanged(int type);
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


    QLabel                      *m_effectTypeLabel;
    QLabel                      *m_levelLabel;
    QLabel                      *m_iterationLabel;

    KDcrawIface::RComboBox      *m_effectType;

    KDcrawIface::RIntNumInput   *m_levelInput;
    KDcrawIface::RIntNumInput   *m_iterationInput;

    Digikam::ImageWidget        *m_previewWidget;

    Digikam::EditorToolSettings *m_gboxSettings;
};

}  // NameSpace DigikamDistortionFXImagesPlugin

#endif /* DISTORTIONFXTOOL_H */
