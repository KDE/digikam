/* ============================================================
 * File  : albumpropsedit.h
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2003-03-09
 * Description : 
 * 
 * Copyright 2003 by Renchi Raju
 *
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

#ifndef ALBUMPROPSEDIT_H
#define ALBUMPROPSEDIT_H

// Qt includes.

#include <qdatetime.h>
#include <qstring.h>
#include <qstringlist.h>

// KDE includes.

#include <kdialogbase.h>

class QDateEdit;
class QListView;
class QLineEdit;
class QCheckBox;
class QCheckListItem;

class PAlbum;

class AlbumPropsEdit : public KDialogBase
{
    Q_OBJECT

public:

    AlbumPropsEdit(PAlbum* album);
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

private:

    void  populateCollections();

    PAlbum         *album_;
    QStringList     albumCollections_;
    
    QLineEdit      *titleEdit_;
    QLineEdit      *commentsEdit_;
    
    QDateEdit      *dateEdit_;

    QListView      *collectionEdit_;
    QCheckListItem *rootCollectionItem_;

private slots:

    void slotAddCollection();
    void slotDelCollection();
};

#endif /* ALBUMPROPSEDIT_H */
