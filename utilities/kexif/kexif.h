//////////////////////////////////////////////////////////////////////////////
//
//    KEXIF.H
//
//    Copyright (C) 2002-2004 Renchi Raju <renchi at pooh.tam.uiuc.edu>
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

#ifndef KEXIF_H
#define KEXIF_H

// Qt includes.

#include <qwidget.h>

class QString;
class QTabWidget;
class QTextEdit;
class QGroupBox;

class KExifData;

class KExif : public QWidget
{
    Q_OBJECT

public:

    KExif(QWidget* parent=0, const char *name=0,
          WFlags fl = WDestructiveClose);
    ~KExif();

    int loadFile(const QString& filename);
    int loadData(const QString& filename, char *data, int size);

private:

    KExifData *mExifData;
    QTabWidget *mTabWidget;
    QTextEdit *mTextEdit;
    QGroupBox *mMainBox;

    QString mFileName;

private slots:

    void slot_tabPageChanged(QWidget*);
    void slot_showItemDescription(const QString& desc);
    void slot_close();
};

#endif  // KEXIF_H
