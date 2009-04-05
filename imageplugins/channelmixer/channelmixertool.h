/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-02-26
 * Description : image channels mixer.
 *
 * Copyright (C) 2005-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef CHANNELMIXERTOOL_H
#define CHANNELMIXERTOOL_H

// Local includes

#include "editortool.h"

class QCheckBox;
class QPushButton;

namespace KDcrawIface
{
class RDoubleNumInput;
}

namespace Digikam
{
class HistogramWidget;
class ColorGradientWidget;
class ImageWidget;
class DColor;
class EditorToolSettings;
}

namespace DigikamChannelMixerImagesPlugin
{

class ChannelMixerTool : public Digikam::EditorTool
{
    Q_OBJECT

public:

    ChannelMixerTool(QObject *parent);
    ~ChannelMixerTool();

private:

    void readSettings();
    void writeSettings();
    void finalRendering();
    void adjustSliders();

private Q_SLOTS:

    void slotSaveAsSettings();
    void slotLoadSettings();
    void slotResetCurrentChannel();
    void slotResetSettings();
    void slotEffect();
    void slotChannelChanged();
    void slotGainsChanged();
    void slotMonochromeActived(bool mono);
    void slotColorSelectedFromTarget(const Digikam::DColor &color);

private:

    uchar                        *m_destinationPreviewData;

    double                        m_redRedGain;
    double                        m_redGreenGain;
    double                        m_redBlueGain;
    double                        m_greenRedGain;
    double                        m_greenGreenGain;
    double                        m_greenBlueGain;
    double                        m_blueRedGain;
    double                        m_blueGreenGain;
    double                        m_blueBlueGain;
    double                        m_blackRedGain;
    double                        m_blackGreenGain;
    double                        m_blackBlueGain;

    QPushButton                  *m_resetButton;

    QCheckBox                    *m_preserveLuminosity;
    QCheckBox                    *m_monochrome;

    KDcrawIface::RDoubleNumInput *m_redGain;
    KDcrawIface::RDoubleNumInput *m_greenGain;
    KDcrawIface::RDoubleNumInput *m_blueGain;

    Digikam::ImageWidget         *m_previewWidget;

    Digikam::EditorToolSettings  *m_gboxSettings;
};

}  // namespace DigikamChannelMixerImagesPlugin

#endif /* CHANNELMIXERTOOL_H */
