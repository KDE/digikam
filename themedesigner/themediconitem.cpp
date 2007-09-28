/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-07-15
 * Description : themed icon item 
 * 
 * Copyright (C) 2005 by Renchi Raju <renchi at pooh.tam.uiuc.edu>
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

#include <qpainter.h>
#include <qpixmap.h>
#include <qpalette.h>
#include <qpen.h>
#include <qfontmetrics.h>
#include <qfont.h>
#include <qdatetime.h>

// KDE includes.

#include <kglobal.h>
#include <klocale.h>
#include <kapplication.h>
#include <kiconloader.h>

// Local includes.

#include "themeengine.h"
#include "themediconview.h"
#include "themediconitem.h"

namespace Digikam
{

static void dateToString(const QDateTime& datetime, QString& str)
{
    str = KGlobal::locale()->formatDateTime(datetime, true, false);
}

static QString squeezedText(QPainter* p, int width, const QString& text)
{
    QString fullText(text);
    fullText.replace("\n"," ");
    QFontMetrics fm(p->fontMetrics());
    int textWidth = fm.width(fullText);
    if (textWidth > width) 
    {
        // start with the dots only
        QString squeezedText = "...";
        int squeezedWidth = fm.width(squeezedText);

        // estimate how many letters we can add to the dots on both sides
        int letters = fullText.length() * (width - squeezedWidth) / textWidth;
        if (width < squeezedWidth) letters=1;
        squeezedText = fullText.left(letters) + "...";
        squeezedWidth = fm.width(squeezedText);

        if (squeezedWidth < width) 
        {
            // we estimated too short
            // add letters while text < label
            do 
            {
                letters++;
                squeezedText = fullText.left(letters) + "..."; 
                squeezedWidth = fm.width(squeezedText);
            }
            while (squeezedWidth < width);

            letters--;
            squeezedText = fullText.left(letters) + "..."; 
        } 
        else if (squeezedWidth > width) 
        {
            // we estimated too long
            // remove letters while text > label
            do 
            {
                letters--;
                squeezedText = fullText.left(letters) + "...";
                squeezedWidth = fm.width(squeezedText);
            } 
            while (letters && squeezedWidth > width);
        }

        if (letters >= 5) 
        {
            return squeezedText;
        }
    }
    
    return fullText;   
}

ThemedIconItem::ThemedIconItem(IconGroupItem* parent)
              : IconItem(parent)
{    
}

ThemedIconItem::~ThemedIconItem()
{    
}

void ThemedIconItem::paintItem()
{
    ThemedIconView* view = (ThemedIconView*)iconView();

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
        r = view->itemPixmapRect();
        KIconLoader *iconLoader = KApplication::kApplication()->iconLoader();    
        QPixmap thumbnail = iconLoader->loadIcon("colors", KIcon::NoGroup,
                                                 100, KIcon::DefaultState, 0, true);
        
        p.drawPixmap(r.x() + (r.width()-thumbnail.width())/2,
                     r.y() + (r.height()-thumbnail.height())/2,
                     thumbnail);
    }

    r = view->itemNameRect();
    p.setFont(view->itemFontReg());
    p.drawText(r, Qt::AlignCenter, squeezedText(&p, r.width(), "IMG_00.JPG"));

    p.setFont(view->itemFontCom());
    r = view->itemCommentsRect();
    p.drawText(r, Qt::AlignCenter, squeezedText(&p, r.width(), i18n("Photo caption")));

    p.setFont(view->itemFontXtra());
    {
        QDateTime date = QDateTime::currentDateTime();

        r = view->itemDateRect();    
        p.setFont(view->itemFontXtra());
        QString str;
        dateToString(date, str);
        p.drawText(r, Qt::AlignCenter, squeezedText(&p, r.width(), str));
    }

    p.setFont(view->itemFontCom());
    p.setPen(isSelected() ? te->textSpecialSelColor() : te->textSpecialRegColor());

    {
        QString tags = i18n("Events, Places, Vacation");
        
        r = view->itemTagRect();    
        p.drawText(r, Qt::AlignCenter, 
                   squeezedText(&p, r.width(), tags));
    }
    

    if (this == view->currentItem())
    {
        p.setPen(QPen(isSelected() ? te->textSelColor() : te->textRegColor(),
                      0, Qt::DotLine));
        p.drawRect(1, 1, pix.width()-2, pix.height()-2);
    }
    
    p.end();
    
    r = rect();
    r = QRect(view->contentsToViewport(QPoint(r.x(), r.y())),
              QSize(r.width(), r.height()));

    bitBlt(view->viewport(), r.x(), r.y(), &pix,
           0, 0, r.width(), r.height());
}

}  // NameSpace Digikam
