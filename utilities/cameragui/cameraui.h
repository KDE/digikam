/* ============================================================
 * File  : cameraui.h
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-07-17
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju

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

#include <qmainwindow.h>

class CameraUIPriv;

class CameraUI : public QMainWindow
{
    Q_OBJECT
    
public:

    CameraUI(QWidget *parent, const QString& model,
             const QString& port, const QString& path);
    ~CameraUI();

private:

    CameraUIPriv *d;

    void loadInitialSize();
    void saveInitialSize();
    
private slots:

    void slotFatal(const QString& msg);
    void slotBusy(bool val);
    void slotSelectionChanged();
    void slotProgress(int val);
    void slotProgressHide();

    void slotDownloadSelected();
    void slotDownloadAll();
};

#endif /* CAMERAUI_H */
