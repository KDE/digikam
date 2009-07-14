/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-11-16
 * Description : a widget to display an image with guides
 *
 * Copyright (C) 2004-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "imageguidewidget.h"
#include "imageguidewidget.moc"

// Qt includes

#include <QRegion>
#include <QPainter>
#include <QPen>
#include <QPixmap>
#include <QTimer>
#include <QRect>
#include <QBrush>
#include <QFont>
#include <QFontMetrics>
#include <QTimerEvent>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QEvent>

// KDE includes

#include <kdebug.h>
#include <kstandarddirs.h>
#include <kcursor.h>
#include <kglobal.h>

// Local includes

#include "dimg.h"
#include "imageiface.h"

namespace Digikam
{

class ImageGuideWidgetPriv
{
public:

    ImageGuideWidgetPriv()
    {
        pixmap                    = 0;
        maskPixmap                = 0;
        previewPixmap             = 0;
        iface                     = 0;
        flicker                   = 0;
        timerID                   = 0;
        focus                     = false;
        onMouseMovePreviewToggled = true;
        renderingPreviewMode      = ImageGuideWidget::NoPreviewMode;
        underExposureIndicator    = false;
        overExposureIndicator     = false;
        drawLineBetweenPoints     = false;
        drawingMask               = false;
        enableDrawMask            = false;
        penWidth                  = 10;
        eraseMask                 = false;
    }

    bool        sixteenBit;
    bool        focus;
    bool        spotVisible;
    bool        onMouseMovePreviewToggled;
    bool        underExposureIndicator;
    bool        overExposureIndicator;
    bool        drawLineBetweenPoints;
    bool        drawingMask;
    bool        enableDrawMask;
    bool        eraseMask;

    int         width;
    int         height;
    int         timerID;
    int         guideMode;
    int         guideSize;
    int         flicker;
    int         renderingPreviewMode;
    int         penWidth;

    // Current spot position in preview coordinates.
    QPoint      spot;
    QPolygon    selectedPoints;

    QRect       rect;

    QColor      guideColor;
    QColor      paintColor;

    QPixmap    *pixmap;
    QPixmap    *maskPixmap;
    QPixmap    *previewPixmap;

    QCursor     maskCursor;

    QPoint      lastPoint;

    ImageIface *iface;

    DImg        preview;
};

ImageGuideWidget::ImageGuideWidget(int w, int h, QWidget *parent,
                                   bool spotVisible, int guideMode,
                                   const QColor& guideColor, int guideSize,
                                   bool blink, bool useImageSelection)
                : QWidget(parent), d(new ImageGuideWidgetPriv)
{
    d->spotVisible = spotVisible;
    d->guideMode   = guideMode;
    d->guideColor  = guideColor;
    d->guideSize   = guideSize;

    setMinimumSize(w, h);
    setMouseTracking(true);
    setAttribute(Qt::WA_DeleteOnClose);

    d->iface        = new ImageIface(w, h);
    d->iface->setPreviewType(useImageSelection);
    uchar *data     = d->iface->getPreviewImage();
    d->width        = d->iface->previewWidth();
    d->height       = d->iface->previewHeight();
    bool sixteenBit = d->iface->previewSixteenBit();
    bool hasAlpha   = d->iface->previewHasAlpha();
    d->preview      = DImg(d->width, d->height, sixteenBit, hasAlpha, data);
    d->preview.setICCProfil( d->iface->getOriginalImg()->getICCProfil() );
    delete [] data;

    d->pixmap        = new QPixmap(w, h);
    d->rect          = QRect(w/2-d->width/2, h/2-d->height/2, d->width, d->height);
    d->maskPixmap    = new QPixmap(d->rect.width(), d->rect.height());
    d->previewPixmap = new QPixmap(d->rect.width(), d->rect.height());
    d->maskPixmap->fill(QColor(0,0,0,0));
    d->previewPixmap->fill(QColor(0,0,0,0));


    d->paintColor.setRgb(255, 255, 255, 255);

    d->lastPoint = QPoint(d->rect.x(),d->rect.y());

    resetSpotPosition();
    setSpotVisible(d->spotVisible, blink);
}

ImageGuideWidget::~ImageGuideWidget()
{
    delete d->iface;

    if (d->timerID)
        killTimer(d->timerID);

    if (d->pixmap)
        delete d->pixmap;

    if(d->maskPixmap)
        delete d->maskPixmap;

    if(d->previewPixmap)
        delete d->previewPixmap;

    delete d;
}

ImageIface* ImageGuideWidget::imageIface()
{
    return d->iface;
}

void ImageGuideWidget::slotToggleUnderExposure(bool u)
{
    d->underExposureIndicator = u;
    updatePreview();
}

void ImageGuideWidget::slotToggleOverExposure(bool o)
{
    d->overExposureIndicator = o;
    updatePreview();
}

void ImageGuideWidget::resetSpotPosition()
{
    d->spot.setX( d->width  / 2 );
    d->spot.setY( d->height / 2 );
    updatePreview();
}

void ImageGuideWidget::slotChangeRenderingPreviewMode(int mode)
{
    d->renderingPreviewMode = mode;
    updatePreview();
}

int ImageGuideWidget::getRenderingPreviewMode()
{
    return (d->renderingPreviewMode);
}

void ImageGuideWidget::setRenderingPreviewMode(int mode)
{
    d->renderingPreviewMode = mode;
}

QPoint ImageGuideWidget::getSpotPosition()
{
    return (QPoint( (int)((float)d->spot.x() * (float)d->iface->originalWidth()  / (float)d->width),
                    (int)((float)d->spot.y() * (float)d->iface->originalHeight() / (float)d->height)));
}

DColor ImageGuideWidget::getSpotColor(int getColorFrom)
{
    if (getColorFrom == OriginalImage)                          // Get point color from full original image
        return (d->iface->getColorInfoFromOriginalImage(getSpotPosition()));
    else if (getColorFrom == PreviewImage)                      // Get point color from full preview image
        return (d->iface->getColorInfoFromPreviewImage(d->spot));

    // In other cases, get point color from preview target image
    return (d->iface->getColorInfoFromTargetPreviewImage(d->spot));
}

void ImageGuideWidget::setSpotVisible(bool spotVisible, bool blink)
{
    d->spotVisible = spotVisible;

    if (blink)
    {
        if (d->spotVisible)
            d->timerID = startTimer(800);
        else
        {
            killTimer(d->timerID);
            d->timerID = 0;
        }
    }

    updatePreview();
}

void ImageGuideWidget::slotChangeGuideColor(const QColor& color)
{
    d->guideColor = color;
    updatePreview();
}

void ImageGuideWidget::slotChangeGuideSize(int size)
{
    d->guideSize = size;
    updatePreview();
}

void ImageGuideWidget::updatePixmap()
{
    QPainter p(d->pixmap);
    QString text;
    QRect textRect, fontRect;
    QFontMetrics fontMt = p.fontMetrics();
    p.setPen(QPen(Qt::red, 1));

    d->pixmap->fill(palette().color(QPalette::Background));

    if (d->renderingPreviewMode == PreviewOriginalImage ||
        (d->renderingPreviewMode == PreviewToggleOnMouseOver && d->onMouseMovePreviewToggled == false ))
    {
        p.drawPixmap(d->rect, *d->previewPixmap);

        text     = i18n("Original");
        fontRect = fontMt.boundingRect(0, 0, d->rect.width(), d->rect.height(), 0, text);
        textRect.setTopLeft(QPoint(d->rect.x() + 20, d->rect.y() + 20));
        textRect.setSize( QSize(fontRect.width()+2, fontRect.height()+2 ) );
        p.fillRect(textRect, QBrush(QColor(250, 250, 255)) );
        p.drawRect(textRect);
        p.drawText(textRect, Qt::AlignCenter, text);
    }
    else if (d->renderingPreviewMode == PreviewTargetImage || d->renderingPreviewMode == NoPreviewMode ||
            (d->renderingPreviewMode == PreviewToggleOnMouseOver && d->onMouseMovePreviewToggled == true ))
    {
        d->iface->paint(d->pixmap,
                        d->rect.x(),
                        d->rect.y(),
                        d->rect.width(),
                        d->rect.height(),
                        d->underExposureIndicator,
                        d->overExposureIndicator,
                        &p);

        if (d->renderingPreviewMode == PreviewTargetImage ||
            d->renderingPreviewMode == PreviewToggleOnMouseOver)
        {
            text     = i18n("Target");
            fontRect = fontMt.boundingRect(0, 0, d->rect.width(), d->rect.height(), 0, text);
            textRect.setTopLeft(QPoint(d->rect.x() + 20, d->rect.y() + 20));
            textRect.setSize( QSize(fontRect.width()+2, fontRect.height()+2 ) );
            p.fillRect(textRect, QBrush(QColor(250, 250, 255)) );
            p.drawRect(textRect);
            p.drawText(textRect, Qt::AlignCenter, text);
        }
    }
    else if (d->renderingPreviewMode == PreviewBothImagesVert ||
             d->renderingPreviewMode == PreviewBothImagesVertCont)
    {
        if (d->renderingPreviewMode == PreviewBothImagesVert)
        {
            // Drawing the original image.
            p.drawPixmap(d->rect, *d->previewPixmap);

            // Drawing the target image under the original.
            d->iface->paint(d->pixmap,
                            d->rect.x()+d->rect.width()/2,
                            d->rect.y(),
                            d->rect.width()/2,
                            d->rect.height(),
                            d->underExposureIndicator,
                            d->overExposureIndicator,
                            &p);
        }
        else
        {
            // Drawing the target image.
            d->iface->paint(d->pixmap,
                            d->rect.x(),
                            d->rect.y(),
                            d->rect.width(),
                            d->rect.height(),
                            d->underExposureIndicator,
                            d->overExposureIndicator,
                            &p);

            // Drawing the original image under the target.
            p.drawPixmap(d->rect.x(), d->rect.y(), *d->previewPixmap,
                         0, 0, d->rect.width()/2, d->rect.height());
        }

        // Drawing the information and others stuff.
        p.fillRect(d->rect.right(), 0, width(), height(), palette().color(QPalette::Background));

        p.setPen(QPen(Qt::white, 2, Qt::SolidLine));
        p.drawLine(d->rect.x()+d->rect.width()/2-1,
                   d->rect.y(),
                   d->rect.x()+d->rect.width()/2-1,
                   d->rect.y()+d->rect.height());
        p.setPen(QPen(Qt::red, 2, Qt::DotLine));
        p.drawLine(d->rect.x()+d->rect.width()/2-1,
                   d->rect.y(),
                   d->rect.x()+d->rect.width()/2-1,
                   d->rect.y()+d->rect.height());

        p.setPen(QPen(Qt::red, 1)) ;

        text     = i18n("Target");
        fontRect = fontMt.boundingRect(0, 0, d->rect.width(), d->rect.height(), 0, text);
        textRect.setTopLeft(QPoint(d->rect.x() + d->rect.width()/2 + 20,
                                   d->rect.y() + 20));
        textRect.setSize( QSize(fontRect.width()+2, fontRect.height()+2) );
        p.fillRect(textRect, QBrush(QColor(250, 250, 255)) );
        p.drawRect(textRect);
        p.drawText(textRect, Qt::AlignCenter, text);

        text     = i18n("Original");
        fontRect = fontMt.boundingRect(0, 0, d->rect.width(), d->rect.height(), 0, text);
        textRect.setTopLeft(QPoint(d->rect.x() + 20, d->rect.y() + 20));
        textRect.setSize( QSize(fontRect.width()+2, fontRect.height()+2 ) );
        p.fillRect(textRect, QBrush(QColor(250, 250, 255)));
        p.drawRect(textRect);
        p.drawText(textRect, Qt::AlignCenter, text);
    }
    else if (d->renderingPreviewMode == PreviewBothImagesHorz ||
             d->renderingPreviewMode == PreviewBothImagesHorzCont)
    {
        if (d->renderingPreviewMode == PreviewBothImagesHorz)
        {
            // Drawing the original image.
            p.drawPixmap(d->rect, *d->previewPixmap);

            // Drawing the target image under the original.
            d->iface->paint(d->pixmap,
                            d->rect.x(),
                            d->rect.y()+d->rect.height()/2,
                            d->rect.width(),
                            d->rect.height()/2,
                            d->underExposureIndicator,
                            d->overExposureIndicator,
                            &p);
        }
        else
        {
            // Drawing the target image.
            d->iface->paint(d->pixmap,
                            d->rect.x(),
                            d->rect.y(),
                            d->rect.width(),
                            d->rect.height(),
                            d->underExposureIndicator,
                            d->overExposureIndicator,
                            &p);

            // Drawing the original image under the target.
            p.drawPixmap(d->rect.x(), d->rect.y(), *d->previewPixmap,
                         0, 0, d->rect.width(), d->rect.height()/2);
        }

        p.fillRect(0, d->rect.bottom(), width(), height(), palette().color(QPalette::Background));

        p.setPen(QPen(Qt::white, 2, Qt::SolidLine));
        p.drawLine(d->rect.x(),
                   d->rect.y()+d->rect.height()/2-1,
                   d->rect.x()+d->rect.width(),
                   d->rect.y()+d->rect.height()/2-1);
        p.setPen(QPen(Qt::red, 2, Qt::DotLine));
        p.drawLine(d->rect.x(),
                   d->rect.y()+d->rect.height()/2-1,
                   d->rect.x()+d->rect.width(),
                   d->rect.y()+d->rect.height()/2-1);

        p.setPen(QPen(Qt::red, 1)) ;

        text     = i18n("Target");
        fontRect = fontMt.boundingRect(0, 0, d->rect.width(), d->rect.height(), 0, text);
        textRect.setTopLeft(QPoint(d->rect.x() + 20,
                                   d->rect.y() + d->rect.height()/2 + 20));
        textRect.setSize( QSize(fontRect.width()+2, fontRect.height()+2) );
        p.fillRect(textRect, QBrush(QColor(250, 250, 255)) );
        p.drawRect(textRect);
        p.drawText(textRect, Qt::AlignCenter, text);

        text     = i18n("Original");
        fontRect = fontMt.boundingRect(0, 0, d->rect.width(), d->rect.height(), 0, text);
        textRect.setTopLeft(QPoint(d->rect.x() + 20, d->rect.y() + 20));
        textRect.setSize( QSize(fontRect.width()+2, fontRect.height()+2 ) );
        p.fillRect(textRect, QBrush(QColor(250, 250, 255)) );
        p.drawRect(textRect);
        p.drawText(textRect, Qt::AlignCenter, text);
    }

    if (d->spotVisible)
    {
       // Adapt spot from image coordinate to widget coordinate.
       int xspot = d->spot.x() + d->rect.x();
       int yspot = d->spot.y() + d->rect.y();

       switch (d->guideMode)
       {
          case HVGuideMode:
          {
             p.setPen(QPen(Qt::white, d->guideSize, Qt::SolidLine));
             p.drawLine(xspot, d->rect.top() + d->flicker, xspot, d->rect.bottom() - d->flicker);
             p.drawLine(d->rect.left() + d->flicker, yspot, d->rect.right() - d->flicker, yspot);
             p.setPen(QPen(d->guideColor, d->guideSize, Qt::DotLine));
             p.drawLine(xspot, d->rect.top() + d->flicker, xspot, d->rect.bottom() - d->flicker);
             p.drawLine(d->rect.left() + d->flicker, yspot, d->rect.right() - d->flicker, yspot);
             break;
          }

          case PickColorMode:
          {
             p.setPen(QPen(d->guideColor, 1, Qt::SolidLine));
             p.drawLine(xspot-10, yspot-10, xspot+10, yspot+10);
             p.drawLine(xspot+10, yspot-10, xspot-10, yspot+10);
             p.setPen(QPen(d->guideColor, 3, Qt::SolidLine));
             p.drawEllipse( xspot-5, yspot-5, 11, 11 );

             if (d->flicker%2 != 0)
             {
                p.setPen(QPen(Qt::white, 1, Qt::SolidLine));
                p.drawEllipse( xspot-5, yspot-5, 11, 11 );
             }

             break;
          }
       }

    }

    // draw additional points added by the image plugin
    if (d->selectedPoints.count() > 0)
    {
        QPainter::RenderHints hints = p.renderHints();

        QColor semiTransGuideColor  = QColor(d->guideColor.red(),
                                             d->guideColor.green(),
                                             d->guideColor.blue(),
                                             75);

        QPoint point;
        int x = 0;
        int y = 0;

        for (int i = 0; i < d->selectedPoints.count(); ++i)
        {
            point = d->selectedPoints.point(i);
            point = translatePointPosition(point);
            x     = point.x();
            y     = point.y();

            p.save();
            p.setRenderHint(QPainter::Antialiasing, true);
            p.setPen(QPen(d->guideColor, 2, Qt::SolidLine));
            p.setBrush(QBrush(semiTransGuideColor));
            p.drawEllipse(point, 6, 6);

            p.restore();
            p.setPen(QPen(d->guideColor, 1, Qt::SolidLine));
            p.setBrush(Qt::NoBrush);
            p.setRenderHint(QPainter::Antialiasing, false);
            p.drawPoint(point);
            p.drawText(QPoint(x+10, y-5), QString::number(i+1));

            // draw a line between the points
            if (d->drawLineBetweenPoints &&
               (i+1) < d->selectedPoints.count() && !d->selectedPoints.point(i+1).isNull())
            {
                p.save();
                p.setPen(QPen(d->guideColor, d->guideSize, Qt::SolidLine));
                QPoint point2 = d->selectedPoints.point(i+1);
                point2        = translatePointPosition(point2);
                p.setRenderHint(QPainter::Antialiasing, true);
                p.drawLine(point, point2);
                p.restore();
            }

        }

        p.setRenderHints(hints);
    }

    p.end();
}

void ImageGuideWidget::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.drawPixmap(0, 0, *d->pixmap);

    if (d->enableDrawMask && d->onMouseMovePreviewToggled == false )
    {
        p.setOpacity(0.7);
        p.drawPixmap(d->rect.x(), d->rect.y(), *d->maskPixmap);
    }

    p.end();
}

void ImageGuideWidget::updatePreview()
{
    updatePixmap();
    update();
}

void ImageGuideWidget::timerEvent(QTimerEvent *e)
{
    if (e->timerId() == d->timerID)
    {
       if (d->flicker == 5) d->flicker=0;
       else d->flicker++;
       updatePreview();
    }
    else
    {
       QWidget::timerEvent(e);
    }
}

void ImageGuideWidget::resizeEvent(QResizeEvent *e)
{
    blockSignals(true);
    delete d->pixmap;
    delete d->previewPixmap;

    int w     = e->size().width();
    int h     = e->size().height();
    int old_w = d->width;
    int old_h = d->height;
    
    uchar *data     = d->iface->setPreviewImageSize(w, h);
    d->width        = d->iface->previewWidth();
    d->height       = d->iface->previewHeight();
    bool sixteenBit = d->iface->previewSixteenBit();
    bool hasAlpha   = d->iface->previewHasAlpha();
    d->preview      = DImg(d->width, d->height, sixteenBit, hasAlpha, data);
    d->preview.setICCProfil( d->iface->getOriginalImg()->getICCProfil() );
    delete [] data;

    d->pixmap         = new QPixmap(w, h);
    d->previewPixmap  = new QPixmap(w, h);
    d->rect           = QRect(w/2-d->width/2, h/2-d->height/2, d->width, d->height);
    *d->maskPixmap    = d->maskPixmap->scaled(d->width,d->height,Qt::IgnoreAspectRatio);
    *d->previewPixmap = d->iface->convertToPixmap(d->preview);

    d->spot.setX((int)((float)d->spot.x() * ( (float)d->width  / (float)old_w)));
    d->spot.setY((int)((float)d->spot.y() * ( (float)d->height / (float)old_h)));
    updatePixmap();
    blockSignals(false);
    emit signalResized();
}

void ImageGuideWidget::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton)
    {
        if ( !d->focus && d->rect.contains( e->x(), e->y() ) && d->spotVisible )
        {
            d->focus = true;
            d->spot.setX(e->x()-d->rect.x());
            d->spot.setY(e->y()-d->rect.y());
        }
        else if (d->enableDrawMask)
        {
            d->lastPoint = QPoint(e->x()-d->rect.x(), e->y()-d->rect.y());
            d->drawingMask = true;
        }

        updatePreview();
    }
}

void ImageGuideWidget::mouseReleaseEvent(QMouseEvent *e)
{
    if (d->rect.contains(e->x(), e->y()))
    {
        if ( d->focus && d->spotVisible)
        {
            d->focus = false;
            updatePreview();
            d->spot.setX(e->x()-d->rect.x());
            d->spot.setY(e->y()-d->rect.y());

            DColor color;
            QPoint point = getSpotPosition();

            if (d->renderingPreviewMode == PreviewOriginalImage)
            {
                color = getSpotColor(OriginalImage);
                emit spotPositionChangedFromOriginal( color, d->spot );
            }
            else if (d->renderingPreviewMode == PreviewTargetImage || d->renderingPreviewMode == NoPreviewMode)
            {
                color = getSpotColor(TargetPreviewImage);
                emit spotPositionChangedFromTarget( color, d->spot );
            }
            else if (d->renderingPreviewMode == PreviewBothImagesVert)
            {
                if (d->spot.x() > d->rect.width()/2)
                {
                    color = getSpotColor(TargetPreviewImage);
                    emit spotPositionChangedFromTarget(color, QPoint(d->spot.x() - d->rect.width()/2,
                                                    d->spot.y()));
                }
                else
                {
                    color = getSpotColor(OriginalImage);
                    emit spotPositionChangedFromOriginal( color, d->spot );
                }
            }
            else if (d->renderingPreviewMode == PreviewBothImagesVertCont)
            {
                if (d->spot.x() > d->rect.width()/2)
                {
                    color = getSpotColor(TargetPreviewImage);
                    emit spotPositionChangedFromTarget( color, d->spot);
                }
                else
                {
                    color = getSpotColor(OriginalImage);
                    emit spotPositionChangedFromOriginal( color, d->spot );
                }
            }
            else if (d->renderingPreviewMode == PreviewBothImagesHorz)
            {
                if (d->spot.y() > d->rect.height()/2)
                {
                    color = getSpotColor(TargetPreviewImage);
                    emit spotPositionChangedFromTarget(color, QPoint(d->spot.x(),
                                                    d->spot.y() - d->rect.height()/2 ));
                }
                else
                {
                    color = getSpotColor(OriginalImage);
                    emit spotPositionChangedFromOriginal( color, d->spot );
                }
            }
            else if (d->renderingPreviewMode == PreviewBothImagesHorzCont)
            {
                if (d->spot.y() > d->rect.height()/2)
                {
                    color = getSpotColor(TargetPreviewImage);
                    emit spotPositionChangedFromTarget( color, d->spot);
                }
                else
                {
                    color = getSpotColor(OriginalImage);
                    emit spotPositionChangedFromOriginal( color, d->spot );
                }
            }
        }
        else if (e->button() == Qt::LeftButton && d->drawingMask)
        {
            d->drawingMask = false;
            updatePreview();
        }
    }
}

void ImageGuideWidget::mouseMoveEvent(QMouseEvent *e)
{
    if (d->rect.contains(e->x(), e->y()))
    {
//         setCursor(Qt::CrossCursor);

        if ( d->focus && d->spotVisible )
        {
            setCursor(Qt::CrossCursor);
            d->spot.setX(e->x()-d->rect.x());
            d->spot.setY(e->y()-d->rect.y());
        }
        else if (d->enableDrawMask)
        {
            setCursor(d->maskCursor);
            if ((e->buttons() & Qt::LeftButton) && d->drawingMask)
            {
                QPoint currentPos = QPoint(e->x()-d->rect.x(), e->y()-d->rect.y());
                drawLineTo(currentPos);
                updatePreview();
            }
        }
    }
    else
    {
        unsetCursor();
    }
}

void ImageGuideWidget::enterEvent(QEvent*)
{
    if ( !d->focus && d->renderingPreviewMode == PreviewToggleOnMouseOver )
    {
        d->onMouseMovePreviewToggled = false;
        updatePixmap();
        repaint();
    }
}

void ImageGuideWidget::leaveEvent(QEvent*)
{
    if ( !d->focus && d->renderingPreviewMode == PreviewToggleOnMouseOver )
    {
        d->onMouseMovePreviewToggled = true;
        updatePixmap();
        update();
    }
}

void ImageGuideWidget::setPoints(const QPolygon& p, bool drawLine)
{
    d->selectedPoints        = p;
    d->drawLineBetweenPoints = drawLine;
    updatePreview();
}

void ImageGuideWidget::resetPoints()
{
    d->selectedPoints.clear();
}

void ImageGuideWidget::drawLineTo(const QPoint& endPoint)
{
    drawLineTo(d->penWidth, d->eraseMask, d->paintColor, d->lastPoint, endPoint);
}

void ImageGuideWidget::drawLineTo(int width, bool erase, const QColor& color, const QPoint& start, const QPoint& end)
{
    QPainter painter(d->maskPixmap);
    if (erase) 
      {
       painter.setPen(QPen(QBrush(Qt::transparent), width, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
       painter.setCompositionMode(QPainter::CompositionMode_Clear);
      }
    else
       painter.setPen(QPen(color, width, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.drawLine(start, end);

    int rad = (width / 2) + 2;

    update(QRect(start, end).normalized().adjusted(-rad, -rad, +rad, +rad));
    d->lastPoint = end;

    painter.end();
}

void ImageGuideWidget::setPaintColor(const QColor& color)
{
    d->paintColor = color;
}

void ImageGuideWidget::setMaskEnabled(bool enabled)
{
    d->enableDrawMask = enabled;
    updateMaskCursor();
    updatePreview();
}

void ImageGuideWidget::setEraseMode(bool erase)
{
    d->eraseMask = erase;
}

QImage ImageGuideWidget::getMask() const
{
    QImage mask = d->maskPixmap->toImage();
    return mask;
}

QPoint ImageGuideWidget::translatePointPosition(QPoint& point)
{
    int x = (int)(point.x() * (float)(d->width)  / (float) d->iface->originalWidth());
    int y = (int)(point.y() * (float)(d->height) / (float) d->iface->originalHeight());
    x += d->rect.x() + 1;
    y += d->rect.y() + 1;
    return (QPoint(x,y));
}

void ImageGuideWidget::setMaskPenSize(int size)
{
    d->penWidth = size;
    updateMaskCursor();
}

void ImageGuideWidget::updateMaskCursor()
{
    int size = d->penWidth;
    if (size > 64)
        size = 64;

    QPixmap pix(size, size);
    pix.fill(Qt::transparent);

    QPainter p(&pix);
    p.drawEllipse( 0, 0, size-1, size-1);

    d->maskCursor = QCursor(pix);

}
}  // namespace Digikam
