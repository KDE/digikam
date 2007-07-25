/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-02-02
 * Description : a pop-up menu to show stars rating selection.
 * 
 * Copyright (C) 2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <QString>
#include <QPainter>
#include <QPixmap>
#include <QBitmap>
#include <QPolygon>

// KDE includes.

#include <klocale.h>
#include <kstandarddirs.h>

// Local includes.

#include "constants.h"
#include "themeengine.h"
#include "ratingpopupmenu.h"
#include "ratingpopupmenu.moc"

namespace Digikam
{

RatingPopupMenu::RatingPopupMenu(QWidget* parent)
               : Q3PopupMenu(parent)
{
    QPolygon starPolygon;

    // Pre-computed star polygon for a 15x15 pixmap.
    starPolygon << QPoint(0,  6);
    starPolygon << QPoint(5,  5);
    starPolygon << QPoint(7,  0);
    starPolygon << QPoint(9,  5);
    starPolygon << QPoint(14, 6);
    starPolygon << QPoint(10, 9);
    starPolygon << QPoint(11, 14);
    starPolygon << QPoint(7,  11);
    starPolygon << QPoint(3,  14);
    starPolygon << QPoint(4,  9);

    QAction *action = addAction(i18n("None"));
    action->setData(0);

    QPixmap starPix(15, 15);
    QPainter p1(&starPix);
    p1.setRenderHint(QPainter::Antialiasing, true);
    p1.setBrush(ThemeEngine::componentData()->textSpecialRegColor());
    p1.setPen(palette().color(QPalette::Active, QPalette::Foreground));
    p1.drawPolygon(starPolygon, Qt::WindingFill);
    p1.end();

    QBitmap clearPix(starPix.width(), starPix.height());
    clearPix.clear();

    for (int i = 1 ; i <= RatingMax ; i++)
    {
        QPixmap pix(starPix.width() * 5, starPix.height());
        pix.fill(palette().color(QPalette::Active, QPalette::Foreground));

        QPainter p2(&pix);
        p2.drawTiledPixmap(0, 0, i*starPix.width(), pix.height(), starPix);
        p2.drawTiledPixmap(i*starPix.width(), 0, 5*starPix.width()-i*starPix.width(), pix.height(), clearPix);
        p2.end();

        QAction *action = addAction(pix, QString(), this, SLOT(slotRatingTriggered()));
        action->setData(i);
    }
}

RatingPopupMenu::~RatingPopupMenu()
{
}

void RatingPopupMenu::slotRatingTriggered()
{
    int r = qobject_cast<QAction*>(sender())->data().toInt();
    emit rating(r);
}

}  // namespace Digikam
