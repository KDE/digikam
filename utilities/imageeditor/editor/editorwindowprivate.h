/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at gmail dot com>
 * Date   : 2006-01-20
 * Description : main image editor GUI implementation
 *               private data.
 *
 * Copyright 2006-2007 by Gilles Caulier
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

class KComboBox;
class KAction;
class KToggleAction;
class KWidgetAction;
class KSelectAction;
class KActionMenu;
class KAccel;

namespace Digikam
{

class ICCSettingsContainer;
class ExposureContainer;

class EditorWindowPriv
{

public:

    EditorWindowPriv()
    {
        removeFullScreenButton = false;
        fullScreenHideToolBar  = false;
        slideShowInFullScreen  = true;

        selectLabel            = 0;
        imagePluginsHelpAction = 0;
        donateMoneyAction      = 0;
        accelerators           = 0;
        viewCMViewAction       = 0;
        filePrintAction        = 0;
        copyAction             = 0;
        resizeAction           = 0;
        cropAction             = 0;
        rotateLeftAction       = 0;
        rotateRightAction      = 0;
        flipHorizAction        = 0;
        flipVertAction         = 0;
        ICCSettings            = 0;
        exposureSettings       = 0;
        underExposureIndicator = 0;
        overExposureIndicator  = 0;
        cmViewIndicator        = 0;
        viewUnderExpoAction    = 0;
        viewOverExpoAction     = 0;
        slideShowAction        = 0;
        zoomFitToWindowAction  = 0;
        zoomFitToSelectAction  = 0;
        zoomPlusAction         = 0;
        zoomMinusAction        = 0;
        zoomCombo              = 0;
        zoomComboAction        = 0;
    }

    ~EditorWindowPriv()
    {
    }

    bool                       removeFullScreenButton;
    bool                       fullScreenHideToolBar;
    bool                       slideShowInFullScreen;

    KComboBox                 *zoomCombo;

    QLabel                    *selectLabel;

    QToolButton               *cmViewIndicator;
    QToolButton               *underExposureIndicator; 
    QToolButton               *overExposureIndicator; 

    KAction                   *imagePluginsHelpAction;
    KAction                   *donateMoneyAction;
    KAction                   *filePrintAction;
    KAction                   *copyAction;
    KAction                   *resizeAction;
    KAction                   *cropAction;
    KAction                   *zoomPlusAction;
    KAction                   *zoomMinusAction;
    KAction                   *zoomFitToSelectAction;
    KAction                   *rotateLeftAction;
    KAction                   *rotateRightAction;
    KAction                   *flipHorizAction;
    KAction                   *flipVertAction;
    KAction                   *slideShowAction;

    KToggleAction             *zoomFitToWindowAction;
    KToggleAction             *viewCMViewAction;
    KToggleAction             *viewUnderExpoAction;
    KToggleAction             *viewOverExpoAction;

    KWidgetAction             *zoomComboAction;

    KAccel                    *accelerators;

    ICCSettingsContainer      *ICCSettings;

    ExposureSettingsContainer *exposureSettings;
};

}  // NameSpace Digikam

#endif /* EDITORWINDOWPRIVATE_H */
