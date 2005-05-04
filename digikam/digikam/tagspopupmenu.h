/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-09-07
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju

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

#include <qpopupmenu.h>
#include <qvaluelist.h>

class AlbumIconView;

class TagsPopupMenu : public QPopupMenu
{
    Q_OBJECT

public:

    TagsPopupMenu(AlbumIconView* view, int addToID,
                  bool onlyAssignedTags=false);
    ~TagsPopupMenu();

private:

    void        clearPopup();
    QPopupMenu* buildSubMenu(int tagid);
    bool        showThisTag(int tagid);

    AlbumIconView*   m_view;
    int              m_addToID;
    bool             m_onlyAssignedTags;
    QValueList<int>  m_assignedTags;
        
private slots:

    void slotAboutToShow();
    void slotActivated(int id);

signals:

    void signalTagActivated(int id);
};

#endif /* TAGSPOPUPMENU_H */
