/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-09-07
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju
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

// Qt includes.

#include <qpopupmenu.h>
#include <qvaluelist.h>
#include <qpixmap.h>

namespace Digikam
{

class TAlbum;

class TagsPopupMenu : public QPopupMenu
{
    Q_OBJECT

public:

    enum Mode
    {
        ASSIGN = 0,
        REMOVE
    };
    
    TagsPopupMenu(const QValueList<Q_LLONG>& selectedImageIDs,
                  int addToID, Mode mode);
    ~TagsPopupMenu();

private:

    void        clearPopup();
    QPopupMenu* buildSubMenu(int tagid);
    void        iterateAndBuildMenu(QPopupMenu *menu, TAlbum *album);
    bool        showThisTag(int tagid);

    QValueList<Q_LLONG>  m_selectedImageIDs;
    int              m_addToID;
    Mode             m_mode;
    QValueList<int>  m_assignedTags;
    QPixmap          m_addTagPix;
        
private slots:

    void slotAboutToShow();
    void slotActivated(int id);

signals:

    void signalTagActivated(int id);
};

}  // namespace Digikam

#endif /* TAGSPOPUPMENU_H */
