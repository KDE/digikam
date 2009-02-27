/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-07-11
 * Description : digiKam image editor Color Balance tool.
 *
 * Copyright (C) 2004-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef RGBTOOL_H
#define RGBTOOL_H

// Local includes.

#include "editortool.h"

class QButtonGroup;
class QSlider;

class KComboBox;

namespace KDcrawIface
{
class RIntNumInput;
}

namespace Digikam
{
class HistogramWidget;
class ImageWidget;
class DColor;
class EditorToolSettings;
}

namespace DigikamImagesPluginCore
{

class RGBTool : public Digikam::EditorTool
{
    Q_OBJECT

public:

    RGBTool(QObject* parent);
    ~RGBTool();

private:

    void writeSettings();
    void readSettings();
    void slotResetSettings();
    void adjustSliders(int r, int g, int b);
    void finalRendering();

private Q_SLOTS:

    void slotEffect();
    void slotColorSelectedFromTarget( const Digikam::DColor &color );

private:

    uchar                        *m_destinationPreviewData;

    QSlider                      *m_rSlider;
    QSlider                      *m_gSlider;
    QSlider                      *m_bSlider;

    KComboBox                    *m_channelCB;

    KDcrawIface::RIntNumInput    *m_rInput;
    KDcrawIface::RIntNumInput    *m_gInput;
    KDcrawIface::RIntNumInput    *m_bInput;

    Digikam::ImageWidget         *m_previewWidget;

    Digikam::EditorToolSettings  *m_gboxSettings;
};

}  // namespace DigikamImagesPluginCore

#endif /* RGBTOOL_H */
