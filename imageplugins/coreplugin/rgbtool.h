/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-07-11
 * Description : digiKam image editor Color Balance tool.
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

#ifndef RGBTOOL_H
#define RGBTOOL_H

// Digikam includes.

#include "editortool.h"

class QComboBox;
class QHButtonGroup;

class QSlider;

namespace KDcrawIface
{
class RIntNumInput;
}

namespace Digikam
{
class HistogramWidget;
class ColorGradientWidget;
class ImageWidget;
class DColor;
}

namespace DigikamImagesPluginCore
{

class RGBTool : public Digikam::EditorTool
{
    Q_OBJECT

public:

    RGBTool(QObject *parent);
    ~RGBTool();

private:

    void writeSettings();
    void readSettings();
    void adjustSliders(int r, int g, int b);
    void finalRendering();

private slots:

    void slotEffect();
    void slotResetSettings();
    void slotChannelChanged(int channel);
    void slotScaleChanged(int scale);
    void slotColorSelectedFromTarget( const Digikam::DColor &color );

private:

    enum HistogramScale
    {
        Linear=0,
        Logarithmic
    };

    enum ColorChannel
    {
        LuminosityChannel=0,
        RedChannel,
        GreenChannel,
        BlueChannel
    };

    uchar                        *m_destinationPreviewData;

    QComboBox                    *m_channelCB;

    QHButtonGroup                *m_scaleBG;

    KDcrawIface::RIntNumInput    *m_rInput;
    KDcrawIface::RIntNumInput    *m_gInput;
    KDcrawIface::RIntNumInput    *m_bInput;

    QSlider                      *m_rSlider;
    QSlider                      *m_gSlider;
    QSlider                      *m_bSlider;

    Digikam::ImageWidget         *m_previewWidget;

    Digikam::ColorGradientWidget *m_hGradient;

    Digikam::HistogramWidget     *m_histogramWidget;

    Digikam::EditorToolSettings  *m_gboxSettings;

};

}  // NameSpace DigikamImagesPluginCore

#endif /* RGBTOOL_H */
