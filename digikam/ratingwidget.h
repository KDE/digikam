/* ============================================================
 * Authors: Owen Hirst <n8rider@sbcglobal.net>
 *          Caulier Gilles 
 * Date   : 2005-08-15
 * Description : a widget to draw stars rating
 * 
 * Copyright 2005 by Owen Hirst
 * Copyright 2006 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef RATINGWIDGET_H
#define RATINGWIDGET_H

// Qt includes.

#include <qwidget.h>

namespace Digikam
{

class RatingWidgetPriv;

class RatingWidget : public QWidget
{
    Q_OBJECT
    
public:

    RatingWidget(QWidget* parent);
    ~RatingWidget();

    void setRating(int val);
    int  rating() const;

signals:

    void signalRatingChanged(int);
    
protected:

    void mousePressEvent(QMouseEvent* e);
    void mouseMoveEvent(QMouseEvent* e);
    void paintEvent(QPaintEvent* e);

private slots:

    void slotThemeChanged();

private:

    RatingWidgetPriv* d;

};

}  // namespace Digikam

#endif // RATINGWIDGET_H
