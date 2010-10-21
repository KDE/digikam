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

// KDE includes

#include <kconfiggroup.h>
#include <kdebug.h>

// Local includes

#include "editorwindow.h"

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

class EditorWindow::EditorWindowPriv
{

public:

    EditorWindowPriv() :
        configAutoZoomEntry("AutoZoom"),
        configBackgroundColorEntry("BackgroundColor"),
        configFullScreenEntry("FullScreen"),
        configFullScreenHideThumbBarEntry("FullScreenHideThumbBar"),
        configFullScreenHideToolBarEntry("FullScreen Hide ToolBar"),
        configJpeg2000CompressionEntry("JPEG2000Compression"),
        configJpeg2000LossLessEntry("JPEG2000LossLess"),
        configJpegCompressionEntry("JPEGCompression"),
        configJpegSubSamplingEntry("JPEGSubSampling"),
        configPgfCompressionEntry("PGFCompression"),
        configPgfLossLessEntry("PGFLossLess"),
        configPngCompressionEntry("PNGCompression"),
        configSlideShowDelayEntry("SlideShowDelay"),
        configSlideShowLoopEntry("SlideShowLoop"),
        configSlideShowPrintApertureFocalEntry("SlideShowPrintApertureFocal"),
        configSlideShowPrintCommentEntry("SlideShowPrintComment"),
        configSlideShowPrintDateEntry("SlideShowPrintDate"),
        configSlideShowPrintExpoSensitivityEntry("SlideShowPrintExpoSensitivity"),
        configSlideShowPrintMakeModelEntry("SlideShowPrintMakeModel"),
        configSlideShowPrintNameEntry("SlideShowPrintName"),
        configSlideShowPrintRatingEntry("SlideShowPrintRating"),
        configSlideShowStartCurrentEntry("SlideShowStartCurrent"),
        configSplitterStateEntry("SplitterState"),
        configTiffCompressionEntry("TIFFCompression"),
        configUnderExposureColorEntry("UnderExposureColor"),
        configUnderExposureIndicatorEntry("UnderExposureIndicator"),
        configUnderExposurePercentsEntry("UnderExposurePercentsEntry"),
        configOverExposureColorEntry("OverExposureColor"),
        configOverExposureIndicatorEntry("OverExposureIndicator"),        configOverExposurePercentsEntry("OverExposurePercentsEntry"),
        configExpoIndicatorModeEntry("ExpoIndicatorMode"),
        configUseRawImportToolEntry("UseRawImportTool"),
        configUseThemeBackgroundColorEntry("UseThemeBackgroundColor"),
        configVerticalSplitterSizesEntry("Vertical Splitter Sizes"),
        configVerticalSplitterStateEntry("Vertical Splitter State"),

        removeFullScreenButton(false),
        fullScreenHideToolBar(false),

        cmViewIndicator(0),
        underExposureIndicator(0),
        overExposureIndicator(0),

        infoLabel(0),

        imagepluginsActionCollection(0),

        contributeAction(0),
        copyAction(0),
        cropAction(0),
        donateMoneyAction(0),
        filePrintAction(0),
        flipHorizAction(0),
        flipVertAction(0),
        libsInfoAction(0),
        rawCameraListAction(0),
        rotateLeftAction(0),
        rotateRightAction(0),
        selectAllAction(0),
        selectNoneAction(0),
        slideShowAction(0),
        softProofOptionsAction(0),
        zoomFitToSelectAction(0),
        zoomMinusAction(0),
        zoomPlusAction(0),
        zoomTo100percents(0),

        undoSignalMapper(0),
        redoSignalMapper(0),

        waitingLoop(0),
        currentWindowModalDialog(0),

        zoomFitToWindowAction(0),
        viewCMViewAction(0),
        viewSoftProofAction(0),
        viewUnderExpoAction(0),
        viewOverExpoAction(0),
        showMenuBarAction(0),

        ICCSettings(0),

        zoomBar(0),
        previewToolBar(0),

        exposureSettings(0),

        toolIface(0)
    {
    }

    ~EditorWindowPriv()
    {
    }

    const QString              configAutoZoomEntry;
    const QString              configBackgroundColorEntry;
    const QString              configFullScreenEntry;
    const QString              configFullScreenHideThumbBarEntry;
    const QString              configFullScreenHideToolBarEntry;
    const QString              configJpeg2000CompressionEntry;
    const QString              configJpeg2000LossLessEntry;
    const QString              configJpegCompressionEntry;
    const QString              configJpegSubSamplingEntry;
    const QString              configPgfCompressionEntry;
    const QString              configPgfLossLessEntry;
    const QString              configPngCompressionEntry;
    const QString              configSlideShowDelayEntry;
    const QString              configSlideShowLoopEntry;
    const QString              configSlideShowPrintApertureFocalEntry;
    const QString              configSlideShowPrintCommentEntry;
    const QString              configSlideShowPrintDateEntry;
    const QString              configSlideShowPrintExpoSensitivityEntry;
    const QString              configSlideShowPrintMakeModelEntry;
    const QString              configSlideShowPrintNameEntry;
    const QString              configSlideShowPrintRatingEntry;
    const QString              configSlideShowStartCurrentEntry;
    const QString              configSplitterStateEntry;
    const QString              configTiffCompressionEntry;
    const QString              configUnderExposureColorEntry;
    const QString              configUnderExposureIndicatorEntry;
    const QString              configUnderExposurePercentsEntry;
    const QString              configOverExposureColorEntry;
    const QString              configOverExposureIndicatorEntry;
    const QString              configOverExposurePercentsEntry;
    const QString              configExpoIndicatorModeEntry;
    const QString              configUseRawImportToolEntry;
    const QString              configUseThemeBackgroundColorEntry;
    const QString              configVerticalSplitterSizesEntry;
    const QString              configVerticalSplitterStateEntry;

    bool                       removeFullScreenButton;
    bool                       fullScreenHideToolBar;

    QToolButton*               cmViewIndicator;
    QToolButton*               underExposureIndicator;
    QToolButton*               overExposureIndicator;

    KSqueezedTextLabel*        infoLabel;

    KActionCollection*         imagepluginsActionCollection;

    KAction*                   contributeAction;
    KAction*                   copyAction;
    KAction*                   cropAction;
    KAction*                   donateMoneyAction;
    KAction*                   filePrintAction;
    KAction*                   flipHorizAction;
    KAction*                   flipVertAction;
    KAction*                   libsInfoAction;
    KAction*                   rawCameraListAction;
    KAction*                   rotateLeftAction;
    KAction*                   rotateRightAction;
    KAction*                   selectAllAction;
    KAction*                   selectNoneAction;
    KAction*                   slideShowAction;
    KAction*                   softProofOptionsAction;
    KAction*                   zoomFitToSelectAction;
    KAction*                   zoomMinusAction;
    KAction*                   zoomPlusAction;
    KAction*                   zoomTo100percents;

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


    void legacyUpdateSplitterState(KConfigGroup &group)
    {

        // Check if the thumbnail size in the config file is splitter based (the
        // old method), and convert to dock based if needed.
        if (group.hasKey(configSplitterStateEntry))
        {
            // Read splitter state from config file
            QByteArray state;
            state = QByteArray::fromBase64(group.readEntry(configSplitterStateEntry, state));

            // Do a cheap check: a splitter state with 3 windows is always 34 bytes.
            if (state.count() == 34)
            {
                // Read the state in streamwise fashion.
                QDataStream stream(state);

                // The first 8 bytes are resp. the magic number and the version
                // (which should be 0, otherwise it's not saved with an older
                // digiKam version). Then follows the list of window sizes.
                qint32 marker;
                qint32 version = -1;
                QList<int> sizesList;

                stream >> marker;
                stream >> version;
                if (version == 0)
                {
                    stream >> sizesList;
                    if (sizesList.count() == 3)
                    {
                        kDebug() << "Found splitter based config, converting to dockbar";
                        // Remove the first entry (the thumbbar) and write the rest
                        // back. Then it should be fine.
                        sizesList.removeFirst();
                        QByteArray newData;
                        QDataStream newStream(&newData, QIODevice::WriteOnly);
                        newStream << marker;
                        newStream << version;
                        newStream << sizesList;
                        char s[24];
                        int numBytes = stream.readRawData(s, 24);
                        newStream.writeRawData(s, numBytes);
                        group.writeEntry(configSplitterStateEntry, newData.toBase64());
                    }
                }
            }
        }

    }

};

}  // namespace Digikam

#endif /* EDITORWINDOWPRIVATE_H */
