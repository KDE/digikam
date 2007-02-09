/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2006-01-20
 * Description : main image editor GUI implementation
 *               private data.
 *
 * Copyright 2006 by Gilles Caulier
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

class QLabel;

class KAction;
class KToggleAction;
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

        imagePluginsHelpAction = 0;
        donateMoneyAction      = 0;
        accelerators           = 0;
        viewCMViewAction       = 0;
        filePrintAction        = 0;
        copyAction             = 0;
        resizeAction           = 0;
        zoomFitAction          = 0;
        zoomPlusAction         = 0;
        zoomMinusAction        = 0;
        cropAction             = 0;
        rotate90Action         = 0;
        rotate180Action        = 0;
        rotate270Action        = 0;
        flipHorzAction         = 0;
        flipVertAction         = 0;
        flipAction             = 0;
        rotateAction           = 0;
        ICCSettings            = 0;
        exposureSettings       = 0;
        underExposureIndicator = 0;
        overExposureIndicator  = 0;
        cmViewIndicator        = 0;
        viewUnderExpoAction    = 0;
        viewOverExpoAction     = 0;
        slideShowAction        = 0;
    }

    ~EditorWindowPriv()
    {
    }

    bool                       removeFullScreenButton;
    bool                       fullScreenHideToolBar;
    bool                       slideShowInFullScreen;

    QLabel                    *cmViewIndicator;
 
    QLabel                    *underExposureIndicator; 
    QLabel                    *overExposureIndicator; 

    KAction                   *imagePluginsHelpAction;
    KAction                   *donateMoneyAction;
    KAction                   *filePrintAction;
    KAction                   *copyAction;
    KAction                   *resizeAction;
    KAction                   *cropAction;
    KAction                   *zoomPlusAction;
    KAction                   *zoomMinusAction;
    KAction                   *rotate90Action;
    KAction                   *rotate180Action;
    KAction                   *rotate270Action;
    KAction                   *flipHorzAction;
    KAction                   *flipVertAction;
    KAction                   *slideShowAction;

    KActionMenu               *flipAction;
    KActionMenu               *rotateAction;

    KToggleAction             *zoomFitAction;
    KToggleAction             *viewCMViewAction;
    KToggleAction             *viewUnderExpoAction;
    KToggleAction             *viewOverExpoAction;

    KAccel                    *accelerators;

    ICCSettingsContainer      *ICCSettings;

    ExposureSettingsContainer *exposureSettings;
};

}  // NameSpace Digikam

#endif /* EDITORWINDOWPRIVATE_H */
