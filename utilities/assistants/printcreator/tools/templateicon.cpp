/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-06-26
 * Description : a tool to print images
 *
 * Copyright (C) 2008      by Andreas Trink <atrink at nociaro dot org>
 * Copyright (C) 2006-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

    Private()
    {
        icon_margin  = 2;
        rotate       = false;
        scale_width  = 0.0;
        scale_height = 0.0;
        pixmap       = 0;
        painter      = 0;
        icon         = 0;
    }

    QSize     paper_size;
    QSize     icon_size;
    int       icon_margin;

    float     scale_width;
    float     scale_height;
    bool      rotate;

    QPixmap*  pixmap;
    QPainter* painter;
    QIcon*    icon;
};

TemplateIcon::TemplateIcon(int height, const QSize& template_size)
    : d(new Private)
{
    d->paper_size         = template_size;
    d->icon_size          = QSize(height-2*d->icon_margin, height-2*d->icon_margin);

    // remark: d->icon_size is the real size of the d->icon, in the combo-box there is no space
    // between the icons, therefore the variable d->icon_margin
    d->icon_size.rwidth() = (int)(float(d->icon_size.height()) *
                            float(d->paper_size.width()) / float(d->paper_size.height()));
    d->scale_width        = float(d->icon_size.width())  / float(d->paper_size.width());
    d->scale_height       = float(d->icon_size.height()) / float(d->paper_size.height());
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
    d->icon_size.rwidth() = (int)(float(d->icon_size.height()) *
                            float(d->paper_size.width()) / float(d->paper_size.height()));
    d->scale_width        = float(d->icon_size.width())  / float(d->paper_size.width());
    d->scale_height       = float(d->icon_size.height()) / float(d->paper_size.height());

#ifdef DEBUG_OUTPUT
    qCDebug(DIGIKAM_GENERAL_LOG) << "begin: d->paper_size.width =" <<  d->paper_size.width();
    qCDebug(DIGIKAM_GENERAL_LOG) << "begin: d->paper_size.height=" <<  d->paper_size.height();
    qCDebug(DIGIKAM_GENERAL_LOG) << "begin: d->icon_size.width  =" <<  d->icon_size.width();
    qCDebug(DIGIKAM_GENERAL_LOG) << "begin: d->icon_size.height =" <<  d->icon_size.height();
    qCDebug(DIGIKAM_GENERAL_LOG) << "begin: d->scale_width      =" <<  d->scale_width;
    qCDebug(DIGIKAM_GENERAL_LOG) << "begin: d->scale_height     =" <<  d->scale_height;
#endif

    // d->icon back ground
    d->pixmap  = new QPixmap(d->icon_size);
    d->pixmap->fill(Qt::color0);

    d->painter = new QPainter();
    d->painter->begin(d->pixmap);

    d->painter->setPen(Qt::color1);
    d->painter->drawRect(d->pixmap->rect());
}

void TemplateIcon::fillRect( int x, int y, int w, int h, const QColor& color )
{
#ifdef DEBUG_OUTPUT
    qCDebug(DIGIKAM_GENERAL_LOG) << "fillRect: x1=" << x << " => " << x       * d->scale_width;
    qCDebug(DIGIKAM_GENERAL_LOG) << "fillRect: y1=" << y << " => " << y       * d->scale_height;
    qCDebug(DIGIKAM_GENERAL_LOG) << "fillRect: x2=" << w << " => " << (x + w) * d->scale_width;
    qCDebug(DIGIKAM_GENERAL_LOG) << "fillRect: y2=" << h << " => " << (y + h) * d->scale_height;
#endif

    d->painter->fillRect((int)(d->icon_margin + x * d->scale_width),
                         (int)(d->icon_margin + y * d->scale_height),
                         (int)(w * d->scale_width),
                         (int)(h * d->scale_height),
                         color);
}

void TemplateIcon::end()
{
    // paint boundary of template
    d->painter->setPen(Qt::color1);

    d->painter->drawRect(d->icon_margin,
                         d->icon_margin,
                         (int)(d->paper_size.width()  * d->scale_width),
                         (int)(d->paper_size.height() * d->scale_height));

    d->painter->end();
    d->icon = new QIcon(*d->pixmap);
}

QIcon& TemplateIcon::getIcon() const
{
    return *d->icon;
}

QSize& TemplateIcon::getSize()
{
    return d->icon_size;
}

QPainter& TemplateIcon::getPainter() const
{
    return *d->painter;
}

} // namespace Digikam
