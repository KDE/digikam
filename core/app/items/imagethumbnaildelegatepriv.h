/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-04-19
 * Description : thumbnail bar for images - the delegate
 *
 * Copyright (C) 2006-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef IMAGETHUMBNAILDELEGATEPRIV_H
#define IMAGETHUMBNAILDELEGATEPRIV_H

// Qt includes

#include <QRect>
#include <QCache>

// Local includes

#include "imagedelegatepriv.h"

namespace Digikam
{

class ImageThumbnailDelegatePrivate : public ImageDelegate::ImageDelegatePrivate
{
public:

    ImageThumbnailDelegatePrivate()
    {
        flow                = QListView::LeftToRight;

        // switch off drawing of frames
        drawMouseOverFrame  = false;
        drawFocusFrame      = false;

        // switch off composing rating over background
        ratingOverThumbnail = true;
    }

    QListView::Flow flow;
    QRect           viewSize;

public:

    void init(ImageThumbnailDelegate* q);
};

} // namespace Digikam

#endif // IMAGETHUMBNAILDELEGATEPRIV_H
