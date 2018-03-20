/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-02-13
 * Description : Layouting photos on a page
 *
 * Copyright (C) 2007-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "atkinspagelayout.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QList>

// Local includes

#include "atkinspagelayouttree.h"

namespace Digikam
{

class AtkinsPageLayout::Private
{
public:

    explicit Private()
    {
        tree = 0;
    }

    QMap<int, int>        indexMap;
    AtkinsPageLayoutTree* tree;
    QRectF                pageRect;
};

AtkinsPageLayout::AtkinsPageLayout(const QRectF& pageRect)
    : d(new Private)
{
    d->pageRect = pageRect;
    d->tree     = new AtkinsPageLayoutTree(aspectRatio(d->pageRect.size()),
                                         absoluteArea(d->pageRect.size()));
}

AtkinsPageLayout::~AtkinsPageLayout()
{
    delete d->tree;
    delete d;
}

double AtkinsPageLayout::aspectRatio(const QSizeF& size)
{
    return size.height() / size.width();
}

double AtkinsPageLayout::absoluteArea(const QSizeF &size)
{
    return size.height() * size.width();
}

void AtkinsPageLayout::addLayoutItem(int key, const QSizeF& size)
{
    double relativeArea = absoluteArea(size) / absoluteArea(d->pageRect.size());
    addLayoutItem(key, aspectRatio(size), relativeArea);
}

void AtkinsPageLayout::addLayoutItem(int key, double aspectRatio, double relativeArea)
{
    int index        = d->tree->addImage(aspectRatio, relativeArea);
    d->indexMap[key] = index;
}

QRectF AtkinsPageLayout::itemRect(int key)
{
    QMap<int,int>::iterator it = d->indexMap.find(key);

    if (it != d->indexMap.end())
    {
        // get rect relative to 0,0
        QRectF rect = d->tree->drawingArea(*it, d->pageRect);
        // translate to page rect origin
        rect.translate(d->pageRect.topLeft());
        return rect;
    }

    return QRectF();
}

} // Namespace Digikam
