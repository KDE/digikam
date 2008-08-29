/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-06-06
 * Description : Red eyes correction tool for image editor
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
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


#ifndef REDEYETOOL_H
#define REDEYETOOL_H

// KDE includes.

#include <kpassivepopup.h>

// Digikam includes.

#include "editortool.h"

class QLabel;
class QComboBox;
class QHButtonGroup;

class KHSSelector;
class KValueSelector;

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
class DImg;
}

namespace DigikamImagesPluginCore
{

class RedEyePassivePopup : public KPassivePopup
{
public:

    RedEyePassivePopup(QWidget* parent)
        : KPassivePopup(parent), m_parent(parent)
    {
    }

protected:

    virtual void positionSelf()
    {
        move(m_parent->x() + 30, m_parent->y() + 30);
    }

private:

    QWidget* m_parent;
};

// ----------------------------------------------------------------

class RedEyeTool : public Digikam::EditorTool
{
    Q_OBJECT

public:

    RedEyeTool(QObject *parent);
    ~RedEyeTool();

private slots:

    void slotEffect();
    void slotResetSettings();
    void slotChannelChanged(int channel);
    void slotScaleChanged(int scale);
    void slotColorSelectedFromTarget(const Digikam::DColor &color);
    void slotHSChanged(int h, int s);

private:

    void readSettings();
    void writeSettings();
    void finalRendering();
    void redEyeFilter(Digikam::DImg& selection);

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

    enum RedThresold
    {
        Mild=0,
        Aggressive
    };

    uchar                        *m_destinationPreviewData;

    QLabel                       *m_thresholdLabel;
    QLabel                       *m_smoothLabel;

    QComboBox                    *m_channelCB;

    QHButtonGroup                *m_scaleBG;

    KDcrawIface::RIntNumInput    *m_tintLevel;
    KDcrawIface::RIntNumInput    *m_redThreshold;
    KDcrawIface::RIntNumInput    *m_smoothLevel;

    KHSSelector                  *m_HSSelector;
    KValueSelector               *m_VSelector;

    Digikam::ImageWidget         *m_previewWidget;

    Digikam::ColorGradientWidget *m_hGradient;

    Digikam::HistogramWidget     *m_histogramWidget;
};

}  // NameSpace DigikamImagesPluginCore

#endif /* REDEYETOOL_H */
