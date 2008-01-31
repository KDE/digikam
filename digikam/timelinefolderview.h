/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-01-14
 * Description : Searches dates folder view used by timeline
 *
 * Copyright (C) 2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef TIMELINEFOLDERVIEW_H
#define TIMELINEFOLDERVIEW_H

// Local includes.

#include "folderview.h"

namespace Digikam
{

class SAlbum;
class TimeLineFolderItem;

class TimeLineFolderView : public FolderView
{
    Q_OBJECT

public:

    TimeLineFolderView(QWidget* parent);
    ~TimeLineFolderView();

    void searchDelete(SAlbum* album);
    QString currentTimeLineSearchName() const;

signals:

    void signalTextSearchFilterMatch(bool);
    void signalAlbumSelected(SAlbum*);
    void signalRenameAlbum(SAlbum*);

public slots:

    void slotTextSearchFilterChanged(const QString&);

private slots:

    void slotAlbumAdded(Album* album);
    void slotAlbumDeleted(Album* album);
    void slotAlbumRenamed(Album* album);
    void slotSelectionChanged();
    void slotContextMenu(QListViewItem*, const QPoint&, int);

protected:

    void selectItem(int id);

private:

    // Used to store in database the name of search performed by 
    // current selection from timeline.
    QString m_currentTimeLineSearchName;
};

}  // namespace Digikam

#endif /* TIMELINEFOLDERVIEW_H */
