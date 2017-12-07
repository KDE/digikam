/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-06-15
 * Description : a brush for use with tool to replace part of the image using another
 *
 * Copyright (C) 2004-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2017      by Shaza Ismail Kaoud <shaza dot ismail dot k at gmail dot com>
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

// Local includes

#include "imagebrushguidewidget.h"


namespace Digikam
{

void ImageBrushGuideWidget::mouseMoveEvent(QMouseEvent* e)
{
    if ((e->buttons() & Qt::LeftButton & srcSet) && (!released))
    {
        released = false;
        qDebug() << "MOOOOOVE The location is: " << e->x() << ", "<< e->y();
        QPoint currentDst = QPoint(e->x(),e->y());
        //updateSpotPosition(src.x() + currentDst.x() - dst.x(), src.y() + currentDst.y() - dst.y());
        currentDst = translateImagePosition(currentDst, false);
        QPoint currentSrc = translateImagePosition(src, true);
        QPoint orgDst = translateImagePosition(dst, false);
        currentSrc = QPoint(currentSrc.x() + currentDst.x() - orgDst.x(), currentSrc.y() + currentDst.y() - orgDst.y());
        //QPoint spotSrc = translateImagePosition(currentSrc, true);
        setSpotPosition(currentSrc);
        //updateSpotPosition(src.x() + currentDst.x() - orgDst.x(), src.y() + currentDst.y() - orgDst.y());
        emit signalClone(currentSrc, currentDst);
    }
    if(!srcSet)
    {
        ImageGuideWidget::mouseMoveEvent(e);
    }
}

void ImageBrushGuideWidget::mouseReleaseEvent(QMouseEvent* e)
{
    if(srcSet)
    {
        QPoint p = translatePointPosition(src);
        setSpotPosition(p);
    }else{
        ImageGuideWidget::mouseReleaseEvent(e);
    }
    released = true;
}

void ImageBrushGuideWidget::mousePressEvent(QMouseEvent* e)
{
    if(!srcSet)
    {
        ImageGuideWidget::mousePressEvent(e);
    }else{
        if (e->button() == Qt::LeftButton)
        {
            dst = QPoint(e->x(),e->y());
            //QPoint edit = translatePointPosition(dst);
        // signal the clone/heal
            QPoint currentSrc = translateImagePosition(src, true);
            QPoint currentDst = translateImagePosition(dst, false);
            emit signalClone(currentSrc, currentDst);
            released = false;
        }
    }
}

void ImageBrushGuideWidget::setSrcSet(bool s)
{
    srcSet = s;
}

bool ImageBrushGuideWidget::isSrcSet()
{
    return srcSet;
}

void ImageBrushGuideWidget::slotSrcSet()
{
    srcSet = !srcSet;
    if(srcSet)
    {
        src =  getSpotPosition();
    }
    released = true;
}
}
