/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2005-04-27
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

#ifndef DATEFOLDERVIEW_H
#define DATEFOLDERVIEW_H

#include <qvbox.h>


class DateFolderViewPriv;
class DAlbum;

class DateFolderView : public QVBox
{
    Q_OBJECT
    
public:

    DateFolderView(QWidget* parent);
    ~DateFolderView();

    void setActive(bool val);
    
private slots:

    void slotDAlbumAdded(DAlbum* album);
    void slotSelectionChanged();
    
private:

    DateFolderViewPriv* d;
};

#endif /* DATEFOLDERVIEW_H */
