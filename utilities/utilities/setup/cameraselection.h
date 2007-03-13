/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Gilles Caulier <caulier dot gilles at gmail dot com>
 * Date   : 2003-02-10
 * Description : Camera type selection dialog
 * 
 * Copyright 2003-2005 by Renchi Raju
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

#ifndef CAMERASELECTION_H
#define CAMERASELECTION_H

// Qt includes.

#include <qstring.h>
#include <qstringlist.h>

// KDE includes.

#include <kdialogbase.h>

namespace Digikam
{

class CameraSelectionPriv;

class CameraSelection : public KDialogBase
{
    Q_OBJECT

public:
    
    CameraSelection( QWidget* parent = 0 );
    ~CameraSelection();

    void setCamera(const QString& title, const QString& model,
                   const QString& port, const QString& path);
    
    QString currentTitle();
    QString currentModel();
    QString currentPortPath();
    QString currentCameraPath();

private:

    void getCameraList();
    void getSerialPortList();
    
private slots:

    void slotPTPCameraLinkUsed();
    void slotUMSCameraLinkUsed();
    void slotSelectionChanged(QListViewItem *item);
    void slotPortChanged();
    void slotOkClicked();
    
signals:

    void signalOkClicked(const QString& title, const QString& model,
                         const QString& port,  const QString& path);

private:

    CameraSelectionPriv* d;        
};

}  // namespace Digikam

#endif // CAMERASELECTION_H
