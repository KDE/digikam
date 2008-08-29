/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-02-14
 * Description : a digiKam image plugin for to apply a color
 *               effect to an image.
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef COLORFXTOOL_H
#define COLORFXTOOL_H

// Digikam includes.

#include "editortool.h"

class QHButtonGroup;
class QComboBox;
class QLabel;

namespace KDcrawIface
{
class RIntNumInput;
class RComboBox;
}

namespace Digikam
{
class ImageWidget;
class ColorGradientWidget;
class HistogramWidget;
class DColor;
}

namespace DigikamColorFXImagesPlugin
{

class ColorFXTool : public Digikam::EditorTool
{
    Q_OBJECT

public:

    ColorFXTool(QObject *parent);
    ~ColorFXTool();

private:

    void readSettings();
    void writeSettings();
    void finalRendering();
    void colorEffect(uchar *data, int w, int h, bool sb);
    void solarize(int factor, uchar *data, int w, int h, bool sb);
    void vivid(int factor, uchar *data, int w, int h, bool sb);
    void neon(uchar *data, int w, int h, bool sb, int Intensity, int BW);
    void findEdges(uchar *data, int w, int h, bool sb, int Intensity, int BW);
    void neonFindEdges(uchar *data, int w, int h, bool sb, bool neon, int Intensity, int BW);

    inline int getOffset(int Width, int X, int Y, int bytesDepth);
    inline int Lim_Max(int Now, int Up, int Max);

private slots:

    void slotEffectTypeChanged(int type);
    void slotEffect();
    void slotResetSettings();
    void slotChannelChanged(int channel);
    void slotScaleChanged(int scale);
    void slotColorSelectedFromTarget(const Digikam::DColor &color);

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

    enum ColorFXTypes
    {
        Solarize=0,
        Vivid,
        Neon,
        FindEdges
    };

    uchar                        *m_destinationPreviewData;

    QComboBox                    *m_channelCB;

    QHButtonGroup                *m_scaleBG;

    QLabel                       *m_effectTypeLabel;
    QLabel                       *m_levelLabel;
    QLabel                       *m_iterationLabel;

    KDcrawIface::RIntNumInput    *m_levelInput;
    KDcrawIface::RIntNumInput    *m_iterationInput;

    KDcrawIface::RComboBox       *m_effectType;

    Digikam::ImageWidget         *m_previewWidget;

    Digikam::ColorGradientWidget *m_hGradient;

    Digikam::HistogramWidget     *m_histogramWidget;
};

}  // NameSpace DigikamColorFXImagesPlugin

#endif /* COLORFXTOOL_H */
