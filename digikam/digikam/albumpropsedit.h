/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2003-03-09
 * Description :
 *
 * Copyright 2003 by Renchi Raju
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

#ifndef ALBUMPROPSEDIT_H
#define ALBUMPROPSEDIT_H

// Qt includes.

#include <qdatetime.h>
#include <qstring.h>
#include <qstringlist.h>

// KDE includes.

#include <kdialogbase.h>

class KDatePicker;
class QComboBox;
class QLineEdit;
class QCheckBox;
class QCheckListItem;
class KTextEdit;

class PAlbum;

class AlbumPropsEdit : public KDialogBase
{
    Q_OBJECT
    
public:

    AlbumPropsEdit(PAlbum* album, bool create=false);
    ~AlbumPropsEdit();

    QString     title() const;
    QString     comments() const;
    QDate       date() const;
    QString     collection() const;
    QStringList albumCollections() const;

    static bool editProps(PAlbum *album,
                          QString& title,
                          QString& comments,
                          QDate& date,
                          QString& collection,
                          QStringList& albumCollections);

    static bool createNew(PAlbum *parent,
                          QString& title,
                          QString& comments,
                          QDate& date,
                          QString& collection,
                          QStringList& albumCollections);

    QDate averageDate( bool basedOnExif ) const;


private:

    PAlbum         *album_;
    QStringList     albumCollections_;

    QLineEdit      *titleEdit_;
    KTextEdit      *commentsEdit_;

    KDatePicker    *datePicker_;

    QComboBox      *collectionCombo_;
    QCheckListItem *rootCollectionItem_;

private slots:
   void slotTitleChanged(const QString& newtitle);
   void slotAverageButtonClicked();
};

#endif /* ALBUMPROPSEDIT_H */
