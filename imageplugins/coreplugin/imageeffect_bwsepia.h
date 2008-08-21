/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-12-06
 * Description : Black and White conversion tool.
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

#ifndef IMAGEEFFECT_BWSEPIA_H
#define IMAGEEFFECT_BWSEPIA_H

// Qt includes.

#include <QString>

// Digikam includes.

#include "imagedlgbase.h"

class QListWidget;
class QButtonGroup;
class QComboBox;

class KIntNumInput;
class KTabWidget;

namespace Digikam
{
class HistogramWidget;
class ColorGradientWidget;
class ImageWidget;
class DColor;
class DImg;
class CurvesWidget;
}

namespace DigikamImagesPluginCore
{

class PreviewPixmapFactory;

class ImageEffect_BWSepia : public Digikam::ImageDlgBase
{
    Q_OBJECT

public:

    ImageEffect_BWSepia(QWidget *parent);
    ~ImageEffect_BWSepia();

    friend class PreviewPixmapFactory;

protected:

    QPixmap getThumbnailForEffect(int type);
    void finalRendering();

protected slots:

    virtual void slotTimer();

private:

    void readUserSettings();
    void writeUserSettings();
    void resetValues();
    void blackAndWhiteConversion(uchar *data, int w, int h, bool sb, int type);
    void updatePreviews();

private slots:

    void slotUser2();
    void slotUser3();
    void slotEffect();
    void slotChannelChanged(int channel);
    void slotScaleChanged(int scale);
    void slotSpotColorChanged(const Digikam::DColor &color);
    void slotColorSelectedFromTarget(const Digikam::DColor& color);
    void slotFilterSelected();

private:

    enum BlackWhiteConversionType
    {
        BWNoFilter=0,         // B&W filter to the front of lens.
        BWGreenFilter,
        BWOrangeFilter,
        BWRedFilter,
        BWYellowFilter,

        BWGeneric,            // B&W film simulation.
        BWAgfa200X,
        BWAgfapan25,
        BWAgfapan100,
        BWAgfapan400,
        BWIlfordDelta100,
        BWIlfordDelta400,
        BWIlfordDelta400Pro3200,
        BWIlfordFP4,
        BWIlfordHP5,
        BWIlfordPanF,
        BWIlfordXP2Super,
        BWKodakTmax100,
        BWKodakTmax400,
        BWKodakTriX,

        BWNoTone,             // Chemical color tone filter.
        BWSepiaTone,
        BWBrownTone,
        BWColdTone,
        BWSeleniumTone,
        BWPlatinumTone,
        BWGreenTone
    };

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

    enum SettingsTab
    {
        FilmTab=0,
        BWFiltersTab,
        ToneTab,
        LuminosityTab
    };

    // Color filter attenuation in percents. 
    double m_redAttn, m_greenAttn, m_blueAttn;

    // Channel mixer color multiplier. 
    double m_redMult, m_greenMult, m_blueMult; 

    uchar                        *m_destinationPreviewData;

    QComboBox                    *m_channelCB;

    QButtonGroup                 *m_scaleBG;

    QListWidget                  *m_bwFilters;
    QListWidget                  *m_bwFilm;
    QListWidget                  *m_bwTone;

    KIntNumInput                 *m_cInput;
    KIntNumInput                 *m_strengthInput;

    KTabWidget                   *m_tab;

    Digikam::ImageWidget         *m_previewWidget;

    Digikam::ColorGradientWidget *m_hGradient;

    Digikam::HistogramWidget     *m_histogramWidget;

    Digikam::CurvesWidget        *m_curvesWidget;

    Digikam::DImg                *m_originalImage;
    Digikam::DImg                 m_thumbnailImage;

    PreviewPixmapFactory         *m_previewPixmapFactory;
};

}  // NameSpace DigikamImagesPluginCore

#endif /* IMAGEEFFECT_BWSEPIA_H */
