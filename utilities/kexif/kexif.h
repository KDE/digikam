/***************************************************************************
                          kexif.h  -  description
                             -------------------
    begin                : Tue Aug 27 18:41:58 CDT 2002
    copyright            : (C) 2002 by Renchi Raju
    email                : renchi@green.tam.uiuc.edu
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KEXIF_H
#define KEXIF_H

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

#endif
