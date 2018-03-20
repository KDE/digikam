/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-06-26
 * Description : a tool to print images
 *
 * Copyright (C) 2008      by Andreas Trink <atrink at nociaro dot org>
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "templateicon.h"

// Local includes

#include "digikam_debug.h"

//define next to get debug output
#undef DEBUG_OUTPUT

namespace Digikam
{

class TemplateIcon::Private
{
public:

    explicit Private()
    {
        iconMargin  = 2;
        rotate      = false;
        scaleWidth  = 0.0;
        scaleHeight = 0.0;
        pixmap      = 0;
        painter     = 0;
        icon        = 0;
    }

    QSize     paperSize;
    QSize     iconSize;
    int       iconMargin;

    float     scaleWidth;
    float     scaleHeight;
    bool      rotate;

    QPixmap*  pixmap;
    QPainter* painter;
    QIcon*    icon;
};

TemplateIcon::TemplateIcon(int height, const QSize& template_size)
    : d(new Private)
{
    d->paperSize         = template_size;
    d->iconSize          = QSize(height-2 * d->iconMargin,
                                 height-2 * d->iconMargin);

    // remark: d->iconSize is the real size of the d->icon, in the combo-box there is no space
    // between the icons, therefore the variable d->iconMargin
    d->iconSize.rwidth() = (int)(float(d->iconSize.height()) *
                            float(d->paperSize.width()) / float(d->paperSize.height()));
    d->scaleWidth        = float(d->iconSize.width())   / float(d->paperSize.width());
    d->scaleHeight       = float(d->iconSize.height())  / float(d->paperSize.height());
}

TemplateIcon::~TemplateIcon()
{
    delete d->pixmap;
    delete d->painter;
    delete d->icon;
    delete d;
}

void TemplateIcon::begin()
 {
    // compute scaling values
    d->iconSize.rwidth() = (int)(float(d->iconSize.height()) *
                            float(d->paperSize.width()) / float(d->paperSize.height()));
    d->scaleWidth        = float(d->iconSize.width())   / float(d->paperSize.width());
    d->scaleHeight       = float(d->iconSize.height())  / float(d->paperSize.height());

#ifdef DEBUG_OUTPUT
    qCDebug(DIGIKAM_GENERAL_LOG) << "begin: d->paperSize.width =" <<  d->paperSize.width();
    qCDebug(DIGIKAM_GENERAL_LOG) << "begin: d->paperSize.height=" <<  d->paperSize.height();
    qCDebug(DIGIKAM_GENERAL_LOG) << "begin: d->iconSize.width  =" <<  d->iconSize.width();
    qCDebug(DIGIKAM_GENERAL_LOG) << "begin: d->iconSize.height =" <<  d->iconSize.height();
    qCDebug(DIGIKAM_GENERAL_LOG) << "begin: d->scaleWidth      =" <<  d->scaleWidth;
    qCDebug(DIGIKAM_GENERAL_LOG) << "begin: d->scaleHeight     =" <<  d->scaleHeight;
#endif

    // d->icon back ground
    d->pixmap  = new QPixmap(d->iconSize);
    d->pixmap->fill(Qt::color0);

    d->painter = new QPainter();
    d->painter->begin(d->pixmap);

    d->painter->setPen(Qt::color1);
    d->painter->drawRect(d->pixmap->rect());
}

void TemplateIcon::fillRect( int x, int y, int w, int h, const QColor& color )
{
#ifdef DEBUG_OUTPUT
    qCDebug(DIGIKAM_GENERAL_LOG) << "fillRect: x1=" << x << " => " << x       * d->scaleWidth;
    qCDebug(DIGIKAM_GENERAL_LOG) << "fillRect: y1=" << y << " => " << y       * d->scaleHeight;
    qCDebug(DIGIKAM_GENERAL_LOG) << "fillRect: x2=" << w << " => " << (x + w) * d->scaleWidth;
    qCDebug(DIGIKAM_GENERAL_LOG) << "fillRect: y2=" << h << " => " << (y + h) * d->scaleHeight;
#endif

    d->painter->fillRect((int)(d->iconMargin + x * d->scaleWidth),
                         (int)(d->iconMargin + y * d->scaleHeight),
                         (int)(w * d->scaleWidth),
                         (int)(h * d->scaleHeight),
                         color);
}

void TemplateIcon::end()
{
    // paint boundary of template
    d->painter->setPen(Qt::color1);

    d->painter->drawRect(d->iconMargin,
                         d->iconMargin,
                         (int)(d->paperSize.width()  * d->scaleWidth),
                         (int)(d->paperSize.height() * d->scaleHeight));

    d->painter->end();
    d->icon = new QIcon(*d->pixmap);
}

QIcon& TemplateIcon::getIcon() const
{
    return *d->icon;
}

QSize& TemplateIcon::getSize()
{
    return d->iconSize;
}

QPainter& TemplateIcon::getPainter() const
{
    return *d->painter;
}

} // namespace Digikam
