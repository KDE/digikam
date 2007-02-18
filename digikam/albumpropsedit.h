/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Tom Albers <tomalbers@kde.nl>
 *          Gilles Caulier 
 * Date   : 2003-03-09
 * Description : Album properties dialog.
 *
 * Copyright 2003-2004 by Renchi Raju
 * Copyright 2005 by Tom Albers
 * Copyright 2006 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

namespace Digikam
{

class AlbumPropsEditPriv;

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

private slots:

    void slotTitleChanged(const QString& newtitle);
    void slotDateLowButtonClicked();
    void slotDateAverageButtonClicked();
    void slotDateHighButtonClicked();

private:

    AlbumPropsEditPriv* d;
    
};

}  // namespace Digikam

#endif /* ALBUMPROPSEDIT_H */
