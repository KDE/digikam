/* ============================================================
 * File  : cameraui.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2003-01-21
 * Description :
 *
 * Copyright 2003 by Renchi Raju
 *
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published bythe Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include <klocale.h>
#include <kaction.h>
#include <kstdaccel.h>
#include <kstatusbar.h>
#include <kprogress.h>
#include <kapp.h>
#include <kconfig.h>
#include <kkeydialog.h>
#include <kedittoolbar.h>

#include <qlabel.h>


#include "camerauiview.h"
#include "camerasettings.h"
#include "cameraui.h"

CameraUI* CameraUI::instance_ = 0;

CameraUI* CameraUI::getInstance()
{
    return instance_;
}

CameraUI::CameraUI(const QString& libraryPath, const QString& downloadAlbum,
                   const CameraType& ctype)
    : KMainWindow( 0, ctype.title() )
{
    fullScreen = false;
    instance_ = this;

    setCaption( ctype.title() );

    KConfig* config=kapp->config();

    cameraSettings_ = new CameraSettings();
    cameraSettings_->readSettings();

    view_       = 0;
    cameraType_ = ctype;

    setupView();
    setupActions();
    setupConnections();

    setAutoSaveSettings();
    applyMainWindowSettings (config);

    if (!cameraSettings_->getShowFolders())
        hideFoldersAction->activate();

    changeLibraryPath(libraryPath);
    changeDownloadAlbum(downloadAlbum);
}

CameraUI::~CameraUI()
{
    
    emit signalFinished();

    cameraSettings_->setShowFolders(!hideFoldersAction->isChecked());
    cameraSettings_->saveSettings();
    delete cameraSettings_;
    instance_ = 0;
}

const CameraType& CameraUI::cameraType()
{
    return cameraType_;
}

void CameraUI::setupView()
{
    statusLabel_ = new QLabel(statusBar());
    statusLabel_->setText(i18n("Ready"));
    statusBar()->addWidget( statusLabel_, 7, true);
    progressBar_ = new KProgress(statusBar());
    progressBar_->setTotalSteps(100);
    statusBar()->addWidget( progressBar_, 5, true);

    view_ = new CameraUIView(this, cameraType_);
    setCentralWidget(view_);

    view_->applySettings(cameraSettings_);
}

void CameraUI::setupActions()
{
    // -----------------------------------------------------------------
    camConnectAction = new KToggleAction(i18n("Connect"),
                                       "cameraconnect",
                                       CTRL+Key_D,
                                       view_,
                                       SLOT(slotCameraConnectToggle()),
                                       actionCollection(),
                                       "connect");

    // -----------------------------------------------------------------
    camCancelAction = new KAction(i18n("Stop"),
                                       "stop",
                                       KShortcut(Key_Escape),
                                       view_,
                                       SLOT(slotCameraCancel()),
                                       actionCollection(),
                                       "cancel");

    // -----------------------------------------------------------------
    camDownloadAction = new KActionMenu(i18n("Download"),
                                       "downloadcamera",
                                       actionCollection(),
                                       "download");

    camDownloadAction->setDelayed(false);

    KAction* downloadSelectedAction = new KAction(i18n("Selected"),
                                       0,
                                       view_,
                                       SLOT(slotCameraDownloadSelected()),
                                       actionCollection());

    KAction* downloadAllAction = new KAction(i18n("All"),
                                       0,
                                       view_,
                                       SLOT(slotCameraDownloadAll()),
                                       actionCollection());

    camDownloadAction->insert(downloadSelectedAction);
    camDownloadAction->insert(downloadAllAction);

    // -----------------------------------------------------------------
    camDeleteAction = new KActionMenu(i18n("Delete"),
                                       "deleteimage",
                                       actionCollection(),
                                       "delete");

    camDeleteAction->setDelayed(false);

    KAction* deleteSelectedAction = new KAction(i18n("Selected"),
                                       0,
                                       view_,
                                       SLOT(slotCameraDeleteSelected()),
                                       actionCollection());

    KAction *deleteAllAction = new KAction(i18n("All"),
                                       0,
                                       view_,
                                       SLOT(slotCameraDeleteAll()),
                                       actionCollection());

    camDeleteAction->insert(deleteSelectedAction);
    camDeleteAction->insert(deleteAllAction);

    // -----------------------------------------------------------------
    camUploadAction = new KAction(i18n("Upload"),
                                       "uploadcamera",
                                       CTRL+SHIFT+Key_U,
                                       view_, SLOT(slotCameraUpload()),
                                       actionCollection(),
                                       "upload");

    // -----------------------------------------------------------------
    camInfoAction = new KAction(i18n("Camera Information"),
                                       "camerainfo",
                                       CTRL+Key_F3,
                                       view_,
                                       SLOT(slotCameraInformation()),
                                       actionCollection(),
                                       "information");

    // -----------------------------------------------------------------
    hideFoldersAction = new KToggleAction(i18n("Hide Camera Folders"),
                                       "hidecamerafolders",
                                       0,
                                       view_,
                                       SLOT(slotShowFoldersToggle()),
                                       actionCollection(),
                                       "hideFolders");

    // -----------------------------------------------------------------
    hideStatusBarAction = new KToggleAction(i18n("Show/Hide StatusBar"),
                                       0,
                                       0,
                                       this,
                                       SLOT(slotShowStatusBarToggle()),
                                       actionCollection(),
                                       "hideStatusBar");

    // -----------------------------------------------------------------
    thumbSizePlusAction = new KAction(i18n("Increase Thumbnail Size"),
                                       "viewmag+",
                                       ALT+Key_Plus,
                                       view_,
                                       SLOT(slotThumbSizePlus()),
                                       actionCollection(),
                                       "thumbSizeIncrease");

    // -----------------------------------------------------------------
    thumbSizeMinusAction = new KAction(i18n("Decrease Thumbnail Size"),
                                       "viewmag-",
                                       ALT+Key_Minus,
                                       view_,
                                       SLOT(slotThumbSizeMinus()),
                                       actionCollection(),
                                       "thumbSizeDecrease");

    // -----------------------------------------------------------------
    fullScreenAction = new KAction(i18n("Toggle Full Screen"),
                                   "window_fullscreen",
                                   CTRL+SHIFT+Key_F,
                                   this,
                                   SLOT(slotToggleFullScreen()),
                                   actionCollection(),
                                   "full_screen");

    // -----------------------------------------------------------------
    selectAllAction = new KAction(i18n("Select All"),
                                       0,
                                       CTRL+Key_A,
                                       view_,
                                       SLOT(slotSelectAll()),
                                       actionCollection(),
                                       "selectAll");

    // -----------------------------------------------------------------
    selectNoneAction = new KAction(i18n("Select None"),
                                       0,
                                       CTRL+Key_U,
                                       view_,
                                       SLOT(slotSelectNone()),
                                       actionCollection(),
                                       "selectNone");

    // -----------------------------------------------------------------
    selectInvertAction = new KAction(i18n("Invert Selection"),
                                       0,
                                       CTRL+Key_Asterisk,
                                       view_,
                                       SLOT(slotSelectInvert()),
                                       actionCollection(),
                                       "selectInvert");

    // -----------------------------------------------------------------
    selectNewAction = new KAction(i18n("Select New Items"),
                                       0,
                                       CTRL+Key_Slash,
                                       view_,
                                       SLOT(slotSelectNew()),
                                       actionCollection(),
                                       "selectNew");

    // -----------------------------------------------------------------
    KStdAction::quit(this, SLOT(slotExit()), actionCollection(), "cameraui_exit");

    // -----------------------------------------------------------

    KStdAction::keyBindings(this, SLOT(slotEditKeys()), actionCollection());
    KStdAction::configureToolbars(this, SLOT(slotConfToolbars()), actionCollection());

    // -----------------------------------------------------------

    // Disable the help menu
    setHelpMenuEnabled(false);
    createGUI("digikamcameraui.rc");

    // -----------------------------------------------------------------

    setCameraConnected(false);
}

void CameraUI::setupConnections()
{
    connect(view_, SIGNAL(signalStatusMsg(const QString&)),
            this,  SLOT(slotSetStatusMsg(const QString&)));
    connect(view_, SIGNAL(signalProgressVal(int)),
            this,  SLOT(slotSetProgressVal(int)));
    connect(view_, SIGNAL(signalBusy(bool)),
             this, SLOT(slotBusy(bool)));
}

void CameraUI::enableThumbSizePlusAction(bool val)
{
    thumbSizePlusAction->setEnabled(val);
}

void CameraUI::enableThumbSizeMinusAction(bool val)
{
    thumbSizeMinusAction->setEnabled(val);
}

void CameraUI::setCameraConnected(bool val)
{
    camConnectAction->setChecked(val);
    camDownloadAction->setEnabled(val);
    camDeleteAction->setEnabled(val);
    camUploadAction->setEnabled(val);
    camInfoAction->setEnabled(val);
}


void CameraUI::slotExit()
{
    close();    
}

void CameraUI::slotSetStatusMsg(const QString& msg)
{
    statusLabel_->setText(msg);
}

void CameraUI::slotSetProgressVal(int val)
{
    if (val >= 0 && val <= 100) {
        progressBar_->setProgress(val);
    }
}

void CameraUI::slotResetStatusBar()
{
    if (camConnectAction->isChecked()) {
        statusLabel_->setText(i18n("Connected"));
    }
    else {
        statusLabel_->setText(i18n("Disconnected"));
    }
    progressBar_->setTotalSteps(100);
    progressBar_->setProgress(0);
}

void CameraUI::slotShowStatusBarToggle()
{
    if (statusBar()->isHidden())
        statusBar()->show();
    else
        statusBar()->hide();
}

void CameraUI::slotBusy(bool val)
{
    if (!val)
        slotResetStatusBar();
    camCancelAction->setEnabled(val);
}

void CameraUI::slotEditKeys()
 {
 KKeyDialog::configure( actionCollection()
 #if KDE_VERSION >= 306
     , true /*allow on-letter shortcuts*/
 #endif
         );
 }

void CameraUI::slotConfToolbars()
{
	saveMainWindowSettings(KGlobal::config());
	KEditToolbar *dlg = new KEditToolbar(actionCollection(), "digikamcameraui.rc");
	if (dlg->exec())
	{
		createGUI("digikamcameraui.rc");
		applyMainWindowSettings(KGlobal::config());
	}
	delete dlg;
}

void CameraUI::slotToggleFullScreen()
{
if (fullScreen)
    {
    showNormal();
    fullScreen = false;
    move(0, 0);
    }
else
    {
    showFullScreen();
    fullScreen = true;
    }
}

// public slots --------------------------------------------------

void CameraUI::changeLibraryPath(const QString& libraryPath)
{
    view_->setLibraryPath(libraryPath);
}

void CameraUI::changeDownloadAlbum(const QString& album)
{
    view_->setCurrentAlbum(album);
}

void CameraUI::downloadSelected()
{
    view_->slotCameraDownloadSelected();
}

void CameraUI::connectCamera()
{
    camConnectAction->activate();
}


#include "cameraui.moc"
