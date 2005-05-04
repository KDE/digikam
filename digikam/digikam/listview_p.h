/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-02-21
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

#ifndef LISTVIEW_P_H
#define LISTVIEW_P_H

#include <qptrvector.h>

#include "listitem.h"

class QTimer;

class ListViewPriv
{
public:

    QPtrVector<ListItem> visibleItems;
    
    int                  itemMargin;
    int                  itemHeight;

    int                  controlSize;  // size of controls (arrows)
    int                  arrowBoxPos;  // x pos of arrows (has to offset
                                       // with item offset) 
    
    ListItem*            rootItem;
    ListItem*            selectedItem;

    QTimer*              timer;

    struct SortableItem {
        ListItem* item;
    };
    
};



#endif /* LISTVIEW_P_H */
