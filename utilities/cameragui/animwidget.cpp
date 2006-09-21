/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Gilles Caulier <caulier dot gilles at kdemail dot net> 
 * Date   : 2004-09-21
 * Description : an animated busy widget 
 * 
 * Copyright 2004-2005 by Renchi Raju
 * Copyright 2006 by Gilles Caulier
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

#include <qpainter.h>
#include <qpixmap.h>
#include <qpalette.h>
#include <qcolor.h>
#include <qtimer.h>

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
    int      size;
    
    QTimer  *timer;
    
    QPixmap *pix;    
};


AnimWidget::AnimWidget(QWidget* parent, int size)
          : QWidget(parent, 0, WResizeNoErase|WRepaintNoErase)
{
    d = new AnimWidgetPriv;
    setBackgroundMode(Qt::NoBackground);
    
    d->size = size;
    d->pix  = new QPixmap(d->size, d->size);
    setFixedSize(d->size, d->size);

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
    d->pix->fill(QColor(201, 208, 255));
    QPainter p(d->pix);

    p.translate(d->size/2, d->size/2);

    if (d->timer->isActive())
    {
        p.setPen(QPen(colorGroup().text()));
        p.rotate( d->pos );
    }
    else
    {
        p.setPen(QPen(colorGroup().dark()));
    }
            
    for ( int i=0 ; i<12 ; i++ )
    {
        p.drawLine(d->size/2-4, 0, d->size/2-2, 0);
        p.rotate(30);
    }
    
    p.end();
    bitBlt(this, 0, 0, d->pix);
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

