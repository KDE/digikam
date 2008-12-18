/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-02-02
 * Description : a pop-up menu to show stars rating selection.
 *
 * Copyright (C) 2007-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// KDE includes.

#include <kmenu.h>

class QAction;

namespace Digikam
{

class RatingPopupMenu : public KMenu
{
    Q_OBJECT

public:

    RatingPopupMenu(QWidget* parent=0);
    ~RatingPopupMenu();

signals:

    void signalRatingChanged(int);

private slots:

    void slotRatingTriggered();
    void slotHovered(QAction *);
};

}  // namespace Digikam

#endif // RATING_POPUP_MENU_H
