/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-06-15
 * Description : a brush for use with tool to replace part of the image using another
 *
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

#include "imagebrushguidewidget.h"

// Local includes

#include "digikam_debug.h"

namespace DigikamEditorHealingCloneToolPlugin
{

 ImageBrushGuideWidget::ImageBrushGuideWidget(QWidget* const parent ,
                          bool spotVisible,
                          int guideMode ,
                          const QColor& guideColor ,
                          int guideSize ,
                          bool blink ,
                          ImageIface::PreviewType type):
     ImageGuideWidget( parent, spotVisible, guideMode , guideColor,  guideSize , blink, type)
{


}

 void ImageBrushGuideWidget::setDefaults()
 {
     this->default_h = this->height();
     this->default_w = this->width();
     this->float_h = default_h;
     this->float_w = default_w;
 }
void ImageBrushGuideWidget::mouseMoveEvent(QMouseEvent* e)
{
    if(!isMPressed)
        oldPos = e->globalPos() ;
    qCDebug(DIGIKAM_DPLUGIN_EDITOR_LOG) << "MPressed is: " <<isMPressed;
    if (isMPressed)
    {
        const QPoint delta = e->globalPos() - oldPos;
        move(x()+delta.x(), y()+delta.y());
        oldPos = e->globalPos();
        qCDebug(DIGIKAM_DPLUGIN_EDITOR_LOG) << "Right@: " << x() << ", "<< y();
    }
    else if ((e->buttons() & Qt::LeftButton) && !srcSet)
    {
        qCDebug(DIGIKAM_DPLUGIN_EDITOR_LOG) << "Move The location is: " << e->x() << ", "<< e->y();

        QPoint currentDst = QPoint(e->x(), e->y());

        currentDst        = translateItemPosition(currentDst, false);
        QPoint currentSrc = translateItemPosition(src, true);
        QPoint orgDst     = translateItemPosition(dst, false);
        currentSrc        = QPoint(currentSrc.x() + currentDst.x() - orgDst.x(), currentSrc.y() + currentDst.y() - orgDst.y());

        setSpotPosition(currentSrc);

        emit signalClone(currentSrc, currentDst);
    }

    if (srcSet)
    {
        ImageGuideWidget::mouseMoveEvent(e);
    }


}

void ImageBrushGuideWidget::mouseReleaseEvent(QMouseEvent* e)
{
    ImageGuideWidget::mouseReleaseEvent(e);

    if (srcSet)
    {
        src    = getSpotPosition();
        srcSet = false;
    }
    else
    {
        QPoint p = translatePointPosition(src);
        setSpotPosition(p);
    }
}

void ImageBrushGuideWidget::mousePressEvent(QMouseEvent* e)
{

     oldPos = e->globalPos() ;
     if(isMPressed)
     {
         isMPressed = false;
     }

    else if (srcSet)
    {
        ImageGuideWidget::mousePressEvent(e);
    }
    else
    {
        if (e->button() == Qt::LeftButton)
        {
            dst = QPoint(e->x(), e->y());

            QPoint currentSrc = translateItemPosition(src, true);
            QPoint currentDst = translateItemPosition(dst, false);

            emit signalClone(currentSrc, currentDst);
        }

    }

}

void ImageBrushGuideWidget :: keyPressEvent(QKeyEvent *e)
{
    QWidget::keyPressEvent(e);

    // Need to put the setDefault method in its appropriate place. This is a hook.
    if(first_time)
        setDefaults();

    if(e->key() == Qt :: Key_M)
    {

        if(isMPressed)
            isMPressed = false;
        else
            isMPressed = true;
    }

    if(e->key() == Qt :: Key_Plus)
    {
        qCDebug(DIGIKAM_DPLUGIN_EDITOR_LOG) << "AF keyPLus is: " << this->width() << this->height();
        this->float_h += .1 * this->default_h;
        this->float_w += .1 * this->default_w;
        this->resize((int)this->float_w, (int)this->float_h);
    }

    if(e->key() == Qt :: Key_Minus)
    {
        this->float_h -= .1 * this->default_h;
        this->float_w -= .1 * this->default_w;
        this->resize((int)this->float_w, (int)this->float_h);
    }
}

void ImageBrushGuideWidget::  keyReleaseEvent(QKeyEvent *e)
{

}
void ImageBrushGuideWidget::slotSetSourcePoint()
{
    srcSet = true;
}

} // namespace DigikamEditorHealingCloneToolPlugin
