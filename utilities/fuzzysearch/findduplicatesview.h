/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-05-19
 * Description : Find Duplicates View.
 *
 * Copyright (C) 2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef FINDDUPLICATESVIEW_H
#define FINDDUPLICATESVIEW_H

// Qt includes.

#include <QWidget>
#include <QPixmap>

// Local includes.

#include "treefolderitem.h"

class KJob;

namespace Digikam
{

class SAlbum;

class FindDuplicatesAlbumItem : public TreeFolderItem
{

public:

    FindDuplicatesAlbumItem(QTreeWidget* parent, SAlbum* album);
    virtual ~FindDuplicatesAlbumItem();

    SAlbum* album() const;
    int id() const;

private:

    void setThumb(const QPixmap& pix);

private:

    SAlbum *m_album;
};

// -------------------------------------------------------------------

class FindDuplicatesViewPriv;

class FindDuplicatesView : public QWidget
{
    Q_OBJECT

public:

    FindDuplicatesView(QWidget *parent=0);
    ~FindDuplicatesView();

signals:

    void signalUpdateFingerPrints();

private:

    void populateTreeView();

private slots:

    void slotFindDuplicates();

    void slotDuplicatesSearchTotalAmount(KJob*, KJob::Unit, qulonglong);
    void slotDuplicatesSearchProcessedAmount(KJob*, KJob::Unit, qulonglong);
    void slotDuplicatesSearchResult(KJob*);

private:

    FindDuplicatesViewPriv *d;
};

}  // NameSpace Digikam

#endif /* FINDDUPLICATESVIEW_H */
