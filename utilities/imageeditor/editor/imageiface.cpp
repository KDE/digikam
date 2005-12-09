/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *         Gilles Caulier <caulier dot gilles at free.fr> 
 * Date  : 2004-02-14
 * Description :
 *
 * Copyright 2004-2005 by Renchi Raju, Gilles Caulier
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

#include <qwidget.h>
#include <qsize.h>
#include <qpixmap.h>
#include <qbitmap.h>
#include <qpainter.h>

// Local includes.

#include "dimginterface.h"
#include "bcgmodifier.h"
#include "imageiface.h"

namespace Digikam
{

class ImageIfacePriv
{
public:

    DImg          image;
    DImg          previewImage;

    int           constrainWidth;
    int           constrainHeight;

    int           originalWidth;
    int           originalHeight;
    int           originalBytesDepth;

    QPixmap       qcheck;
    QPixmap       qpix;
    QBitmap       qmask;
};

ImageIface::ImageIface(int w, int h)
{
    d = new ImageIfacePriv;

    d->constrainWidth     = w;
    d->constrainHeight    = h;

    d->originalWidth      = DImgInterface::instance()->origWidth();
    d->originalHeight     = DImgInterface::instance()->origHeight();
    d->originalBytesDepth = DImgInterface::instance()->bytesDepth();
    
    d->qpix.setMask(d->qmask);
    d->qcheck.resize(8, 8);

    QPainter p;
    p.begin(&d->qcheck);
    p.fillRect(0, 0, 4, 4, QColor(144,144,144));
    p.fillRect(4, 4, 4, 4, QColor(144,144,144));
    p.fillRect(0, 4, 4, 4, QColor(100,100,100));
    p.fillRect(4, 0, 4, 4, QColor(100,100,100));
    p.end();
}

ImageIface::~ImageIface()
{
    delete d;
}

DImg ImageIface::setPreviewImageSize(int w, int h)
{
    if (!d->previewImage.isNull())
        d->previewImage.reset();

    d->constrainWidth  = w;
    d->constrainHeight = h;

    return (getPreviewImage());
}

DImg ImageIface::getPreviewImage()
{
    if (d->previewImage.isNull()) 
    {
        DImg im = DImgInterface::instance()->getImage();

        if ( im.isNull() ) 
            return DImg::DImg();

        QSize sz(im.width(), im.height());
        sz.scale(d->constrainWidth, d->constrainHeight, QSize::ScaleMin);

        d->image = im.smoothScale(sz.width(), sz.height());
        d->previewImage = d->image;
        
        d->qmask.resize(d->previewImage.width(), d->previewImage.height());
        d->qpix.resize(d->previewImage.width(), d->previewImage.height());
    }

    return (d->previewImage);
}

DImg ImageIface::getOriginalImage()
{
    return DImgInterface::instance()->getImage();
}

DImg ImageIface::getImageSelection()
{
    return DImgInterface::instance()->getImageSelection();
}

void ImageIface::putPreviewImage(DImg& image)
{
    if (image.isNull())
        return;

    d->image = image.copy();
}

void ImageIface::putOriginalImage(const QString &caller, DImg& image)
{
    if (image.isNull())
        return;

    DImgInterface::instance()->putImage(caller, image);
}

void ImageIface::putImageSelection(DImg& selection)
{
    if (selection.isNull())
        return;

    DImgInterface::instance()->putImageSelection(selection);
}

int ImageIface::previewWidth()
{
    return d->previewImage.width();
}

int ImageIface::previewHeight()
{
    return d->previewImage.height();
}

int ImageIface::originalWidth()
{
    return DImgInterface::instance()->origWidth();
}

int ImageIface::originalHeight()
{
    return DImgInterface::instance()->origHeight();
}

bool ImageIface::originalSixteenBit()
{
    return DImgInterface::instance()->sixteenBit();
}

bool ImageIface::originalImageHasAlpha()
{
    return DImgInterface::instance()->hasAlpha();
}

int ImageIface::selectedWidth()
{
    int x, y, w, h;
    DImgInterface::instance()->getSelectedArea(x, y, w, h);
    return w;
}

int ImageIface::selectedHeight()
{
    int x, y, w, h;
    DImgInterface::instance()->getSelectedArea(x, y, w, h);
    return h;
}

int ImageIface::selectedXOrg()
{
    int x, y, w, h;
    DImgInterface::instance()->getSelectedArea(x, y, w, h);
    return x;
}

int ImageIface::selectedYOrg()
{
    int x, y, w, h;
    DImgInterface::instance()->getSelectedArea(x, y, w, h);
    return y;
}

void ImageIface::setPreviewBCG(double brightness, double contrast, double gamma, bool overIndicator)
{
    DImg preview = getPreviewImage(); 
    BCGModifier cmod;
    cmod.setOverIndicator(overIndicator);
    cmod.setGamma(gamma);
    cmod.setBrightness(brightness);
    cmod.setContrast(contrast);
    cmod.applyBCG(preview);
    putPreviewImage(preview);
}

void ImageIface::setOriginalBCG(double brightness, double contrast, double gamma)
{
    DImgInterface::instance()->setBCG(brightness, contrast, gamma);    
}

void ImageIface::paint(QPaintDevice* device, int x, int y, int w, int h)
{
    if ( !d->image.isNull() ) 
    {
        if (d->image.hasAlpha()) 
        {
            QPainter p(&d->qpix);
            p.drawTiledPixmap(0, 0, d->qpix.width(), d->qpix.height(), d->qcheck);
            p.end();
        }

        QPixmap pixImage = d->image.convertToPixmap();
        bitBlt ( &d->qpix, 0, 0, &pixImage, 0, 0, w, h, Qt::CopyROP, false );

    }

    bitBlt(device, x, y, &d->qpix, 0, 0, -1, -1, Qt::CopyROP, false);
}

// -----------------------------------------------------------------------------------
// FIXME Remove methods below when all image plugins will be ported to DImg

uint* ImageIface::getPreviewData()           
{
    if (d->previewImage.isNull()) 
    {
        DImg im = DImgInterface::instance()->getImage();

        if ( im.isNull() ) 
            return 0;

        QSize sz(im.width(), im.height());
        sz.scale(d->constrainWidth, d->constrainHeight, QSize::ScaleMin);

        d->image = im.smoothScale(sz.width(), sz.height());
        d->previewImage = d->image;

        d->qmask.resize(d->previewImage.width(), d->previewImage.height());
        d->qpix.resize(d->previewImage.width(), d->previewImage.height());
    }

    return (uint*)d->previewImage.bits();
}

void ImageIface::putPreviewData(uint* data)  
{
    if (!data)
        return;

    uchar* origData = d->image.bits();
    int    w        = d->image.width();
    int    h        = d->image.height();
    int    bd       = d->image.bytesDepth();

    memcpy(origData, data, w * h * bd);
}

uint* ImageIface::getOriginalData()
{
    uint *ptr = DImgInterface::instance()->getData();
    int   w   = DImgInterface::instance()->origWidth();
    int   h   = DImgInterface::instance()->origHeight();
    int   bd  = DImgInterface::instance()->bytesDepth();

    if (!ptr || !w || !h) 
        return 0;
        
    uchar *origData = new uchar[w * h * bd];
    memcpy(origData, ptr, w * h * bd);

    return (uint*)origData;
}

uint* ImageIface::setPreviewSize(int w, int h)
{
    if (!d->previewImage.isNull())
        d->previewImage.reset();

    d->previewImage = DImg(w, h, d->image.sixteenBit(), d->image.hasAlpha());
    
    return (getPreviewData());
}

void ImageIface::putOriginalData(const QString &caller, uint* data, int w, int h)
{
    if (!data)
        return;

    DImgInterface::instance()->putData(caller, data, w, h);
}

void ImageIface::putSelectedData(uint* data)
{
    if (!data)
        return;

    DImgInterface::instance()->putSelectedData(data);
}

uint* ImageIface::getSelectedData()
{
    return DImgInterface::instance()->getSelectedData();    
}

}   // namespace Digikam

