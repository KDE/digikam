/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Gilles Caulier <caulier dot gilles at free.fr>
 * Date   : 2004-02-14
 * Description : simple widget to display an image 
 * 
 * Copyright 2004 by Renchi Raju
 * Copyright 2005-2006 by Gilles Caulier
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

#include <qregion.h>
#include <qpainter.h>

// Local includes.

#include "imageiface.h"
#include "imagewidget.h"

namespace Digikam
{

class ImageWidgetPriv
{
public:

    ImageWidgetPriv()
    {
        data  = 0;
        iface = 0;
    }

    uchar      *data;
    int         width;
    int         height;

    QRect       rect;

    ImageIface *iface;
};

ImageWidget::ImageWidget(int w, int h, QWidget *parent)
           : QWidget(parent, 0, Qt::WDestructiveClose)
{
    d = new ImageWidgetPriv;
    setBackgroundMode(Qt::NoBackground);
    setMinimumSize(w, h);

    d->iface  = new ImageIface(w, h);
    d->data   = d->iface->getPreviewImage();
    d->width  = d->iface->previewWidth();
    d->height = d->iface->previewHeight();
    d->rect   = QRect(w/2-d->width/2, h/2-d->height/2, d->width, d->height);
}

ImageWidget::~ImageWidget()
{
    delete [] d->data;
    delete d->iface;
    delete d;
}

ImageIface* ImageWidget::imageIface()
{
    return d->iface;
}

void ImageWidget::paintEvent(QPaintEvent *)
{
    d->iface->paint(this, d->rect.x(), d->rect.y(),
                   d->rect.width(), d->rect.height());

    QRect r(0, 0, width(), height());
    QRegion reg(r);
    reg -= d->rect;

    QPainter p(this);
    p.setClipRegion(reg);
    p.fillRect(r, colorGroup().background());
    p.end();
}

void ImageWidget::resizeEvent(QResizeEvent * e)
{
    int w     = e->size().width();
    int h     = e->size().height();
    d->data   = d->iface->setPreviewImageSize(w, h);
    d->width  = d->iface->previewWidth();
    d->height = d->iface->previewHeight();

    d->rect = QRect(w/2-d->width/2, h/2-d->height/2, d->width, d->height);
    emit signalResized();
}

} // namespace Digikam

#include "imagewidget.moc"
