/* ============================================================
 * File  : setup.h
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2003-02-10
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

#ifndef SETUP_H
#define SETUP_H

#include <kdialogbase.h>

class SetupCamera;
class SetupGeneral;

class Setup : public KDialogBase {

    Q_OBJECT

public:

    Setup(QWidget* parent=0, const char* name=0);
    ~Setup();

private:

    SetupCamera*  cameraPage_;
    SetupGeneral* generalPage_;

private slots:

    void slotOkClicked();
};


#endif
