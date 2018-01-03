/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-09-07
 * Description : a pop-up menu implementation to display a
 *               hierarchical view of digiKam tags.
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef TAGSPOPUPMENU_H
#define TAGSPOPUPMENU_H

// Qt includes

#include <QList>
#include <QMenu>

namespace Digikam
{

class Album;
class TAlbum;

class TagsPopupMenu : public QMenu
{
    Q_OBJECT

public:

    enum Mode
    {
        ASSIGN = 0,
        REMOVE,
        DISPLAY,           // Used by "GoTo Tag" feature
        RECENTLYASSIGNED
    };

public:

    TagsPopupMenu(qlonglong selectedImageId, Mode mode, QWidget* const parent = 0);
    TagsPopupMenu(const QList<qlonglong>& selectedImageIDs, Mode mode, QWidget* const parent = 0);
    ~TagsPopupMenu();

Q_SIGNALS:

    void signalTagActivated(int id);
    void signalPopupTagsView();

private Q_SLOTS:

    void slotAboutToShow();
    void slotToggleTag(QAction*);
    void slotAddTag(QAction*);
    void slotTagThumbnail(Album*, const QPixmap&);

private:

    void     setup(Mode mode);
    void     clearPopup();
    QMenu*   buildSubMenu(int tagid);
    void     iterateAndBuildMenu(QMenu* menu, TAlbum* album);
    void     buildFlatMenu(QMenu* menu);
    void     setAlbumIcon(QAction* action, TAlbum* album);

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* TAGSPOPUPMENU_H */
