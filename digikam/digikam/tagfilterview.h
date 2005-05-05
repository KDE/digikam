/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2005-05-05
 * Copyright 2005 by Renchi Raju
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
 * ============================================================ */

#ifndef TAGFILTERVIEW_H
#define TAGFILTERVIEW_H

#include <qlistview.h>

class Album;
class TagFilterViewPriv;

class TagFilterView : public QListView
{
    Q_OBJECT

public:

    TagFilterView(QWidget* parent);
    ~TagFilterView();

    void triggerChange();
    
private slots:

    void slotTagAdded(Album* album);
    void slotClear();
    void slotTimeOut();

private:

    TagFilterViewPriv *d;
};


#endif /* TAGFILTERVIEW_H */
