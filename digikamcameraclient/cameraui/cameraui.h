/* ============================================================
 * File  : cameraui.h
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2003-01-21
 * Description : 
 * 
 * Copyright 2003 by Renchi Raju

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

#ifndef CAMERAUI_H
#define CAMERAUI_H

#include <kmainwindow.h>

#include "cameratype.h"

class QLabel;
class KAction;
class KActionMenu;
class KToggleAction;
class KProgress;
class KDirLister;

class CameraUIView;
class CameraSettings;

class CameraUI : public KMainWindow
{
    Q_OBJECT

public:

    bool fullScreen;

    CameraUI(const QString& libraryPath, const QString& downloadAlbum,
             const CameraType& ctype);
    ~CameraUI();

    void enableThumbSizePlusAction(bool val);
    void enableThumbSizeMinusAction(bool val);
    void setCameraConnected(bool val);

    void changeLibraryPath(const QString& libraryPath);
    void changeDownloadAlbum(const QString& downloadAlbum);
    void downloadSelected();
    void connectCamera();
    
    static CameraUI* getInstance();
    const CameraType& cameraType();

private:

    void setupView();
    void setupActions();
    void setupConnections();

private:

    static CameraUI *instance_;
    
    CameraUIView    *view_;
    QLabel          *statusLabel_;
    KProgress       *progressBar_;

    KToggleAction *camConnectAction;
    KAction       *camCancelAction;
    KActionMenu   *camDownloadAction;
    KActionMenu   *camDeleteAction;
    KAction       *camUploadAction;
    KAction       *camInfoAction;

    KAction       *selectAllAction;
    KAction       *selectNoneAction;
    KAction       *selectInvertAction;
    KAction       *selectNewAction;

    KToggleAction *hideFoldersAction;
    KToggleAction *hideStatusBarAction;
    KAction       *thumbSizePlusAction;
    KAction       *thumbSizeMinusAction;
    KAction       *fullScreenAction;

    CameraType      cameraType_;
    CameraSettings *cameraSettings_;

private slots:

    void slotExit();    
    void slotSetStatusMsg(const QString& msg);
    void slotSetProgressVal(int val);
    void slotResetStatusBar();
    void slotShowStatusBarToggle();
    void slotBusy(bool val);
    void slotEditKeys();
    void slotConfToolbars();
    void slotToggleFullScreen();

signals:

    void signalFinished();
};

#endif /* CAMERAUI_H */
