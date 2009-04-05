/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-07-15
 * Description : themed icon item
 *
 * Copyright (C) 2005 by Renchi Raju <renchi at pooh.tam.uiuc.edu>
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

#include "themediconitem.h"

// Qt includes

#include <QPainter>
#include <QPixmap>
#include <QPalette>
#include <QPen>
#include <QFontMetrics>
#include <QFont>
#include <QDateTime>

// KDE includes

#include <kglobal.h>
#include <klocale.h>
#include <kapplication.h>
#include <kiconloader.h>

// Local includes

#include "albumiconitem.h"
#include "themeengine.h"
#include "themediconview.h"

namespace Digikam
{

ThemedIconItem::ThemedIconItem(IconGroupItem* parent)
              : IconItem(parent)
{
}

ThemedIconItem::~ThemedIconItem()
{
}

void ThemedIconItem::paintItem(QPainter* p2)
{
    ThemedIconView* view = qobject_cast<ThemedIconView*>(iconView());
    if (!view)
        return;

    QPixmap pix;
    QRect   r;

    if (isSelected())
        pix = *(view->itemBaseSelPixmap());
    else
        pix = *(view->itemBaseRegPixmap());

    ThemeEngine* te = ThemeEngine::instance();

    QPainter p(&pix);
    p.setPen(isSelected() ? te->textSelColor() : te->textRegColor());

    {
        r                       = view->itemPixmapRect();
        KIconLoader *iconLoader = KIconLoader::global();
        QPixmap thumbnail       = iconLoader->loadIcon("image-jpeg", KIconLoader::NoGroup, 128);

        p.drawPixmap(r.x() + (r.width()-thumbnail.width())/2,
                     r.y() + (r.height()-thumbnail.height())/2,
                     thumbnail);
    }

    r = view->itemNameRect();
    p.setFont(view->itemFontReg());
    p.drawText(r, Qt::AlignCenter, AlbumIconItem::squeezedText(&p, r.width(), "IMG_00.JPG"));

    p.setFont(view->itemFontCom());
    r = view->itemCommentsRect();
    p.drawText(r, Qt::AlignCenter, AlbumIconItem::squeezedText(&p, r.width(), i18n("Photo caption")));

    p.setFont(view->itemFontXtra());
    {
        QDateTime date = QDateTime::currentDateTime();
        r              = view->itemDateRect();
        p.setFont(view->itemFontXtra());
        QString str;
        AlbumIconItem::dateToString(date, str);
        p.drawText(r, Qt::AlignCenter, AlbumIconItem::squeezedText(&p, r.width(), str));
    }

    p.setFont(view->itemFontCom());
    p.setPen(isSelected() ? te->textSpecialSelColor() : te->textSpecialRegColor());

    {
        QString tags = i18n("Events, Places, Vacation");
        r            = view->itemTagRect();
        p.drawText(r, Qt::AlignCenter, AlbumIconItem::squeezedText(&p, r.width(), tags));
    }


    if (this == view->currentItem())
    {
        p.setPen(QPen(isSelected() ? te->textSelColor() : te->textRegColor(), 0, Qt::DotLine));
        p.drawRect(1, 1, pix.width()-2, pix.height()-2);
    }

    p.end();

    r = view->itemRect();
//    r = QRect(view->contentsToViewport(QPoint(r.x(), r.y())),
//              QSize(r.width(), r.height()));

    p2->drawPixmap(r.x(), r.y(), pix, 0, 0, r.width(), r.height());
}

}  // namespace Digikam
