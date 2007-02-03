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

// Qt includes.

#include <qstring.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qbitmap.h>

// KDE includes.

#include <klocale.h>
#include <kglobal.h>
#include <kstandarddirs.h>

// Local includes.

#include "themeengine.h"
#include "ratingpopupmenu.h"
#include "ratingpopupmenu.moc"

namespace Digikam
{

RatingPopupMenu::RatingPopupMenu(QWidget* parent)
               : QPopupMenu(parent)
{
    KGlobal::dirs()->addResourceType("digikam_rating", KGlobal::dirs()->kde_default("data") + "digikam/data");
    QString ratingPixPath = KGlobal::dirs()->findResourceDir("digikam_rating", "rating.png");
    ratingPixPath += "/rating.png";

    insertItem(i18n("None"), 0);

    QBitmap starbm(ratingPixPath);
    QBitmap clearbm(starbm.width(), starbm.height(), true);    

    for (int i = 1 ; i <= 5 ; i++)
    {
        QPixmap pix(starbm.width() * 5, starbm.height());
        pix.fill(ThemeEngine::instance()->textSpecialRegColor());
        QBitmap mask(starbm.width() * 5, starbm.height());
        QPainter painter(&mask);
        painter.drawTiledPixmap(0, 0, 
                                i*starbm.width(), pix.height(), 
                                starbm);
        painter.drawTiledPixmap(i*starbm.width(), 0, 
                                5*starbm.width()-i*starbm.width(), pix.height(), 
                                clearbm);
        painter.end();
        pix.setMask(mask);
        insertItem(pix, i);
    }
}

RatingPopupMenu::~RatingPopupMenu()
{
}

}  // namespace Digikam

