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

#ifndef SETUPGENERAL_H
#define SETUPGENERAL_H

// Qt includes.

#include <qwidget.h>

class QRadioButton;
class QCheckBox;
class QLineEdit;

class SetupGeneral : public QWidget
{
    Q_OBJECT
    
public:

    SetupGeneral(QWidget* parent = 0);
    ~SetupGeneral();

    void applySettings();

private:

    void readSettings();

    QLineEdit    *albumPathEdit;

    QRadioButton *smallIconButton_;
    QRadioButton *mediumIconButton_;
    QRadioButton *largeIconButton_;
    QRadioButton *hugeIconButton_;

    QCheckBox    *iconShowMimeBox_;
    QCheckBox    *iconShowSizeBox_;
    QCheckBox    *iconShowDateBox_;
    QCheckBox    *iconShowCommentsBox_;
    QCheckBox    *iconSaveExifBox_;
    
private slots:

    void slotChangeAlbumPath();
};

#endif // SETUPGENERAL_H 
