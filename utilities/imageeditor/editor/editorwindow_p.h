/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-01-20
 * Description : main image editor GUI implementation
 *               private data.
 *
 * Copyright (C) 2006-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef EDITORWINDOWPRIVATE_H
#define EDITORWINDOWPRIVATE_H

// Qt includes

#include <qlist.h>
#include <qstring.h>

class QDialog;
class QEventLoop;
class QLabel;
class QSignalMapper;
class QToolButton;
class QWidgetAction;

class KAction;
class KActionCollection;
class KSqueezedTextLabel;
class KToggleAction;

namespace Digikam
{

class DZoomBar;
class EditorToolIface;
class ExposureSettingsContainer;
class ICCSettingsContainer;
class PreviewToolBar;

class EditorWindowPriv
{

public:

    EditorWindowPriv()
    {
        configGroupName                          = "ImageViewer Settings";
        configAutoBrightnessEntry                = "AutoBrightness";
        configAutoZoomEntry                      = "AutoZoom";
        configBackgroundColorEntry               = "BackgroundColor";
        configCaBlueMultiplierEntry              = "caBlueMultiplier";
        configCaRedMultiplierEntry               = "caRedMultiplier";
        configCustomWhiteBalanceEntry            = "CustomWhiteBalance";
        configCustomWhiteBalanceGreenEntry       = "CustomWhiteBalanceGreen";
        configDontStretchPixelsEntry             = "DontStretchPixels";
        configEnableCACorrectionEntry            = "EnableCACorrection";
        configEnableNoiseReductionEntry          = "EnableNoiseReduction";
        configFullScreenEntry                    = "FullScreen";
        configFullScreenHideThumbBarEntry        = "FullScreenHideThumbBar";
        configFullScreenHideToolBarEntry         = "FullScreen Hide ToolBar";
        configJpeg2000CompressionEntry           = "JPEG2000Compression";
        configJpeg2000LossLessEntry              = "JPEG2000LossLess";
        configJpegCompressionEntry               = "JPEGCompression";
        configJpegSubSamplingEntry               = "JPEGSubSampling";
        configMedianFilterPassesEntry            = "MedianFilterPasses";
        configNRThresholdEntry                   = "NRThreshold";
        configOverExposureColorEntry             = "OverExposureColor";
        configOverExposureIndicatorEntry         = "OverExposureIndicator";
        configPgfCompressionEntry                = "PGFCompression";
        configPgfLossLessEntry                   = "PGFLossLess";
        configPngCompressionEntry                = "PNGCompression";
        configRAWBrightnessEntry                 = "RAWBrightness";
        configRAWQualityEntry                    = "RAWQuality";
        configRGBInterpolate4ColorsEntry         = "RGBInterpolate4Colors";
        configSixteenBitsImageEntry              = "SixteenBitsImage";
        configSlideShowDelayEntry                = "SlideShowDelay";
        configSlideShowLoopEntry                 = "SlideShowLoop";
        configSlideShowPrintApertureFocalEntry   = "SlideShowPrintApertureFocal";
        configSlideShowPrintCommentEntry         = "SlideShowPrintComment";
        configSlideShowPrintDateEntry            = "SlideShowPrintDate";
        configSlideShowPrintExpoSensitivityEntry = "SlideShowPrintExpoSensitivity";
        configSlideShowPrintMakeModelEntry       = "SlideShowPrintMakeModel";
        configSlideShowPrintNameEntry            = "SlideShowPrintName";
        configSlideShowPrintRatingEntry          = "SlideShowPrintRating";
        configSlideShowStartCurrentEntry         = "SlideShowStartCurrent";
        configSplitterStateEntry                 = "SplitterState";
        configTiffCompressionEntry               = "TIFFCompression";
        configUnclipColorsEntry                  = "UnclipColors";
        configUnderExposureColorEntry            = "UnderExposureColor";
        configUnderExposureIndicatorEntry        = "UnderExposureIndicator";
        configUseRawImportToolEntry              = "UseRawImportTool";
        configUseThemeBackgroundColorEntry       = "UseThemeBackgroundColor";
        configVerticalSplitterSizesEntry         = "Vertical Splitter Sizes";
        configVerticalSplitterStateEntry         = "Vertical Splitter State";
        configWhiteBalanceEntry                  = "WhiteBalance";
        removeFullScreenButton       = false;
        fullScreenHideToolBar        = false;
        infoLabel                    = 0;
        donateMoneyAction            = 0;
        viewCMViewAction             = 0;
        filePrintAction              = 0;
        copyAction                   = 0;
        cropAction                   = 0;
        rotateLeftAction             = 0;
        rotateRightAction            = 0;
        flipHorizAction              = 0;
        flipVertAction               = 0;
        ICCSettings                  = 0;
        exposureSettings             = 0;
        underExposureIndicator       = 0;
        overExposureIndicator        = 0;
        cmViewIndicator              = 0;
        viewUnderExpoAction          = 0;
        viewOverExpoAction           = 0;
        slideShowAction              = 0;
        zoomFitToWindowAction        = 0;
        zoomFitToSelectAction        = 0;
        zoomPlusAction               = 0;
        zoomMinusAction              = 0;
        zoomTo100percents            = 0;
        selectAllAction              = 0;
        selectNoneAction             = 0;
        waitingLoop                  = 0;
        currentWindowModalDialog     = 0;
        undoSignalMapper             = 0;
        redoSignalMapper             = 0;
        rawCameraListAction          = 0;
        libsInfoAction               = 0;
        contributeAction             = 0;
        toolIface                    = 0;
        showMenuBarAction            = 0;
        imagepluginsActionCollection = 0;
        viewSoftProofAction          = 0;
        softProofOptionsAction       = 0;
        previewToolBar               = 0;
        zoomBar                      = 0;
    }

    ~EditorWindowPriv()
    {
    }

    QString                    configGroupName;
    QString                    configAutoBrightnessEntry;
    QString                    configAutoZoomEntry;
    QString                    configBackgroundColorEntry;
    QString                    configCaBlueMultiplierEntry;
    QString                    configCaRedMultiplierEntry;
    QString                    configCustomWhiteBalanceEntry;
    QString                    configCustomWhiteBalanceGreenEntry;
    QString                    configDontStretchPixelsEntry;
    QString                    configEnableCACorrectionEntry;
    QString                    configEnableNoiseReductionEntry;
    QString                    configFullScreenEntry;
    QString                    configFullScreenHideThumbBarEntry;
    QString                    configFullScreenHideToolBarEntry;
    QString                    configJpeg2000CompressionEntry;
    QString                    configJpeg2000LossLessEntry;
    QString                    configJpegCompressionEntry;
    QString                    configJpegSubSamplingEntry;
    QString                    configMedianFilterPassesEntry;
    QString                    configNRThresholdEntry;
    QString                    configOverExposureColorEntry;
    QString                    configOverExposureIndicatorEntry;
    QString                    configPgfCompressionEntry;
    QString                    configPgfLossLessEntry;
    QString                    configPngCompressionEntry;
    QString                    configRAWBrightnessEntry;
    QString                    configRAWQualityEntry;
    QString                    configRGBInterpolate4ColorsEntry;
    QString                    configSixteenBitsImageEntry;
    QString                    configSlideShowDelayEntry;
    QString                    configSlideShowLoopEntry;
    QString                    configSlideShowPrintApertureFocalEntry;
    QString                    configSlideShowPrintCommentEntry;
    QString                    configSlideShowPrintDateEntry;
    QString                    configSlideShowPrintExpoSensitivityEntry;
    QString                    configSlideShowPrintMakeModelEntry;
    QString                    configSlideShowPrintNameEntry;
    QString                    configSlideShowPrintRatingEntry;
    QString                    configSlideShowStartCurrentEntry;
    QString                    configSplitterStateEntry;
    QString                    configTiffCompressionEntry;
    QString                    configUnclipColorsEntry;
    QString                    configUnderExposureColorEntry;
    QString                    configUnderExposureIndicatorEntry;
    QString                    configUseRawImportToolEntry;
    QString                    configUseThemeBackgroundColorEntry;
    QString                    configVerticalSplitterSizesEntry;
    QString                    configVerticalSplitterStateEntry;
    QString                    configWhiteBalanceEntry;


    bool                       removeFullScreenButton;
    bool                       fullScreenHideToolBar;

    QToolButton*               cmViewIndicator;
    QToolButton*               underExposureIndicator;
    QToolButton*               overExposureIndicator;

    KSqueezedTextLabel*        infoLabel;

    KActionCollection*         imagepluginsActionCollection;

    KAction*                   donateMoneyAction;
    KAction*                   contributeAction;
    KAction*                   filePrintAction;
    KAction*                   copyAction;
    KAction*                   cropAction;
    KAction*                   zoomPlusAction;
    KAction*                   zoomMinusAction;
    KAction*                   zoomTo100percents;
    KAction*                   zoomFitToSelectAction;
    KAction*                   rotateLeftAction;
    KAction*                   rotateRightAction;
    KAction*                   flipHorizAction;
    KAction*                   flipVertAction;
    KAction*                   slideShowAction;
    KAction*                   selectAllAction;
    KAction*                   selectNoneAction;
    KAction*                   rawCameraListAction;
    KAction*                   libsInfoAction;
    KAction*                   softProofOptionsAction;

    QSignalMapper*             undoSignalMapper;
    QSignalMapper*             redoSignalMapper;

    QEventLoop*                waitingLoop;
    QDialog*                   currentWindowModalDialog;

    KToggleAction*             zoomFitToWindowAction;
    KToggleAction*             viewCMViewAction;
    KToggleAction*             viewSoftProofAction;
    KToggleAction*             viewUnderExpoAction;
    KToggleAction*             viewOverExpoAction;
    KToggleAction*             showMenuBarAction;

    QList<int>                 fullscreenSizeBackup;

    ICCSettingsContainer*      ICCSettings;

    DZoomBar*                  zoomBar;
    PreviewToolBar*            previewToolBar;

    ExposureSettingsContainer* exposureSettings;

    EditorToolIface*           toolIface;
};

}  // namespace Digikam

#endif /* EDITORWINDOWPRIVATE_H */
