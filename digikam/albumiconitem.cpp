/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2003-04-25
 * Description : implementation to render album icon item.
 * 
 * Copyright 2003-2005 by Renchi Raju and Gilles Caulier
 * Copyright 2006 by Gilles Caulier
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
#include <qstring.h>
#include <qpen.h>
#include <qfontmetrics.h>
#include <qfont.h>
#include <qdatetime.h>
#include <qstringlist.h>

// KDE includes.

#include <kurl.h>
#include <kglobal.h>
#include <klocale.h>
#include <kio/global.h>

// Local includes.

#include "themeengine.h"
#include "thumbnailsize.h"
#include "imageinfo.h"
#include "albumsettings.h"
#include "icongroupitem.h"
#include "pixmapmanager.h"
#include "albumiconview.h"
#include "albumiconitem.h"

namespace Digikam
{

class AlbumIconItemPriv
{
public:

    AlbumIconItemPriv()
    {
        dirty = true;
        info  = 0;
        view  = 0;
    }

    bool           dirty;

    QRect          tightPixmapRect;

    ImageInfo     *info;

    AlbumIconView *view;
};

static void dateToString(const QDateTime& datetime, QString& str)
{
    str = KGlobal::locale()->formatDateTime(datetime, true, false);
}

AlbumIconItem::AlbumIconItem(IconGroupItem* parent, ImageInfo* info)
             : IconItem(parent)
{
    d = new AlbumIconItemPriv;
    d->view = (AlbumIconView*) parent->iconView();
    d->info = info;
}

AlbumIconItem::~AlbumIconItem()
{
    delete d;
}

QString AlbumIconItem::squeezedText(QPainter* p, int width, const QString& text)
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

bool AlbumIconItem::isDirty()
{
    return d->dirty;
}

ImageInfo* AlbumIconItem::imageInfo() const
{
    return d->info;
}

int AlbumIconItem::compare(IconItem *item)
{
    const AlbumSettings *settings = d->view->settings();
    AlbumIconItem *iconItem = static_cast<AlbumIconItem*>(item);
    
    switch (settings->getImageSortOrder())
    {
        case(AlbumSettings::ByIName):
        {
            return d->info->name().localeAwareCompare(iconItem->d->info->name());
        }
        case(AlbumSettings::ByIPath):
        {
            return d->info->kurl().path().compare(iconItem->d->info->kurl().path());
        }
        case(AlbumSettings::ByIDate):
        {
            if (d->info->dateTime() < iconItem->d->info->dateTime())
                return -1;
            else if (d->info->dateTime() > iconItem->d->info->dateTime())
                return 1;
            else
                return 0;
        }
        case(AlbumSettings::ByISize):
        {
            int mysize(d->info->fileSize());
            int hissize(iconItem->d->info->fileSize());
            if (mysize < hissize)
                return -1;
            else if (mysize > hissize)
                return 1;
            else
                return 0;
        }
        case(AlbumSettings::ByIRating):
        {
            int myrating(d->info->rating());
            int hisrating(iconItem->d->info->rating());
            if (myrating < hisrating)
                return 1;
            else if (myrating > hisrating)
                return -1;
            else
                return 0;
        }
    }

    return 0;
}

QRect AlbumIconItem::clickToOpenRect()
{
    if (d->tightPixmapRect.isNull())
        return rect();
    
    QRect pixmapRect = d->tightPixmapRect;
    QRect r          = rect();

    pixmapRect.moveBy(r.x(), r.y());
    return pixmapRect;
}

void AlbumIconItem::paintItem()
{
    QPixmap pix;
    QRect   r;
    const AlbumSettings *settings = d->view->settings();
    
    if (isSelected())
        pix = *(d->view->itemBaseSelPixmap());
    else
        pix = *(d->view->itemBaseRegPixmap());

    ThemeEngine* te = ThemeEngine::instance();
    
    QPainter p(&pix);
    p.setPen(isSelected() ? te->textSelColor() : te->textRegColor());


    d->dirty = true;
    
    QPixmap *thumbnail = d->view->pixmapManager()->find(d->info->kurl());
    if (thumbnail)
    {
        r = d->view->itemPixmapRect();
        p.drawPixmap(r.x() + (r.width()-thumbnail->width())/2,
                     r.y() + (r.height()-thumbnail->height())/2,
                     *thumbnail);
        d->tightPixmapRect.setRect(r.x() + (r.width()-thumbnail->width())/2,
                                 r.y() + (r.height()-thumbnail->height())/2,
                                 thumbnail->width(), thumbnail->height());
        d->dirty = false;
    }

    if (settings->getIconShowRating())
    {
        r = d->view->itemRatingRect();
        QPixmap ratingPixmap = d->view->ratingPixmap();

        int rating = d->info->rating();
        
        int x, w;
        x = r.x() + (r.width() - rating * ratingPixmap.width())/2;
        w = rating * ratingPixmap.width();
        
        p.drawTiledPixmap(x, r.y(), w, r.height(), ratingPixmap);
    }    
    
    if (settings->getIconShowName())
    {
        r = d->view->itemNameRect();
        p.setFont(d->view->itemFontReg());
        p.drawText(r, Qt::AlignCenter, squeezedText(&p, r.width(),
                                                    d->info->name()));
    }

    p.setFont(d->view->itemFontCom());
    
    if (settings->getIconShowComments())
    {
        QString comments = d->info->caption();
        
        r = d->view->itemCommentsRect();
        p.drawText(r, Qt::AlignCenter, squeezedText(&p, r.width(), comments));
    }

    p.setFont(d->view->itemFontXtra());

    if (settings->getIconShowDate())
    {
        QDateTime date(d->info->dateTime());

        r = d->view->itemDateRect();    
        p.setFont(d->view->itemFontXtra());
        QString str;
        dateToString(date, str);
        str = i18n("created : %1").arg(str);
        p.drawText(r, Qt::AlignCenter, squeezedText(&p, r.width(), str));
    }
    
    if (settings->getIconShowModDate())
    {
        QDateTime date(d->info->modDateTime());

        r = d->view->itemModDateRect();    
        p.setFont(d->view->itemFontXtra());
        QString str;
        dateToString(date, str);
        str = i18n("modified : %1").arg(str);
        p.drawText(r, Qt::AlignCenter, squeezedText(&p, r.width(), str));
    }
    
    if (settings->getIconShowResolution())
    {
        QSize dims = d->info->dimensions();
        if (dims.isValid())
        {
            QString mpixels, resolution;
            mpixels.setNum(dims.width()*dims.height()/1000000.0, 'f', 2);
            resolution = (!dims.isValid()) ? i18n("Unknown") : i18n("%1x%2 (%3Mpx)")
                         .arg(dims.width()).arg(dims.height()).arg(mpixels);
            r = d->view->itemResolutionRect();    
            p.drawText(r, Qt::AlignCenter, squeezedText(&p, r.width(), resolution));
        }
    }

    if (settings->getIconShowSize())
    {
        r = d->view->itemSizeRect();    
        p.drawText(r, Qt::AlignCenter,
                   squeezedText(&p, r.width(),
                                KIO::convertSize(d->info->fileSize())));
    }

    p.setFont(d->view->itemFontCom());
    p.setPen(isSelected() ? te->textSpecialSelColor() : te->textSpecialRegColor());

    if (settings->getIconShowTags())
    {
        QString tags = d->info->tagNames().join(", ");
        
        r = d->view->itemTagRect();    
        p.drawText(r, Qt::AlignCenter, 
                   squeezedText(&p, r.width(), tags));
    }

    if (this == d->view->currentItem())
    {
        p.setPen(QPen(isSelected() ? te->textSelColor() : te->textRegColor(),
                      0, Qt::DotLine));
        p.drawRect(1, 1, pix.width()-2, pix.height()-2);
    }
    
    p.end();
    
    r = rect();
    r = QRect(d->view->contentsToViewport(QPoint(r.x(), r.y())),
              QSize(r.width(), r.height()));

    bitBlt(d->view->viewport(), r.x(), r.y(), &pix,
           0, 0, r.width(), r.height());
}

QRect AlbumIconItem::thumbnailRect() const
{
    QRect pixmapRect = d->view->itemPixmapRect();
    QRect r          = rect();

    pixmapRect.moveBy(r.x(), r.y());
    return pixmapRect;
}

}  // namespace Digikam
