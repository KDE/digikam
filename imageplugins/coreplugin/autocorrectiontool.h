/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-31
 * Description : Auto-Color correction tool.
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

#ifndef AUTOCORRECTIONTOOL_H
#define AUTOCORRECTIONTOOL_H

// Qt includes.

#include <qstring.h>

// Digikam includes.

#include "editortool.h"

class QHButtonGroup;
class QComboBox;
class QListBox;
class QButtonGroup;

namespace Digikam
{
class HistogramWidget;
class ColorGradientWidget;
class ImageWidget;
class DColor;
class DImg;
}

namespace DigikamImagesPluginCore
{

class AutoCorrectionTool : public Digikam::EditorTool
{
    Q_OBJECT

public:

    AutoCorrectionTool(QObject *parent);
    ~AutoCorrectionTool();

protected:

    void finalRendering();

private slots:

    void slotEffect();
    void slotResetSettings();
    void slotChannelChanged(int channel);
    void slotScaleChanged(int scale);
    void slotColorSelectedFromTarget(const Digikam::DColor &color);

private:

    enum AutoCorrectionType
    {
        AutoLevelsCorrection=0,
        NormalizeCorrection,
        EqualizeCorrection,
        StretchContrastCorrection,
        AutoExposureCorrection
    };

private:

    void readSettings();
    void writeSettings();

    void autoCorrection(uchar *data, int w, int h, bool sb, int type);
    QPixmap getThumbnailForEffect(AutoCorrectionType type);

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

    QListBox                     *m_correctionTools;

    Digikam::ImageWidget         *m_previewWidget;

    Digikam::ColorGradientWidget *m_hGradient;

    Digikam::HistogramWidget     *m_histogramWidget;

    Digikam::DImg                 m_thumbnailImage;
};

}  // NameSpace DigikamImagesPluginCore

#endif /* AUTOCORRECTIONTOOL_H */
