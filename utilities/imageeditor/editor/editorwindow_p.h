/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-01-20
 * Description : core image editor GUI implementation private data.
 *
 * Copyright (C) 2006-2014 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <QList>
#include <QString>
#include <QSignalMapper>

// KDE includes

#include <kconfiggroup.h>
#include <kactionmenu.h>
#include <kmenu.h>
#include <kdebug.h>
#include <kservice.h>

// Local includes

#include "globals.h"
#include "editorwindow.h"
#include "versionmanager.h"

class QDialog;
class QEventLoop;
class QLabel;
class QToolButton;
class QWidgetAction;

class KAction;
class KActionCollection;
class KSqueezedTextLabel;
class KToggleAction;

namespace Digikam
{

class ActionCategorizedView;
class DZoomBar;
class EditorToolIface;
class ExposureSettingsContainer;
class ICCSettingsContainer;
class PreviewToolBar;

class EditorWindow::Private
{

public:

    Private() :
        cmViewIndicator(0),
        underExposureIndicator(0),
        overExposureIndicator(0),
        infoLabel(0),
        imagepluginsActionCollection(0),
        copyAction(0),
        cropAction(0),
        autoCropAction(0),
        filePrintAction(0),
        flipHorizAction(0),
        flipVertAction(0),
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
        formatMenuActionMapper(0),
        waitingLoop(0),
        currentWindowModalDialog(0),
        zoomFitToWindowAction(0),
        viewCMViewAction(0),
        viewSoftProofAction(0),
        viewUnderExpoAction(0),
        viewOverExpoAction(0),
        showMenuBarAction(0),
        selectToolsActionView(0),
        ICCSettings(0),
        zoomBar(0),
        previewToolBar(0),
        exposureSettings(0),
        toolIface(0)
    {
    }

    ~Private()
    {
    }

    void legacyUpdateSplitterState(KConfigGroup& group);
    void plugNewVersionInFormatAction(EditorWindow* const q, KActionMenu* const menuAction, const QString& text, const QString& format);
    void addPageUpDownActions(EditorWindow* const q, QWidget* const w);

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

    KSqueezedTextLabel*          infoLabel;

    KActionCollection*           imagepluginsActionCollection;

    KAction*                     copyAction;
    KAction*                     cropAction;
    KAction*                     autoCropAction;
    KAction*                     filePrintAction;
    KAction*                     flipHorizAction;
    KAction*                     flipVertAction;
    KAction*                     rotateLeftAction;
    KAction*                     rotateRightAction;
    KAction*                     selectAllAction;
    KAction*                     selectNoneAction;
    KAction*                     slideShowAction;
    KAction*                     softProofOptionsAction;
    KAction*                     zoomFitToSelectAction;
    KAction*                     zoomMinusAction;
    KAction*                     zoomPlusAction;
    KAction*                     zoomTo100percents;

    QSignalMapper*               undoSignalMapper;
    QSignalMapper*               redoSignalMapper;
    QSignalMapper*               formatMenuActionMapper;

    QEventLoop*                  waitingLoop;
    QDialog*                     currentWindowModalDialog;

    KToggleAction*               zoomFitToWindowAction;
    KToggleAction*               viewCMViewAction;
    KToggleAction*               viewSoftProofAction;
    KToggleAction*               viewUnderExpoAction;
    KToggleAction*               viewOverExpoAction;
    KToggleAction*               showMenuBarAction;

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

const QString EditorWindow::Private::configAutoZoomEntry("AutoZoom");
const QString EditorWindow::Private::configBackgroundColorEntry("BackgroundColor");
const QString EditorWindow::Private::configJpeg2000CompressionEntry("JPEG2000Compression");
const QString EditorWindow::Private::configJpeg2000LossLessEntry("JPEG2000LossLess");
const QString EditorWindow::Private::configJpegCompressionEntry("JPEGCompression");
const QString EditorWindow::Private::configJpegSubSamplingEntry("JPEGSubSampling");
const QString EditorWindow::Private::configPgfCompressionEntry("PGFCompression");
const QString EditorWindow::Private::configPgfLossLessEntry("PGFLossLess");
const QString EditorWindow::Private::configPngCompressionEntry("PNGCompression");
const QString EditorWindow::Private::configSplitterStateEntry("SplitterState");
const QString EditorWindow::Private::configTiffCompressionEntry("TIFFCompression");
const QString EditorWindow::Private::configUnderExposureColorEntry("UnderExposureColor");
const QString EditorWindow::Private::configUnderExposureIndicatorEntry("UnderExposureIndicator");
const QString EditorWindow::Private::configUnderExposurePercentsEntry("UnderExposurePercentsEntry");
const QString EditorWindow::Private::configOverExposureColorEntry("OverExposureColor");
const QString EditorWindow::Private::configOverExposureIndicatorEntry("OverExposureIndicator");
const QString EditorWindow::Private::configOverExposurePercentsEntry("OverExposurePercentsEntry");
const QString EditorWindow::Private::configExpoIndicatorModeEntry("ExpoIndicatorMode");
const QString EditorWindow::Private::configUseRawImportToolEntry("UseRawImportTool");
const QString EditorWindow::Private::configUseThemeBackgroundColorEntry("UseThemeBackgroundColor");
const QString EditorWindow::Private::configVerticalSplitterSizesEntry("Vertical Splitter Sizes");
const QString EditorWindow::Private::configVerticalSplitterStateEntry("Vertical Splitter State");

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

void EditorWindow::Private::plugNewVersionInFormatAction(EditorWindow* const q, KActionMenu* const menuAction,
                                                         const QString& text, const QString& format)
{
    if (!formatMenuActionMapper)
    {
        formatMenuActionMapper = new QSignalMapper(q);

        connect(formatMenuActionMapper, SIGNAL(mapped(QString)),
                q, SLOT(saveNewVersionInFormat(QString)));
    }

    KAction* const action = new KAction(text, q);

    connect(action, SIGNAL(triggered()),
            formatMenuActionMapper, SLOT(map()));

    formatMenuActionMapper->setMapping(action, format);
    menuAction->menu()->addAction(action);
}

void EditorWindow::Private::addPageUpDownActions(EditorWindow* const q, QWidget* const w)
{
    defineShortcut(w, Qt::Key_Down,  q, SLOT(slotForward()));
    defineShortcut(w, Qt::Key_Right, q, SLOT(slotForward()));
    defineShortcut(w, Qt::Key_Up,    q, SLOT(slotBackward()));
    defineShortcut(w, Qt::Key_Left,  q, SLOT(slotBackward()));
}

}  // namespace Digikam

#endif /* EDITORWINDOWPRIVATE_H */
