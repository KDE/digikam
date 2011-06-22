/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-05-19
 * Description : Find Duplicates View.
 *
 * Copyright (C) 2008-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2008-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2009      by Andi Clemens <andi dot clemens at gmx dot net>
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

// Qt includes

#include <QWidget>
#include <QPixmap>
#include <QTreeWidget>

// KDE includes

#include <kjob.h>

// Local includes

#include "thumbnailloadthread.h"

class QTreeWidgetItem;

namespace Digikam
{
class Album;
class SAlbum;

class FindDuplicatesView : public QWidget
{
    Q_OBJECT

public:

    FindDuplicatesView(QWidget* parent=0);
    ~FindDuplicatesView();

    SAlbum* currentFindDuplicatesAlbum() const;

Q_SIGNALS:

    void signalUpdateFingerPrints();

public Q_SLOTS:

    void slotSetSelectedAlbum(Album*);
    void slotSetSelectedTag(Album*);

private Q_SLOTS:

    void populateTreeView();
    void slotAlbumAdded(Album* a);
    void slotAlbumDeleted(Album* a);
    void slotSearchUpdated(SAlbum* a);
    void slotClear();
    void slotFindDuplicates();
    void slotDuplicatesAlbumActived(QTreeWidgetItem*, int);

    void slotDuplicatesSearchTotalAmount(KJob*, KJob::Unit, qulonglong);
    void slotDuplicatesSearchProcessedAmount(KJob*, KJob::Unit, qulonglong);
    void slotDuplicatesSearchResult(KJob*);

    void slotCancelButtonPressed();

    void slotAlbumSelectionChanged(Album*, Qt::CheckState);
    void slotTagSelectionChanged(Album*, Qt::CheckState);
    void slotUpdateAlbumsAndTags();

private:

    void enableControlWidgets(bool);
    bool checkForValidSettings();

    void updateAlbumsBox();
    void updateTagsBox();

    bool validAlbumSettings();
    bool validTagSettings();

    void resetAlbumsAndTags();

private:

    class FindDuplicatesViewPriv;
    FindDuplicatesViewPriv* const d;
};

// --------------------------------------------------------------------------------------------------------

class FindDuplicatesAlbum : public QTreeWidget
{
    Q_OBJECT

public:

    FindDuplicatesAlbum(QWidget* parent=0);
    virtual ~FindDuplicatesAlbum();

private :

    void drawRow(QPainter* p, const QStyleOptionViewItem& opt, const QModelIndex& index) const;

private Q_SLOTS:

    void slotThumbnailLoaded(const LoadingDescription&, const QPixmap&);

private:

    class FindDuplicatesAlbumPriv;
    FindDuplicatesAlbumPriv* const d;
};

}  // namespace Digikam

#endif /* FINDDUPLICATESVIEW_H */
