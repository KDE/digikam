/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-01-16
 * Description : Qt item view for images - common delegate code
 *
 * Copyright (C) 2002-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2002-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009 by Andi Clemens <andi dot clemens at gmx dot net>
 * Copyright (C) 2006-2010 by Marcel Wiesweg <marcel.wiesweg@gmx.de>
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

#include "ditemdelegate.moc"

// C++ includes

#include <cmath>

// Qt includes

#include <QCache>
#include <QPainter>

// KDE includes

#include <kglobal.h>
#include <kio/global.h>
#include <klocale.h>
#include <kdebug.h>

// Local includes

#include "dcategorizedview.h"
#include "themeengine.h"
#include "thumbbar.h"

namespace Digikam
{

class DItemDelegatePriv
{
public:

    DItemDelegatePriv()
    {
    }

    QCache<QString, QPixmap>  thumbnailBorderCache;
    QCache<QString, QString>  squeezedTextCache;

};

DItemDelegate::DItemDelegate(DCategorizedView *parent)
             : QAbstractItemDelegate(parent), d(new DItemDelegatePriv)
{
}

DItemDelegate::~DItemDelegate()
{
    delete d;
}

void DItemDelegate::clearCaches()
{
    d->thumbnailBorderCache.clear();
    d->squeezedTextCache.clear();
}

QPixmap DItemDelegate::thumbnailBorderPixmap(const QSize& pixSize) const
{
    const int radius         = 3;
    const QColor borderColor = QColor(0, 0, 0, 128);

    QString cacheKey  = QString::number(pixSize.width()) + '-' + QString::number(pixSize.height());
    QPixmap *cachePix = d->thumbnailBorderCache.object(cacheKey);

    if (!cachePix)
    {
        QPixmap pix = ThumbBarView::generateFuzzyRect(QSize(pixSize.width()  + 2*radius,
                                                            pixSize.height() + 2*radius),
                                                      borderColor, radius);
        const_cast<DItemDelegate*>(this)->d->thumbnailBorderCache.insert(cacheKey, new QPixmap(pix));
        return pix;
    }

    return *cachePix;
}

QString DItemDelegate::dateToString(const QDateTime& datetime)
{
    return KGlobal::locale()->formatDateTime(datetime, KLocale::ShortDate, false);
}

QString DItemDelegate::squeezedTextCached(QPainter* p, int width, const QString& text) const
{
    QCache<QString, QString> *cache = &const_cast<DItemDelegate*>(this)->d->squeezedTextCache;
    // We do not need to include the font into cache key, the cache is cleared on font change
    QString cacheKey = QString::number(width) + QString::number(qHash(text));
    QString *cachedString = cache->object(cacheKey);
    if (cachedString)
        return *cachedString;

    QString result = squeezedText(p->fontMetrics(), width, text);

    cache->insert(cacheKey, new QString(result));
    return result;
}

QString DItemDelegate::squeezedText(const QFontMetrics &fm, int width, const QString& text)
{
    QString fullText(text);
    fullText.replace('\n',' ');
    int textWidth = fm.width(fullText);
    QString result = fullText;

    if (textWidth > width)
    {
        // start with the dots only
        QString squeezedText = "...";
        int squeezedWidth    = fm.width(squeezedText);

        // estimate how many letters we can add to the dots on both sides
        int letters = fullText.length() * (width - squeezedWidth) / textWidth;
        if (width < squeezedWidth) letters=1;
        squeezedText  = fullText.left(letters) + "...";
        squeezedWidth = fm.width(squeezedText);

        if (squeezedWidth < width)
        {
            // we estimated too short
            // add letters while text < label
            do
            {
                ++letters;
                squeezedText  = fullText.left(letters) + "...";
                squeezedWidth = fm.width(squeezedText);
            }
            while (squeezedWidth < width);

            --letters;
            squeezedText = fullText.left(letters) + "...";
        }
        else if (squeezedWidth > width)
        {
            // we estimated too long
            // remove letters while text > label
            do
            {
                --letters;
                squeezedText  = fullText.left(letters) + "...";
                squeezedWidth = fm.width(squeezedText);
            }
            while (letters && squeezedWidth > width);
        }

        if (letters >= 5)
        {

            result = squeezedText;
        }
    }
    return result;
}

} // namespace Digikam
