/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-07-23
 * Description : QGraphicsRectItem wrapper for FacesEngine
 *
 * Copyright (C) 2009-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C)      2008 by Adrien Bustany <madcat at mymadcat dot com>
 * Copyright (C)      2010 by Aditya Bhatt <adityabhatt1991 at gmail dot com>
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

#ifndef DIGIKAM_DEMO_FANCY_RECT_H
#define DIGIKAM_DEMO_FANCY_RECT_H

// Qt includes

#include <QGraphicsItem>
#include <QGraphicsRectItem>
#include <QRectF>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QWidget>

namespace Digikam
{

class FancyRect : public QGraphicsRectItem
{

public:

    explicit FancyRect(QGraphicsItem* const parent = 0);
    explicit FancyRect(const QRectF& rect, QGraphicsItem* const parent = 0);

    FancyRect(qreal x, qreal y, qreal w, qreal h, QGraphicsItem* const parent = 0);
    FancyRect(QGraphicsRectItem* const other, QGraphicsItem* const parent);

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = 0);
};

} // namespace Digikam

#endif // DIGIKAM_DEMO_FANCY_RECT_H
