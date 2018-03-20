/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-03-27
 * Description : Threaded image filter to fix hot pixels
 *
 * Copyright (C) 2005-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2005-2006 by Unai Garro <ugarro at users dot sourceforge dot net>
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

#ifndef HOTPIXEL_H
#define HOTPIXEL_H

// Qt includes

#include <QRect>

namespace Digikam
{

class HotPixel
{

public:

    QRect rect;
    int   luminosity;

public:

    int y() const
    {
        return rect.y();
    };

    int x() const
    {
        return rect.x();
    };

    int width() const
    {
        return rect.width();
    };

    int height() const
    {
        return rect.height();
    };

    bool operator==(const HotPixel& p) const
    {
        //we can say they're same hotpixel spot if they
        //touch(next to) each other horizontally or vertically, not diagonal corners
        //return (rect.intersects(p.rect));
        return (rect != p.rect) && (x() + width()  >= p.x() && x() <= p.x() + p.width()   &&
                                    y() + height() >= p.y() && y() <= p.y() + p.height()) &&
                                   !diagonal(rect, p.rect);
    }

private:

    bool diagonal(QRect r1, QRect r2) const
    {
        //locate next-to positions

        bool top    = r1.y() + height() - 1 == r2.y() - 1; //r1 is on the top of r2
        bool left   = r1.x() + width()  - 1 == r2.x() - 1; //r1 is on the left of r2
        bool right  = r1.x() == r2.x() + r2.width();
        bool bottom = r1.y() == r2.y() + r2.height();

        return ((top && left) || (top && right) || (bottom && left) || (bottom && right));
    }
};

}  // namespace Digikam

#endif  // HOTPIXEL_H
