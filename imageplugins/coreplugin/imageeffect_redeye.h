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

#ifndef IMAGEEFFECT_REDEYE_H
#define IMAGEEFFECT_REDEYE_H

// KDE includes.

#include <kpassivepopup.h>

// Digikam includes.

#include "imagedlgbase.h"

class QButtonGroup;
class QColor;
class QComboBox;
class QLabel;

class KHueSaturationSelector;
class KColorValueSelector;

namespace KDcrawIface
{
class RIntNumInput;
}

namespace Digikam
{
class ColorGradientWidget;
class DColor;
class DImg;
class HistogramWidget;
class ImageWidget;
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

class ImageEffect_RedEye : public Digikam::ImageDlgBase
{
    Q_OBJECT

public:

    ImageEffect_RedEye(QWidget *parent);
    ~ImageEffect_RedEye();

private slots:

    void slotEffect();
    void slotChannelChanged(int channel);
    void slotScaleChanged(int scale);
    void slotColorSelectedFromTarget(const Digikam::DColor &color);
    void slotHSChanged(int h, int s);
    void slotVChanged(int v);

private:

    void readUserSettings();
    void writeUserSettings();
    void resetValues();
    void finalRendering();
    void redEyeFilter(Digikam::DImg& selection);
    void setColor(QColor color);

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

    QColor                        m_selColor;

    QLabel                       *m_thresholdLabel;
    QLabel                       *m_smoothLabel;

    QComboBox                    *m_channelCB;

    QButtonGroup                 *m_scaleBG;

    KHueSaturationSelector       *m_HSSelector;
    KColorValueSelector          *m_VSelector;

    KDcrawIface::RIntNumInput    *m_tintLevel;
    KDcrawIface::RIntNumInput    *m_redThreshold;
    KDcrawIface::RIntNumInput    *m_smoothLevel;

    Digikam::ImageWidget         *m_previewWidget;

    Digikam::ColorGradientWidget *m_hGradient;

    Digikam::HistogramWidget     *m_histogramWidget;
};

}  // NameSpace DigikamImagesPluginCore

#endif /* IMAGEEFFECT_REDEYE_H */
