/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date   : 2004-11-16
 * Description : a widget to display an image with guides
 *
 * Copyright 2004-2007 by Gilles Caulier
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
#include <qpen.h>
#include <qpixmap.h>
#include <qtooltip.h>
#include <qtimer.h>
#include <qrect.h>
#include <qbrush.h>
#include <qfont.h> 
#include <qfontmetrics.h> 

// KDE include.

#include <kstandarddirs.h>
#include <kcursor.h>
#include <kglobal.h> 

// Local includes.

#include "ddebug.h"
#include "dimg.h"
#include "imageiface.h"
#include "imageguidewidget.h"
#include "imageguidewidget.moc"

namespace Digikam
{

class ImageGuideWidgetPriv
{
public:

    ImageGuideWidgetPriv()
    {
        pixmap                    = 0;
        iface                     = 0;
        flicker                   = 0;
        timerID                   = 0;
        focus                     = false;
        onMouseMovePreviewToggled = false;
        renderingPreviewMode      = ImageGuideWidget::NoPreviewMode;
        underExposureIndicator    = false;
        overExposureIndicator     = false;
    }

    bool        sixteenBit;
    bool        focus;
    bool        spotVisible;
    bool        onMouseMovePreviewToggled;
    bool        underExposureIndicator;
    bool        overExposureIndicator;
    
    int         width;
    int         height;
    int         timerID;
    int         guideMode;
    int         guideSize;
    int         flicker;
    int         renderingPreviewMode;

    // Current spot position in preview coordinates.
    QPoint      spot;
    
    QRect       rect;
    
    QColor      guideColor;
        
    QPixmap    *pixmap;
    
    ImageIface *iface;
    
    DImg        preview;
};

ImageGuideWidget::ImageGuideWidget(int w, int h, QWidget *parent,
                                   bool spotVisible, int guideMode, 
                                   QColor guideColor, int guideSize, bool blink)
                : QWidget(parent, 0, Qt::WDestructiveClose)
{
    d = new ImageGuideWidgetPriv;
    d->spotVisible = spotVisible;
    d->guideMode   = guideMode;
    d->guideColor  = guideColor;
    d->guideSize   = guideSize;
    
    setBackgroundMode(Qt::NoBackground);
    setMinimumSize(w, h);
    setMouseTracking(true);

    d->iface        = new ImageIface(w, h);
    uchar *data     = d->iface->getPreviewImage();
    d->width        = d->iface->previewWidth();
    d->height       = d->iface->previewHeight();
    bool sixteenBit = d->iface->previewSixteenBit();
    bool hasAlpha   = d->iface->previewHasAlpha();
    d->preview      = DImg(d->width, d->height, sixteenBit, hasAlpha, data);
    delete [] data;

    d->pixmap = new QPixmap(w, h);
    d->rect   = QRect(w/2-d->width/2, h/2-d->height/2, d->width, d->height);

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

void ImageGuideWidget::resetSpotPosition(void)
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

int ImageGuideWidget::getRenderingPreviewMode(void)
{
    return (d->renderingPreviewMode);
}

QPoint ImageGuideWidget::getSpotPosition(void)
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

void ImageGuideWidget::slotChangeGuideColor(const QColor &color)
{
    d->guideColor = color;
    updatePreview();
}

void ImageGuideWidget::slotChangeGuideSize(int size)
{
    d->guideSize = size;
    updatePreview();
}

void ImageGuideWidget::updatePixmap( void )
{
    QPainter p(d->pixmap);
    QString text;
    QRect textRect, fontRect;
    QFontMetrics fontMt = p.fontMetrics();
    p.setPen(QPen(Qt::red, 1)) ;
    
    d->pixmap->fill(colorGroup().background());

    if (d->renderingPreviewMode == PreviewOriginalImage ||
        (d->renderingPreviewMode == PreviewToggleOnMouseOver && d->onMouseMovePreviewToggled == false ))
    {
        p.drawPixmap(d->rect, d->iface->convertToPixmap(d->preview));

        text = i18n("Original");
        fontRect = fontMt.boundingRect(0, 0, d->rect.width(), d->rect.height(), 0, text);
        textRect.setTopLeft(QPoint(d->rect.x() + 20, d->rect.y() + 20));
        textRect.setSize( QSize(fontRect.width(), fontRect.height() ) );
        p.fillRect(textRect, QBrush(QColor(250, 250, 255)) );
        p.drawRect(textRect);
        p.drawText(textRect, Qt::AlignCenter, text);
    }
    else if (d->renderingPreviewMode == PreviewTargetImage || d->renderingPreviewMode == NoPreviewMode ||
            (d->renderingPreviewMode == PreviewToggleOnMouseOver && d->onMouseMovePreviewToggled == true ))
    {
        d->iface->paint(d->pixmap, d->rect.x(), d->rect.y(),
                        d->rect.width(), d->rect.height(), 
                        d->underExposureIndicator, d->overExposureIndicator);

        if (d->renderingPreviewMode == PreviewTargetImage ||
            d->renderingPreviewMode == PreviewToggleOnMouseOver)
        {
            text = i18n("Target");
            fontRect = fontMt.boundingRect(0, 0, d->rect.width(), d->rect.height(), 0, text);
            textRect.setTopLeft(QPoint(d->rect.x() + 20, d->rect.y() + 20));
            textRect.setSize( QSize(fontRect.width(), fontRect.height() ) );
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
            p.drawPixmap(d->rect, d->iface->convertToPixmap(d->preview));

            // Drawing the target image under the original.
            d->iface->paint(d->pixmap,
                            d->rect.x()+d->rect.width()/2,
                            d->rect.y(),
                            d->rect.width()/2,
                            d->rect.height(), 
                            d->underExposureIndicator, 
                            d->overExposureIndicator);
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
                            d->overExposureIndicator);

            // Drawing the original image under the target.
            p.drawPixmap(d->rect.x(), d->rect.y(), d->iface->convertToPixmap(d->preview),
                         0, 0, d->rect.width()/2, d->rect.height());
        }

        // Drawing the information and others stuff.
        p.fillRect(d->rect.right(), 0, width(), height(), colorGroup().background());

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

        text = i18n("Target");
        fontRect = fontMt.boundingRect(0, 0, d->rect.width(), d->rect.height(), 0, text);
        textRect.setTopLeft(QPoint(d->rect.x() + d->rect.width()/2 + 20,
                                   d->rect.y() + 20));
        textRect.setSize( QSize(fontRect.width(), fontRect.height()) );
        p.fillRect(textRect, QBrush(QColor(250, 250, 255)) );
        p.drawRect(textRect);
        p.drawText(textRect, Qt::AlignCenter, text);

        text = i18n("Original");
        fontRect = fontMt.boundingRect(0, 0, d->rect.width(), d->rect.height(), 0, text);
        textRect.setTopLeft(QPoint(d->rect.x() + 20, d->rect.y() + 20));
        textRect.setSize( QSize(fontRect.width(), fontRect.height() ) );
        p.fillRect(textRect, QBrush(QColor(250, 250, 255)) );
        p.drawRect(textRect);
        p.drawText(textRect, Qt::AlignCenter, text);
    }
    else if (d->renderingPreviewMode == PreviewBothImagesHorz ||
             d->renderingPreviewMode == PreviewBothImagesHorzCont)
    {
        if (d->renderingPreviewMode == PreviewBothImagesHorz)
        {
            // Drawing the original image.
            p.drawPixmap(d->rect, d->iface->convertToPixmap(d->preview));

            // Drawing the target image under the original.
            d->iface->paint(d->pixmap,
                            d->rect.x(),
                            d->rect.y()+d->rect.height()/2,
                            d->rect.width(),
                            d->rect.height()/2, 
                            d->underExposureIndicator, 
                            d->overExposureIndicator);
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
                            d->overExposureIndicator);

            // Drawing the original image under the target.
            p.drawPixmap(d->rect.x(), d->rect.y(), d->iface->convertToPixmap(d->preview),
                         0, 0, d->rect.width(), d->rect.height()/2);
        }

        p.fillRect(0, d->rect.bottom(), width(), height(), colorGroup().background());

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

        text = i18n("Target");
        fontRect = fontMt.boundingRect(0, 0, d->rect.width(), d->rect.height(), 0, text);
        textRect.setTopLeft(QPoint(d->rect.x() + 20,
                                   d->rect.y() + d->rect.height()/2 + 20));
        textRect.setSize( QSize(fontRect.width(), fontRect.height()) );
        p.fillRect(textRect, QBrush(QColor(250, 250, 255)) );
        p.drawRect(textRect);
        p.drawText(textRect, Qt::AlignCenter, text);

        text = i18n("Original");
        fontRect = fontMt.boundingRect(0, 0, d->rect.width(), d->rect.height(), 0, text);
        textRect.setTopLeft(QPoint(d->rect.x() + 20, d->rect.y() + 20));
        textRect.setSize( QSize(fontRect.width(), fontRect.height() ) );
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
    
    p.end();
}

void ImageGuideWidget::paintEvent( QPaintEvent * )
{
    bitBlt(this, 0, 0, d->pixmap);
}

void ImageGuideWidget::updatePreview( void )
{
    updatePixmap();
    repaint(false);
}

void ImageGuideWidget::timerEvent(QTimerEvent * e)
{
    if (e->timerId() == d->timerID)
    {
       if (d->flicker == 5) d->flicker=0;
       else d->flicker++;
       updatePreview();
    }
    else
       QWidget::timerEvent(e);
}

void ImageGuideWidget::resizeEvent(QResizeEvent * e)
{
    blockSignals(true);
    delete d->pixmap;
    int w = e->size().width();
    int h = e->size().height();
    int old_w = d->width;
    int old_h = d->height;

    uchar *data     = d->iface->setPreviewImageSize(w, h);
    d->width        = d->iface->previewWidth();
    d->height       = d->iface->previewHeight();
    bool sixteenBit = d->iface->previewSixteenBit();
    bool hasAlpha   = d->iface->previewHasAlpha();
    d->preview      = DImg(d->width, d->height, sixteenBit, hasAlpha, data);
    delete [] data;

    d->pixmap = new QPixmap(w, h);
    d->rect   = QRect(w/2-d->width/2, h/2-d->height/2, d->width, d->height);

    d->spot.setX((int)((float)d->spot.x() * ( (float)d->width  / (float)old_w)));
    d->spot.setY((int)((float)d->spot.y() * ( (float)d->height / (float)old_h)));
    updatePixmap();
    blockSignals(false);
    emit signalResized();
}

void ImageGuideWidget::mousePressEvent ( QMouseEvent * e )
{
    if ( !d->focus && e->button() == Qt::LeftButton &&
         d->rect.contains( e->x(), e->y() ) && d->spotVisible )
    {
       d->focus = true;
       d->spot.setX(e->x()-d->rect.x());
       d->spot.setY(e->y()-d->rect.y());
       updatePreview();
    }
}

void ImageGuideWidget::mouseReleaseEvent ( QMouseEvent *e )
{
    if ( d->rect.contains( e->x(), e->y() ) && d->focus && d->spotVisible)
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
}

void ImageGuideWidget::mouseMoveEvent ( QMouseEvent * e )
{
    if ( d->rect.contains( e->x(), e->y() ) && !d->focus && d->spotVisible )
    {
        setCursor( KCursor::crossCursor() );
    }
    else if ( d->rect.contains( e->x(), e->y() ) && d->focus && d->spotVisible )
    {
        d->spot.setX(e->x()-d->rect.x());
        d->spot.setY(e->y()-d->rect.y());
    }
    else
    {
        unsetCursor();
    }
}

void ImageGuideWidget::enterEvent( QEvent * )
{
    if ( !d->focus && d->renderingPreviewMode == PreviewToggleOnMouseOver )
    {
        d->onMouseMovePreviewToggled = false;
        updatePixmap();
        repaint(false);
    }
}

void ImageGuideWidget::leaveEvent( QEvent * )
{
    if ( !d->focus && d->renderingPreviewMode == PreviewToggleOnMouseOver )
    {
        d->onMouseMovePreviewToggled = true;
        updatePixmap();
        repaint(false);
    }
}

}  // NameSpace Digikam

