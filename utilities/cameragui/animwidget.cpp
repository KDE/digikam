/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-09-21
 * Description : an animated busy widget 
 * 
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2007 by Gilles Caulier <caulier dot gilles at gmail dot com> 
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

// Qt includes.

#include <QPainter>
#include <QPixmap>
#include <QPalette>
#include <QColor>
#include <QTimer>

// Local includes.

#include "animwidget.h"
#include "animwidget.moc"

namespace Digikam
{

class AnimWidgetPriv
{
public:

    AnimWidgetPriv()
    {
        timer = 0;
        pix   = 0;
        pos   = 0;
    }

    int      pos;
    
    QTimer  *timer;
    
    QPixmap *pix;    
};


AnimWidget::AnimWidget(QWidget* parent, int size)
          : QWidget(parent)
{
    d = new AnimWidgetPriv;
 
    setAttribute(Qt::WA_DeleteOnClose);
    setMinimumSize(size, size);

    d->pix   = new QPixmap();
    d->timer = new QTimer();
    
    connect(d->timer, SIGNAL(timeout()),
            this, SLOT(slotTimeout()));
}

AnimWidget::~AnimWidget()
{
    delete d;
}

void AnimWidget::start()
{
    d->pos = 0;
    d->timer->start(100);
}

void AnimWidget::stop()
{
    d->pos = 0;
    d->timer->stop();
    repaint();
}

void AnimWidget::paintEvent(QPaintEvent*)
{
    d->pix->fill(palette().background().color());
    QPainter p(d->pix);

    p.translate(d->pix->width()/2, d->pix->height()/2);

    if (d->timer->isActive())
    {
        p.setPen(QPen(palette().color(QPalette::Text)));
        p.rotate( d->pos );
    }
    else
    {
        p.setPen(QPen(palette().color(QPalette::Dark)));
    }
            
    for ( int i=0 ; i<12 ; i++ )
    {
        p.drawLine(d->pix->width()/2-4, 0, d->pix->height()/2-2, 0);
        p.rotate(30);
    }
    
    p.end();

    QPainter p2(this);
    p2.drawPixmap(0, 0, *d->pix);
    p2.end();
}

void AnimWidget::resizeEvent(QResizeEvent *e)
{
    delete d->pix;
    int size = qMax(e->size().width(), e->size().height());
    d->pix = new QPixmap(size, size);
}

void AnimWidget::slotTimeout()
{
    d->pos = (d->pos + 10) % 360;
    repaint();    
}

bool AnimWidget::running() const
{
    return d->timer->isActive();    
}

}  // namespace Digikam
