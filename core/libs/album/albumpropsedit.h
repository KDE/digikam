/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-03-09
 * Description : Album properties dialog.
 *
 * Copyright (C) 2003-2004 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2005      by Tom Albers <tomalbers@kde.nl>
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes

#include <QString>
#include <QStringList>
#include <QDialog>

namespace Digikam
{

class PAlbum;

class AlbumPropsEdit : public QDialog
{
    Q_OBJECT

public:

    explicit AlbumPropsEdit(PAlbum* const album, bool create=false);
    ~AlbumPropsEdit();

    QString     title()           const;
    QString     comments()        const;
    QDate       date()            const;
    int         parent()          const;
    QString     category()        const;
    QStringList albumCategories() const;

    static bool editProps(PAlbum* const album,
                          QString&      title,
                          QString&      comments,
                          QDate&        date,
                          QString&      category,
                          QStringList&  albumCategories);

    static bool createNew(PAlbum* const parent,
                          QString&      title,
                          QString&      comments,
                          QDate&        date,
                          QString&      category,
                          QStringList&  albumCategories,
                          int&          parentSelector);

private Q_SLOTS:

    void slotTitleChanged(const QString& newtitle);
    void slotDateLowButtonClicked();
    void slotDateAverageButtonClicked();
    void slotDateHighButtonClicked();
    void slotHelp();

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* ALBUMPROPSEDIT_H */
