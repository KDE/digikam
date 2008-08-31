/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-25
 * Description : a digiKam image plugin to reduce
 *               vignetting on an image.
 *
 * Copyright (C) 2005-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef ANTIVIGNETTINGTOOL_H
#define ANTIVIGNETTINGTOOL_H

// Digikam includes.

#include "editortool.h"

class QLabel;

namespace KDcrawIface
{
class RIntNumInput;
class RDoubleNumInput;
}

namespace Digikam
{
class EditorToolSettings;
class ImageWidget;
}

namespace DigikamAntiVignettingImagesPlugin
{

class AntiVignettingTool : public Digikam::EditorToolThreaded
{
    Q_OBJECT

public:

    AntiVignettingTool(QObject *parent);
    ~AntiVignettingTool();

private slots:

    void slotResetSettings();

private:

    void writeSettings();
    void readSettings();
    void prepareEffect();
    void prepareFinal();
    void putPreviewData();
    void putFinalData();
    void renderingFinished();

private:

    QLabel                       *m_maskPreviewLabel;

    KDcrawIface::RIntNumInput    *m_brightnessInput;
    KDcrawIface::RIntNumInput    *m_contrastInput;

    KDcrawIface::RDoubleNumInput *m_gammaInput;
    KDcrawIface::RDoubleNumInput *m_densityInput;
    KDcrawIface::RDoubleNumInput *m_powerInput;
    KDcrawIface::RDoubleNumInput *m_radiusInput;

    Digikam::ImageWidget         *m_previewWidget;

    Digikam::EditorToolSettings  *m_gboxSettings;
};

}  // NameSpace DigikamAntiVignettingImagesPlugin

#endif /* ANTIVIGNETTINGTOOL_H */
