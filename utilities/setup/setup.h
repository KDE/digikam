//////////////////////////////////////////////////////////////////////////////
//
//    SETUP.H
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

#ifndef SETUP_H
#define SETUP_H

// KDE includes.

#include <kdialogbase.h>

class QFrame;
class SetupCamera;
class SetupGeneral;

class Setup : public KDialogBase {

    Q_OBJECT

public:

    Setup(QWidget* parent=0, const char* name=0);
    ~Setup();

private:

    QFrame * page_general;
    QFrame * page_camera;
    
    SetupCamera*  cameraPage_;
    SetupGeneral* generalPage_;

private slots:

    void slotOkClicked();
};

#endif  // SETUP_H
