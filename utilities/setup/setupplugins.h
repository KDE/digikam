//////////////////////////////////////////////////////////////////////////////
//
//    SETUPGENERAL.H
//
//    Copyright (C) 2004 Gilles CAULIER <caulier dot gilles at free.fr>
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

#ifndef SETUPMIME_H
#define SETUPMIME_H

// Qt includes.

#include <qwidget.h>

class QLineEdit;

class SetupMime : public QWidget
{
    Q_OBJECT
    
public:

    SetupMime(QWidget* parent = 0);
    ~SetupMime();

    void applySettings();

private:

    void readSettings();

    QLineEdit *m_imageFileFilterEdit;
    QLineEdit *m_movieFileFilterEdit;
    QLineEdit *m_audioFileFilterEdit;
    QLineEdit *m_rawFileFilterEdit;
};

#endif // SETUPMIME_H 
