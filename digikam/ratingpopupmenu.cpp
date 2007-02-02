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
    // -- Load rating Pixmap ------------------------------------------

    KGlobal::dirs()->addResourceType("digikam_rating",
                                     KGlobal::dirs()->kde_default("data")
                                     + "digikam/data");
    QString ratingPixPath = KGlobal::dirs()->findResourceDir("digikam_rating", "rating.png");
    ratingPixPath += "/rating.png";
    QPixmap ratingPixmap(ratingPixPath);

    QPainter painter(&ratingPixmap);
    painter.fillRect(0, 0, ratingPixmap.width(), ratingPixmap.height(),
                     ThemeEngine::instance()->textSpecialRegColor());
    painter.end();

    // ----------------------------------------------------------------

    insertItem(i18n("None"), 0);

    for (int i = 1 ; i <= 5 ; i++)
    {
        QPixmap pix(ratingPixmap.width() * 5, ratingPixmap.height());
        pix.fill(colorGroup().background());

        QPainter painter(&pix);
        painter.drawTiledPixmap(0, 0, i*ratingPixmap.width(), 
                                pix.height(), ratingPixmap);
        painter.end();
        insertItem(pix, i);
    }
}

RatingPopupMenu::~RatingPopupMenu()
{
}

}  // namespace Digikam

