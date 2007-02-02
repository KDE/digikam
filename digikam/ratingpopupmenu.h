/* ============================================================
 * Authors: Caulier Gilles <caulier dot gilles at kdemail dot net>
 * Date   : 2007-02-02
 * Description : a pop-up menu to show stars rating selection.
 * 
 * Copyright 2007 by Gilles Caulier
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

#ifndef RATING_POPUP_MENU_H
#define RATING_POPUP_MENU_H

// Qt includes.

#include <qpopupmenu.h>

namespace Digikam
{

class RatingPopupMenuPriv;

class RatingPopupMenu : public QPopupMenu
{
    Q_OBJECT
    
public:

    RatingPopupMenu(QWidget* parent=0);
    ~RatingPopupMenu();

};

}  // namespace Digikam

#endif // RATING_POPUP_MENU_H
