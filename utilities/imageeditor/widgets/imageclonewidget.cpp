/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-06-20
 * Description : a digiKam image plugin to clone area .
 *
 * Copyright (C) 2011-06-20 by Zhang Jie <zhangjiehangyuan2005 dot at gmail dot com>
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

#include "imageclonewidget.moc"

// Qt includes

#include <QRegion>
#include <QPainter>
#include <QPen>
#include <QTimer>
#include <QRect>
#include <QBrush>
#include <QFont>
#include <QFontMetrics>

// KDE includes

#include <kstandarddirs.h>
#include <kcursor.h>
#include <kdebug.h>

//local includes

#include "imageiface.h"

namespace Digikam
{

class ImageCloneWidget::ImageCloneWidgetPriv
{
public:
    ImageCloneWidgetPriv():
        sixteenBit(false),
        width(0),
        height(0),
        Border(0),
        pixmap(0),
        preveiwMask(0),
        maskImage(0),
        preview(0),
        iface(0)
    {
    }

    bool           sixteenBit;
    int            width;
    int            height;
    QRect          rect;
    int            Border;
    QColor         MASK_BG;     // background color of maskimage
    QColor         MASK_C;      // forground color of maskimage
    QColor         bgColor;     // background color of the widget
    QPoint         centerPoint; // selected centerPoint of the source area
    QPoint         startPoint;  // the first point of a stroke
    QPoint         dis;         // dis means the distance between startPoit and centerPoint, the value can be negative
                                // Current spot position in preview coordinates.
    QPoint         spot;
    QPixmap*       pixmap;

    DImg*          preveiwMask;
    DImg*          maskImage;
//  DImg*          origImage;
    DImg*          preview;
    ImageIface*    iface;

    CloneContainer settings;
};

ImageCloneWidget::ImageCloneWidget(QWidget* parent, const CloneContainer& settings)
    : QWidget(parent), d(new ImageCloneWidgetPriv)
{
      int w = 480, h = 320;
      setMinimumSize(w, h);
      setMouseTracking(true);
      setAttribute(Qt::WA_DeleteOnClose);

      d->iface        = new ImageIface(w, h);
//FIXME      d->iface->setPreviewType(useImageSelection);
      uchar* data     = d->iface->getPreviewImage();
      d->width        = d->iface->previewWidth();
      d->height       = d->iface->previewHeight();
      bool sixteenBit = d->iface->previewSixteenBit();
      bool hasAlpha   = d->iface->previewHasAlpha();
      d->preview      = new DImg(d->width, d->height, sixteenBit, hasAlpha, data);
      d->preview->setIccProfile( d->iface->getOriginalImg()->getIccProfile() );
      delete [] data;

      d->pixmap        = new QPixmap(w, h);
      d->rect          = QRect(w/2-d->width/2, h/2-d->height/2, d->width, d->height);

      d->spot.setX( d->width  / 2 );
      d->spot.setY( d->height / 2 );

      // setMouseTracking(true);//alreadhave in imageguidewidget
      d->centerPoint.setX(0);
      d->centerPoint.setY(0);
      d->startPoint.setX(0);
      d->startPoint.setY(0);
      d->dis.setX(0);
      d->dis.setY(0);
//      d->MASK_BG         = new QColor(255,255,255);
//      d->MASK_C          = new QColor(0,0,0);
      d->bgColor         = palette().color(QPalette::Base);
      d->Border          = 10;
      d->MASK_BG.setRgb(255,255,255,255);
      d->MASK_C.setRgb(0,0,0,255);
      //DColor* backColor  = new DColor(255,255,255,255,false);//fillt maskImage with white
      //origImage = DImg(&imageIface()->getOriginalImg()->copy());

      w       = imageIface()->getOriginalImg()->width();
      h       = imageIface()->getOriginalImg()->height();
      
      bool sixteen = imageIface()->getOriginalImg()->sixteenBit();
      bool alpha   = imageIface()->getOriginalImg()->hasAlpha();
      d->maskImage = new DImg(w, h, sixteen, alpha);
      d->maskImage->fill(DColor(d->MASK_BG));

      w       = d->preview->width();
      h       = d->preview->height();
      sixteen = d->preview->sixteenBit();
      alpha   = d->preview->hasAlpha();
      d->preveiwMask = new DImg(w, h, sixteen, alpha);
      d->preveiwMask->fill(DColor(d->MASK_BG));
      d->settings    = settings;
 }

ImageCloneWidget::~ImageCloneWidget()
{
    delete d->iface;
    if (d->pixmap)
    {
        delete d->pixmap;
    }

    if (d->preveiwMask)
    {
        delete d->preveiwMask;
    }

    if (d->maskImage)
    {
        delete d->maskImage;
    }

    if (d->preview)
    {
    delete d->preview;
    }

    delete d;
}

ImageIface* ImageCloneWidget::imageIface() const
{
    return d->iface;
}

void ImageCloneWidget::setBackgroundColor(const QColor& bg)
{
    d->bgColor = bg;
    updatePreview();
}

void   ImageCloneWidget::setPreview()
{
    uchar* data     = d->iface->getPreviewImage();
    d->preview->detach();
    d->preview->putImageData(data);
    delete [] data;
    d->preview->setIccProfile( d->iface->getOriginalImg()->getIccProfile() );
    d->width        = d->iface->previewWidth();
    d->height       = d->iface->previewHeight();

    int  w       = d->preview->width();
    int  h       = d->preview->height();
    bool sixteen = d->preview->sixteenBit();
    bool alpha   = d->preview->hasAlpha();
    d->preveiwMask->detach();
    d->preveiwMask = new DImg(w, h, sixteen, alpha);
    d->preveiwMask->fill(DColor(d->MASK_BG));

    w       = imageIface()->getOriginalImg()->width();
    h       = imageIface()->getOriginalImg()->height();
    sixteen = imageIface()->getOriginalImg()->sixteenBit();
    alpha   = imageIface()->getOriginalImg()->hasAlpha();
    d->maskImage->detach();
    d->maskImage = new DImg(w, h, sixteen, alpha);
    d->maskImage->fill(DColor(d->MASK_BG));  
}


/*
void   ImageCloneWidget::setOriginalImage()
{
    uchar* data     = d->iface->getOriginalImg();
    d->origImage->detach();
    d->origImage->putImageData(data);
    d->origImage->setIccProfile( d->iface->getOriginalImg()->getIccProfile() );
}
*/

void ImageCloneWidget::setContainer(const CloneContainer& settings)
{
    d->settings = settings;
}

CloneContainer ImageCloneWidget::getContainer() const
{
    return d->settings;
}

void ImageCloneWidget:: upDis()
{
    d->dis.setX(d->centerPoint.x() - d->startPoint.x());
    d->dis.setY(d->centerPoint.y() - d->startPoint.y());
}

QPoint ImageCloneWidget::getDis() const
{
    return d->dis;
}

QPoint ImageCloneWidget::getOriDis() 
{
    float ratio = d->iface->originalWidth()/d->preview->width();
    QPoint p;
    p.setX(d->dis.x()*ratio);
    p.setY(d->dis.y()*ratio);
    return p;
 }

/*
DImg* ImageCloneWidget::getOrigImage()
{
    return d->origImage;
}

DImg* ImageCloneWidget::getPreview()
{
    return d->preview;
}
*/

DImg* ImageCloneWidget::getMaskImg() const
{
    return d->maskImage;
}

DImg*  ImageCloneWidget::getPreviewMask() const
{
    return d->preveiwMask;
}

bool ImageCloneWidget::inimage(DImg *img, const int x, const int y ) 
{
    if ( x >= 0 && (uint)x < img->width() && y >= 0 && (uint)y < img->height())
        return true;
    else
        return false;
}

bool ImageCloneWidget::inBrushpixmap(QPixmap* brushmap, const int x, const int y)
{
    if ( x >= 0 && x < brushmap->width() && y >= 0 && y < brushmap->height())
        return true;
    else
        return false;
}

void ImageCloneWidget::TreateAsBordor(DImg* image, const int x, const int y)
{
    float ratio = d->iface->originalWidth()/d->preview->width();
    float alpha = d->settings.opacity/100; // brush.alpha between 0 and 100
    DColor forgcolor;
    DColor bacgcolor;
    DColor newcolor;

    if ( (-d->Border < (x-getOriDis().x()) && (x-getOriDis().x()) <= 0) &&
         ((y-getOriDis().y()) < image->height() && 0<= (y-getOriDis().y()))
       )
    {
        forgcolor = d->preview->getPixelColor(0,y-getOriDis().y());
    }
    else if ( ((int)image->width()-1 <= x-getOriDis().x()) &&
              (x-getOriDis().x()< (image->width()+d->Border)) &&
              (y-getOriDis().y() < image->height()) &&
              (0<= y-getOriDis().y())
            )
    {
        forgcolor = d->preview->getPixelColor(d->preview->width()-1,(y-getOriDis().y())/ratio);

    }
    else if ( (0 <= x-getOriDis().x() && x-getOriDis().x()< image->width()) &&
              (-d->Border< y-getOriDis().y() && y-getOriDis().y() <= 0)
            )
    {
        forgcolor = d->preview->getPixelColor((x-getOriDis().x())/ratio,0);
    }
    else if ( (0 <= x-getOriDis().x() && x-getOriDis().x()< image->width()) &&
              (image->height()-1< y-getOriDis().y() &&  y-getOriDis().y()< (image->height()+d->Border))
            )
    {
        forgcolor = d->preview->getPixelColor((x-getOriDis().x())/ratio,d->preview->height()-1);
    }

    bacgcolor = d->preview->getPixelColor(x/ratio,y/ratio);
    newcolor.setRed(forgcolor.red()*alpha + bacgcolor.red()*(1-alpha)) ;
    newcolor.setGreen(forgcolor.green()*alpha + bacgcolor.green()*(1-alpha));
    newcolor.setBlue(forgcolor.blue()*alpha + bacgcolor.blue()*(1-alpha));
    newcolor.setAlpha(forgcolor.alpha());

    QPoint pre;
    pre.setX(x*width()/d->iface->originalWidth());
    pre.setY(y*height()/d->iface->originalHeight());

    d->preview->setPixelColor(pre.x(),pre.y(),newcolor);
    d->preveiwMask->setPixelColor(pre.x(),pre.y(),DColor(d->MASK_C));

    // origImage.setPixelColor(x,y,newcolor);
    d->maskImage->setPixelColor(x,y,DColor(d->MASK_C));//08/02
}

void ImageCloneWidget::addToMask(const QPoint& point)
{
    QPoint p;
    float ratio = d->iface->originalWidth()/d->preview->width();
    p.setX(point.x()*d->iface->originalWidth()/d->preview->width()) ;
    p.setY(point.y()*d->iface->originalHeight()/d->preview->height()) ;
    int mainDia = d->settings.mainDia*ratio;

    int mapDisx = p.x() - mainDia/2;
    int mapDisy = p.y() - mainDia/2;

    float alpha = d->settings.opacity/100; // alpha between 0 and 1

    QPixmap brushmap = d->settings.brush.getPixmap().scaled(QSize(d->settings.mainDia, d->settings.mainDia), Qt::KeepAspectRatio);
    DImg* brushimg = new DImg(brushmap.toImage());

    for(int j = p.y()-mainDia/2; j < p.y()+mainDia/2 ; j++)
    {
        for(int i = p.x() - mainDia/2; i < p.x() + mainDia/2 ; i++)
        {
            if(inBrushpixmap(&brushmap,i,j))
            {
                if(inimage(d->maskImage,i,j) && d->maskImage->getPixelColor(i,j).getQColor() == d->MASK_BG)
                {
                    DColor color = brushimg->getPixelColor(i-mapDisx,j-mapDisy);

                    if(color.alpha()!=0)
                    //if(color.getQColor()!=bac)
                    {
                        // color.setAlpha(255);

                        if(inimage(d->maskImage, i-getOriDis().x(), j-getOriDis().y()))
                        {
                            d->maskImage->setPixelColor(i,j,DColor(d->MASK_C));
                            QPoint pre;
                            pre.setX(i*width()/d->iface->originalWidth());
                            pre.setY(j*height()/d->iface->originalHeight());
                            d->preveiwMask->setPixelColor(pre.x(),pre.y(),DColor(d->MASK_C));

                            DColor forgcolor = d->preview->getPixelColor(i/ratio-d->dis.x(),j/ratio-d->dis.y());
                            DColor bacgcolor = d->iface->getOriginalImg()->getPixelColor(i/ratio, j/ratio);
                            DColor newcolor;
                            newcolor.setRed(forgcolor.red()*alpha + bacgcolor.red()*(1-alpha)) ;
                            newcolor.setGreen(forgcolor.green()*alpha + bacgcolor.green()*(1-alpha));
                            newcolor.setBlue(forgcolor.blue()*alpha + bacgcolor.blue()*(1-alpha));
                            newcolor.setAlpha(forgcolor.alpha());

                            d->preview->setPixelColor(pre.x(),pre.y(),newcolor);
                        //  origImage.setPixelColor(i,j,newcolor);
                        }
                        else
                        {
                            TreateAsBordor(d->iface->getOriginalImg(),i,j);
                        }
                    }
                }
            }
        }
    }
}

void ImageCloneWidget::mousePressEvent(QMouseEvent* e)
{
    if (e->button() == Qt::LeftButton)
    {
        if (d->rect.contains(e->x(), e->y()))
        {
            d->spot.setX(e->x()-d->rect.x());
            d->spot.setY(e->y()-d->rect.y());
            //QPoint p = getSpotPosition();
            if(inimage(d->preview, d->spot.x(), d->spot.y()))
            {
                if(d->settings.drawMode)
                {
                    d->centerPoint = d->spot;
                    upDis();
                    d->maskImage->fill(DColor(d->MASK_BG));//fill maskImage with white

                }
                else if(d->settings.selectMode)
                {
                    QPainter p(d->pixmap);//or d->preview
                    p.setRenderHint(QPainter::Antialiasing, true);
                    p.setPen(QColor(255,0,0));
                    p.drawEllipse(d->spot, 3, 3);

                    d->startPoint = d->spot;
                }
            }
        }
    }
}

void ImageCloneWidget::mouseReleaseEvent(QMouseEvent* e)
{
    if (d->rect.contains(e->x(), e->y()))
    {
        if(d->settings.selectMode)
            emit signalDrawingComplete();
    }
}

void ImageCloneWidget::mouseMoveEvent(QMouseEvent* e)
{
    if (e->button() == Qt::LeftButton)
    {
        if (d->rect.contains(e->x(), e->y()))
        {
            d->spot.setX(e->x()-d->rect.x());
            d->spot.setY(e->y()-d->rect.y());
            //QPoint p = getSpotPosition();
            if(inimage(d->preview, d->spot.x(), d->spot.y()))
                addToMask(d->spot);
        }
    }
}

//---------------Same as ImageGuideWidget-----------
void ImageCloneWidget::resizeEvent(QResizeEvent *e)
{
    blockSignals(true);
    delete d->pixmap;
    delete d->preview;

    int w     = e->size().width();
    int h     = e->size().height();
    //old_w = d->width;
    //old_h = d->height;

    uchar* data     = d->iface->setPreviewImageSize(w, h);
    d->width        = d->iface->previewWidth();
    d->height       = d->iface->previewHeight();
    bool sixteenBit = d->iface->previewSixteenBit();
    bool hasAlpha   = d->iface->previewHasAlpha();
    d->preview      = new DImg(d->width, d->height, sixteenBit, hasAlpha, data);
    d->preview->setIccProfile( d->iface->getOriginalImg()->getIccProfile() );
    delete [] data;

    d->pixmap         = new QPixmap(w, h);
    d->rect           = QRect(w/2-d->width/2, h/2-d->height/2, d->width, d->height);

    updatePixmap();
    blockSignals(false);
    emit signalResized();
}

void ImageCloneWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.drawPixmap(0, 0, *d->pixmap);
    p.end();
}

void ImageCloneWidget::updatePreview()
{
    updatePixmap();
    update();
}

void ImageCloneWidget::updatePixmap()
{
    QPainter p(d->pixmap);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setBackgroundMode(Qt::TransparentMode);
    d->pixmap->fill(d->bgColor);
    d->iface->paint(d->pixmap, d->rect.x(), d->rect.y(), d->rect.width(), d->rect.height(), &p);
}

} // namespace Digikam
