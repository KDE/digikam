/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2003-02-10
 * Description : 
 * 
 * Copyright 2003 by Renchi Raju

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

class QComboBox;
class QListView;
class QListViewItem;
class QRadioButton;
class QVButtonGroup;
class QLabel;
class QLineEdit;

namespace Digikam
{

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
    
private:
    
    QListView* listView_;
    QLineEdit* titleEdit_;
    QVButtonGroup* portButtonGroup_;
    QRadioButton* usbButton_;
    QRadioButton* serialButton_;
    QLabel* portPathLabel_;
    QComboBox* portPathComboBox_;
    QComboBox* umsMountComboBox_;

    QString UMSCameraNameActual_;
    QString UMSCameraNameShown_;
    QStringList serialPortList_;

private slots:

    void slotSelectionChanged(QListViewItem *item);
    void slotPortChanged();

    void slotOkClicked();
    
signals:

    void signalOkClicked(const QString& title,
                         const QString& model,
                         const QString& port,
                         const QString& path);
        
};

}  // namespace Digikam

#endif // CAMERASELECTION_H
