/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-10-09
 * Description : a widget to filter album contents by rating
 * 
 * Copyright (C) 2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2007 by Arnd Baecker <arnd dot baecker at web dot de>
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

#ifndef RATINGFILTER_H
#define RATINGFILTER_H

// Local includes.

#include "albumlister.h"
#include "ratingwidget.h"

namespace Digikam
{

class RatingFilterPriv;

class RatingFilter : public RatingWidget
{
    Q_OBJECT
    
public:

    RatingFilter(QWidget* parent);
    ~RatingFilter();

    void setRatingFilterCondition(AlbumLister::RatingCondition cond);
    AlbumLister::RatingCondition ratingFilterCondition();

signals:

    void signalRatingFilterChanged(int, AlbumLister::RatingCondition);

protected:

    void mousePressEvent(QMouseEvent*);
    void mouseMoveEvent(QMouseEvent*);
    void mouseReleaseEvent(QMouseEvent*);
       
private:

    void updateRatingTooltip();

private slots:

    void slotRatingChanged();

private:

    RatingFilterPriv* d;
};

}  // namespace Digikam

#endif // RATINGWIDGET_H
