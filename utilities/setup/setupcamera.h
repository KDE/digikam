//////////////////////////////////////////////////////////////////////////////
//
//    SETUPGENERAL.H
//
//    Copyright (C) 2003-2004 Renchi Raju <renchi at pooh.tam.uiuc.edu>
//                            Gilles CAULIER <caulier dot gilles at free.fr>
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef SETUPCAMERA_H
#define SETUPCAMERA_H

// QT includes.

#include <qwidget.h>

class QListView;
class QListViewItem;
class QPushButton;

class SetupCamera : public QWidget
{
    Q_OBJECT

public:

    SetupCamera( QWidget* parent = 0 );
    ~SetupCamera();

    void applySettings();
    
private:
    
    QListView*   listView_;
    QPushButton* addButton_;
    QPushButton* removeButton_;
    QPushButton* editButton_;
    QPushButton* autoDetectButton_;

private slots:

    void slotSelectionChanged();

    void slotAddCamera();
    void slotRemoveCamera();
    void slotEditCamera();
    void slotAutoDetectCamera();

    void slotAddedCamera(const QString& title, const QString& model,
                         const QString& port,  const QString& path);
    void slotEditedCamera(const QString& title, const QString& model,
                          const QString& port,  const QString& path);
};

#endif // SETUPCAMERA_H 
