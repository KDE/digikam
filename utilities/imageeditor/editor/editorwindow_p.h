/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-01-20
 * Description : main image editor GUI implementation
 *               private data.
 *
 * Copyright (C) 2006-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

class QToolButton;
class QLabel;
class QWidgetAction;
class QEventLoop;
class QSignalMapper;

class KAction;
class KActionCollection;
class KComboBox;
class KToggleAction;

namespace Digikam
{

class EditorToolIface;
class ExposureSettingsContainer;
class ICCSettingsContainer;

class EditorWindowPriv
{

public:

    EditorWindowPriv()
    {
        removeFullScreenButton       = false;
        fullScreenHideToolBar        = false;
        selectLabel                  = 0;
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
        zoomCombo                    = 0;
        zoomComboAction              = 0;
        selectAllAction              = 0;
        selectNoneAction             = 0;
        waitingLoop                  = 0;
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
    }

    ~EditorWindowPriv()
    {
    }

    bool                       removeFullScreenButton;
    bool                       fullScreenHideToolBar;

    QWidgetAction             *zoomComboAction;

    QLabel                    *selectLabel;

    QToolButton               *cmViewIndicator;
    QToolButton               *underExposureIndicator;
    QToolButton               *overExposureIndicator;

    KActionCollection         *imagepluginsActionCollection;

    KAction                   *donateMoneyAction;
    KAction                   *contributeAction;
    KAction                   *filePrintAction;
    KAction                   *copyAction;
    KAction                   *cropAction;
    KAction                   *zoomPlusAction;
    KAction                   *zoomMinusAction;
    KAction                   *zoomTo100percents;
    KAction                   *zoomFitToSelectAction;
    KAction                   *rotateLeftAction;
    KAction                   *rotateRightAction;
    KAction                   *flipHorizAction;
    KAction                   *flipVertAction;
    KAction                   *slideShowAction;
    KAction                   *selectAllAction;
    KAction                   *selectNoneAction;
    KAction                   *rawCameraListAction;
    KAction                   *libsInfoAction;
    KAction                   *softProofOptionsAction;

    QSignalMapper             *undoSignalMapper;
    QSignalMapper             *redoSignalMapper;

    QEventLoop                *waitingLoop;

    KToggleAction             *zoomFitToWindowAction;
    KToggleAction             *viewCMViewAction;
    KToggleAction             *viewSoftProofAction;
    KToggleAction             *viewUnderExpoAction;
    KToggleAction             *viewOverExpoAction;
    KToggleAction             *showMenuBarAction;

    KComboBox                 *zoomCombo;

    QList<int>                 fullscreenSizeBackup;

    ICCSettingsContainer      *ICCSettings;

    ExposureSettingsContainer *exposureSettings;

    EditorToolIface           *toolIface;
};

}  // namespace Digikam

#endif /* EDITORWINDOWPRIVATE_H */
