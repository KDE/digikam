/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-07-16
 * Description : digiKam image editor to adjust Hue, Saturation,
 *               and Lightness of picture.
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

#ifndef HSLTOOL_H
#define HSLTOOL_H

// Local includes.

#include "editortool.h"

class KHueSaturationSelector;

namespace KDcrawIface
{
class RDoubleNumInput;
}

namespace Digikam
{
class ColorGradientWidget;
class DColor;
class EditorToolSettings;
class ImageWidget;
}

namespace DigikamImagesPluginCore
{
class HSPreviewWidget;

class HSLTool : public Digikam::EditorTool
{
    Q_OBJECT

public:

    HSLTool(QObject *parent);
    ~HSLTool();

private Q_SLOTS:

    void slotEffect();
    void slotResetSettings();
    void slotColorSelectedFromTarget( const Digikam::DColor &color );
    void slotHSChanged(int h, int s);
    void slotHChanged(double h);
    void slotSChanged(double s);

private:

    void writeSettings();
    void readSettings();
    void finalRendering();

private:

    uchar                        *m_destinationPreviewData;

    KHueSaturationSelector       *m_HSSelector;

    KDcrawIface::RDoubleNumInput *m_hInput;
    KDcrawIface::RDoubleNumInput *m_sInput;
    KDcrawIface::RDoubleNumInput *m_lInput;

    HSPreviewWidget              *m_HSPreview;

    Digikam::ImageWidget         *m_previewWidget;

    Digikam::EditorToolSettings  *m_gboxSettings;
};

}  // namespace DigikamImagesPluginCore

#endif /* HSLTOOL_H */
