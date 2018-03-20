/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-01-19
 * Description : free space widget tool tip
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "freespacetooltip.h"

// Qt includes

#include <QWidget>
#include <QString>
#include <QRect>

namespace Digikam
{

class FreeSpaceToolTip::Private
{
public:

    Private() :
        parent(0)
    {
    }

    QString  tip;

    QWidget* parent;
};

FreeSpaceToolTip::FreeSpaceToolTip(QWidget* const parent)
    : DItemToolTip(), d(new Private)
{
    d->parent = parent;
}

FreeSpaceToolTip::~FreeSpaceToolTip()
{
    delete d;
}

void FreeSpaceToolTip::setToolTip(const QString& tip)
{
    d->tip = tip;
}

void FreeSpaceToolTip::show()
{
    updateToolTip();
    reposition();

    if (isHidden() && !toolTipIsEmpty())
    {
        DItemToolTip::show();
    }
}

QRect FreeSpaceToolTip::repositionRect()
{
    if (!d->parent)
    {
        return QRect();
    }

    QRect rect = d->parent->rect();
    rect.moveTopLeft(d->parent->mapToGlobal(rect.topLeft()));
    return rect;
}

QString FreeSpaceToolTip::tipContents()
{
    return d->tip;
}

}  // namespace Digikam
