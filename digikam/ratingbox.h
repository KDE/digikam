/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-01-07
 * Description : a box to host rating widget in iocn item
 *
 * Copyright (C) 2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef RATINGBOX_H
#define RATINGBOX_H

// KDE includes.

#include <khbox.h>

namespace Digikam
{

class RatingBoxPriv;

class RatingBox : public KHBox
{
    Q_OBJECT

public:

    RatingBox(QWidget* parent);
    ~RatingBox();

    void setRating(int val);
    int  rating() const;

signals:

    void signalRatingChanged(int);

private:

    RatingBoxPriv* const d;
};

}  // namespace Digikam

#endif // RATINGBOX_H
