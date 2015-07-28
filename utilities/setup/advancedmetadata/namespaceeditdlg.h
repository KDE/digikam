/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-07-03
 * Description : dialog to edit and create digiKam xmp namespaces
 *
 * Copyright (C) 2015 by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
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

#ifndef TAGEDITDLG_H
#define TAGEDITDLG_H

// Qt includes

#include <QMap>
#include <QString>
#include <QKeySequence>
#include <QDialog>

// Local includes

#include "dmetadatasettingscontainer.h"

namespace Digikam
{

class NamespaceEditDlg : public QDialog
{
    Q_OBJECT

public:

    NamespaceEditDlg( bool create, NamespaceEntry& entry, QWidget* parent = 0);
    ~NamespaceEditDlg();

//    QString      title()    const;


//    static bool tagEdit(QWidget* const parent, TAlbum* const album, QString& title, QString& icon, QKeySequence& ks);
//    static bool tagCreate(QWidget* const parent, TAlbum* const album, QString& title, QString& icon, QKeySequence& ks);

//    /** Create a list of new Tag album using a list of tags hierarchies separated by ",".
//        A hierarchy of tags is a string path of tags name separated by "/".
//        If a hierarchy start by "/" or if mainRootAlbum is null, it will be created from
//        root tag album, else it will be created from mainRootAlbum as parent album.
//        'errMap' is Map of TAlbum path and error message if tag creation failed.
//        Return the list of created Albums.
//    */
    static bool create(QWidget* parent, NamespaceEntry& entry);
    static bool edit(QWidget* parent, NamespaceEntry& entry);

    QString namespaceName() const;
    QString nameSpaceSeparator() const;
    bool isTagPath() const;
    QString extraXml() const;


//    static void showtagsListCreationError(QWidget* const parent, const QMap<QString, QString>& errMap);

//private Q_SLOTS:

//    void slotIconChanged();
//    void slotIconResetClicked();
//    void slotTitleChanged(const QString& newtitle);
//    void slotHelp();

private:

    void setupTagGui(NamespaceEntry& entry);
    void setupRatingGui(NamespaceEntry& entry);
    void setupCommentGui(NamespaceEntry& entry);
    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* TAGEDITDLG_H */
