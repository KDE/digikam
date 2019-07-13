/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-01-20
 * Description : core image editor GUI implementation private data.
 *
 * Copyright (C) 2006-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_IMAGE_EDITOR_WINDOW_PRIVATE_H
#define DIGIKAM_IMAGE_EDITOR_WINDOW_PRIVATE_H

// Qt includes

#include <QList>
#include <QString>
#include <QMenu>

// KDE includes

#include <kconfiggroup.h>
#include <kservice.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_globals.h"
#include "editorwindow.h"
#include "versionmanager.h"
#include "dnotificationpopup.h"

class QAction;
class QDialog;
class QEventLoop;
class QLabel;
class QToolButton;
class QWidgetAction;

namespace Digikam
{

class ActionCategorizedView;
class DZoomBar;
class EditorToolIface;
class ExposureSettingsContainer;
class ICCSettingsContainer;
class PreviewToolBar;
class DAdjustableLabel;
class IccProfilesMenuAction;

class Q_DECL_HIDDEN EditorWindow::Private
{
public:

    Private()
      : cmViewIndicator(nullptr),
        underExposureIndicator(nullptr),
        overExposureIndicator(nullptr),
        infoLabel(nullptr),
        copyAction(nullptr),
        cropAction(nullptr),
        flipHorizAction(nullptr),
        flipVertAction(nullptr),
        rotateLeftAction(nullptr),
        rotateRightAction(nullptr),
        selectAllAction(nullptr),
        selectNoneAction(nullptr),
        slideShowAction(nullptr),
        softProofOptionsAction(nullptr),
        zoomFitToSelectAction(nullptr),
        zoomMinusAction(nullptr),
        zoomPlusAction(nullptr),
        zoomTo100percents(nullptr),
        openWithAction(nullptr),
        waitingLoop(nullptr),
        currentWindowModalDialog(nullptr),
        zoomFitToWindowAction(nullptr),
        viewCMViewAction(nullptr),
        viewSoftProofAction(nullptr),
        viewUnderExpoAction(nullptr),
        viewOverExpoAction(nullptr),
        selectToolsActionView(nullptr),
        ICCSettings(nullptr),
        zoomBar(nullptr),
        previewToolBar(nullptr),
        exposureSettings(nullptr),
        toolIface(nullptr)
    {
    }

    ~Private()
    {
    }

    void legacyUpdateSplitterState(KConfigGroup& group);
    void plugNewVersionInFormatAction(EditorWindow* const q, QMenu* const menuAction, const QString& text, const QString& format);

public:

    static const QString         configAutoZoomEntry;
    static const QString         configBackgroundColorEntry;
    static const QString         configJpeg2000CompressionEntry;
    static const QString         configJpeg2000LossLessEntry;
    static const QString         configJpegCompressionEntry;
    static const QString         configJpegSubSamplingEntry;
    static const QString         configPgfCompressionEntry;
    static const QString         configPgfLossLessEntry;
    static const QString         configPngCompressionEntry;
    static const QString         configSplitterStateEntry;
    static const QString         configTiffCompressionEntry;
    static const QString         configUnderExposureColorEntry;
    static const QString         configUnderExposureIndicatorEntry;
    static const QString         configUnderExposurePercentsEntry;
    static const QString         configOverExposureColorEntry;
    static const QString         configOverExposureIndicatorEntry;
    static const QString         configOverExposurePercentsEntry;
    static const QString         configExpoIndicatorModeEntry;
    static const QString         configUseRawImportToolEntry;
    static const QString         configUseThemeBackgroundColorEntry;
    static const QString         configVerticalSplitterSizesEntry;
    static const QString         configVerticalSplitterStateEntry;

    QToolButton*                 cmViewIndicator;
    QToolButton*                 underExposureIndicator;
    QToolButton*                 overExposureIndicator;

    DAdjustableLabel*            infoLabel;

    QAction*                     copyAction;
    QAction*                     cropAction;
    QAction*                     flipHorizAction;
    QAction*                     flipVertAction;
    QAction*                     rotateLeftAction;
    QAction*                     rotateRightAction;
    QAction*                     selectAllAction;
    QAction*                     selectNoneAction;
    QAction*                     slideShowAction;
    QAction*                     softProofOptionsAction;
    QAction*                     zoomFitToSelectAction;
    QAction*                     zoomMinusAction;
    QAction*                     zoomPlusAction;
    QAction*                     zoomTo100percents;
    QAction*                     openWithAction;

    QEventLoop*                  waitingLoop;
    QDialog*                     currentWindowModalDialog;

    QAction*                     zoomFitToWindowAction;
    QAction*                     viewCMViewAction;
    QAction*                     viewSoftProofAction;
    QAction*                     viewUnderExpoAction;
    QAction*                     viewOverExpoAction;

    ActionCategorizedView*       selectToolsActionView;

    ICCSettingsContainer*        ICCSettings;

    DZoomBar*                    zoomBar;
    PreviewToolBar*              previewToolBar;

    ExposureSettingsContainer*   exposureSettings;

    EditorToolIface*             toolIface;

    VersionManager               defaultVersionManager;

    QList<int>                   fullscreenSizeBackup;

    QMap<QString, KService::Ptr> servicesMap;
};

const QString EditorWindow::Private::configAutoZoomEntry(QLatin1String("AutoZoom"));
const QString EditorWindow::Private::configBackgroundColorEntry(QLatin1String("BackgroundColor"));
const QString EditorWindow::Private::configJpeg2000CompressionEntry(QLatin1String("JPEG2000Compression"));
const QString EditorWindow::Private::configJpeg2000LossLessEntry(QLatin1String("JPEG2000LossLess"));
const QString EditorWindow::Private::configJpegCompressionEntry(QLatin1String("JPEGCompression"));
const QString EditorWindow::Private::configJpegSubSamplingEntry(QLatin1String("JPEGSubSampling"));
const QString EditorWindow::Private::configPgfCompressionEntry(QLatin1String("PGFCompression"));
const QString EditorWindow::Private::configPgfLossLessEntry(QLatin1String("PGFLossLess"));
const QString EditorWindow::Private::configPngCompressionEntry(QLatin1String("PNGCompression"));
const QString EditorWindow::Private::configSplitterStateEntry(QLatin1String("SplitterState"));
const QString EditorWindow::Private::configTiffCompressionEntry(QLatin1String("TIFFCompression"));
const QString EditorWindow::Private::configUnderExposureColorEntry(QLatin1String("UnderExposureColor"));
const QString EditorWindow::Private::configUnderExposureIndicatorEntry(QLatin1String("UnderExposureIndicator"));
const QString EditorWindow::Private::configUnderExposurePercentsEntry(QLatin1String("UnderExposurePercentsEntry"));
const QString EditorWindow::Private::configOverExposureColorEntry(QLatin1String("OverExposureColor"));
const QString EditorWindow::Private::configOverExposureIndicatorEntry(QLatin1String("OverExposureIndicator"));
const QString EditorWindow::Private::configOverExposurePercentsEntry(QLatin1String("OverExposurePercentsEntry"));
const QString EditorWindow::Private::configExpoIndicatorModeEntry(QLatin1String("ExpoIndicatorMode"));
const QString EditorWindow::Private::configUseRawImportToolEntry(QLatin1String("UseRawImportTool"));
const QString EditorWindow::Private::configUseThemeBackgroundColorEntry(QLatin1String("UseThemeBackgroundColor"));
const QString EditorWindow::Private::configVerticalSplitterSizesEntry(QLatin1String("Vertical Splitter Sizes"));
const QString EditorWindow::Private::configVerticalSplitterStateEntry(QLatin1String("Vertical Splitter State"));

void EditorWindow::Private::legacyUpdateSplitterState(KConfigGroup& group)
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
            qint32     marker;
            qint32     version = -1;
            QList<int> sizesList;

            stream >> marker;
            stream >> version;

            if (version == 0)
            {
                stream >> sizesList;

                if (sizesList.count() == 3)
                {
                    qCDebug(DIGIKAM_GENERAL_LOG) << "Found splitter based config, converting to dockbar";
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

void EditorWindow::Private::plugNewVersionInFormatAction(EditorWindow* const q, QMenu* const menuAction,
                                                         const QString& text, const QString& format)
{
    QAction* const action = new QAction(text, q);

    connect(action, &QAction::triggered,
            q, [q, format]() { q->saveNewVersionInFormat(format); });

    menuAction->addAction(action);
}

} // namespace Digikam

#endif // DIGIKAM_IMAGE_EDITOR_WINDOW_PRIVATE_H
