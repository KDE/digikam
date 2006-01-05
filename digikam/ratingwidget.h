/* ============================================================
 * Author: Owen Hirst <n8rider@sbcglobal.net>
 * Date  : 2005-08-15
 * Description :
 * 
 * Copyright 2005 by Owen Hirst
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
#include <qpixmap.h>

namespace Digikam
{

class RatingWidget : public QWidget
{
    Q_OBJECT
    
public:

    RatingWidget(QWidget* parent);
    ~RatingWidget();

    void setRating(int val);
    int  rating() const;
    
protected:

    void mousePressEvent(QMouseEvent* e);
    void mouseMoveEvent(QMouseEvent* e);
    void paintEvent(QPaintEvent* e);

private:

    int     m_rating;
    QPixmap m_selPixmap;
    QPixmap m_regPixmap;

signals:

    void signalRatingChanged(int);
};

}  // namespace Digikam

#endif // RATINGWIDGET_H
