/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-01-16
 * Description : Qt item view for images - common delegate code
 *
 * Copyright (C) 2002-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C)      2009 by Andi Clemens <andi dot clemens at gmail dot com>
 * Copyright (C) 2002-2014 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include <QApplication>
#include <QCache>
#include <QPainter>

// KDE includes

#include <kglobal.h>
#include <kio/global.h>
#include <klocale.h>
#include <kdebug.h>

// Local includes

#include "dcategorizedview.h"
#include "thememanager.h"
#include "thumbbardock.h"

namespace Digikam
{

class DItemDelegate::Private
{
public:

    Private()
    {
    }

    QCache<QString, QPixmap> thumbnailBorderCache;
    QCache<QString, QString> squeezedTextCache;
};

DItemDelegate::DItemDelegate(QObject* const parent)
    : QAbstractItemDelegate(parent), d(new Private)
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
    const QColor borderColor = QColor(0, 0, 0, 128);
    QString cacheKey         = QString::number(pixSize.width()) + '-' + QString::number(pixSize.height());
    QPixmap* const cachePix  = d->thumbnailBorderCache.object(cacheKey);

    if (!cachePix)
    {
        const int radius = 3;
        QPixmap pix      = ThumbBarDock::generateFuzzyRect(QSize(pixSize.width()  + 2*radius,
                           pixSize.height() + 2*radius),
                           borderColor, radius);
        const_cast<DItemDelegate*>(this)->d->thumbnailBorderCache.insert(cacheKey, new QPixmap(pix));
        return pix;
    }

    return *cachePix;
}

QPixmap DItemDelegate::makeDragPixmap(const QStyleOptionViewItem& option,
                                      const QList<QModelIndex>& indexes,
                                      const QPixmap& suggestedPixmap)
{
    QPixmap icon = suggestedPixmap;

    if (icon.isNull())
    {
        icon = QPixmap(DesktopIcon("image-jp2", KIconLoader::SizeMedium));
    }

    if (qMax(icon.width(), icon.height()) > KIconLoader::SizeHuge)
    {
        icon = icon.scaled(KIconLoader::SizeHuge, KIconLoader::SizeHuge,
                           Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    int w                 = icon.width();
    int h                 = icon.height();
    const int borderWidth = 6;

    QRect   rect(0, 0, w + borderWidth*2, h + borderWidth*2);
    QRect   pixmapRect(borderWidth, borderWidth, w, h);

    QPixmap pix(rect.size());
    QPainter p(&pix);

/* 
    // border
    p.fillRect(0, 0, pix.width()-1, pix.height()-1, QColor(Qt::white));
    p.setPen(QPen(Qt::black, 1));
    p.drawRect(0, 0, pix.width()-1, pix.height()-1);
*/

    QStyleOption opt(option);
    opt.rect = rect;
    qApp->style()->drawPrimitive(QStyle::PE_PanelTipLabel, &opt, &p);

    p.drawPixmap(pixmapRect.topLeft(), icon);

    QFont f(option.font);
    f.setBold(true);
    p.setFont(f);

    if (indexes.size() > 1)
    {
        QRect   textRect;
        QString text;

        QString text2(i18np("1 Image", "%1 Images", indexes.count()));
        QString text1 = QString::number(indexes.count());
        QRect r1      = p.boundingRect(pixmapRect, Qt::AlignLeft|Qt::AlignTop, text1).adjusted(0,0,1,1);
        QRect r2      = p.boundingRect(pixmapRect, Qt::AlignLeft|Qt::AlignTop, text2).adjusted(0,0,1,1);

        if (r2.width() > pixmapRect.width() || r2.height() > pixmapRect.height())
        {
//            textRect     = r1;
            text         = text1;
            int rectSize = qMax(r1.width(), r1.height());
            textRect     = QRect(0, 0, rectSize, rectSize);
        }
        else
        {
            textRect = QRect(0, 0, r2.width(), r2.height());
            text     = text2;
        }

        textRect.moveLeft((pixmapRect.width() - textRect.width()) / 2 + pixmapRect.x());
        textRect.moveTop((pixmapRect.height() - textRect.height()) * 4 / 5);
        p.fillRect(textRect, QColor(0, 0, 0, 128));
        p.setPen(Qt::white);
        p.drawText(textRect, Qt::AlignCenter, text);
    }

    return pix;
}

QString DItemDelegate::dateToString(const QDateTime& datetime)
{
    return KGlobal::locale()->formatDateTime(datetime, KLocale::ShortDate, false);
}

QString DItemDelegate::squeezedTextCached(QPainter* const p, int width, const QString& text) const
{
    QCache<QString, QString>* const cache = &const_cast<DItemDelegate*>(this)->d->squeezedTextCache;
    // We do not need to include the font into cache key, the cache is cleared on font change
    QString cacheKey                      = QString::number(width) + QString::number(qHash(text));
    QString* cachedString                 = cache->object(cacheKey);

    if (cachedString)
    {
        return *cachedString;
    }

    QString result = squeezedText(p->fontMetrics(), width, text);

    cache->insert(cacheKey, new QString(result));
    return result;
}

QString DItemDelegate::squeezedText(const QFontMetrics& fm, int width, const QString& text)
{
    QString fullText(text);
    fullText.replace('\n',' ');
    return fm.elidedText(text, Qt::ElideRight, width);

/*
    // Home-brewn implementation

    int textWidth  = fm.width(fullText);
    QString result = fullText;

    if (textWidth > width)
    {
        // start with the dots only
        QString squeezedText = "...";
        int squeezedWidth    = fm.width(squeezedText);

        // estimate how many letters we can add to the dots on both sides
        int letters          = fullText.length() * (width - squeezedWidth) / textWidth;

        if (width < squeezedWidth)
        {
            letters=1;
        }

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
*/
}

} // namespace Digikam
