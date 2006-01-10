/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *         Gilles Caulier <caulier dot gilles at free.fr> 
 * Date  : 2004-02-14
 * Description : image data interface for image plugins
 *
 * Copyright 2004-2005 by Renchi Raju, Gilles Caulier
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

#include <qwidget.h>
#include <qsize.h>
#include <qpixmap.h>
#include <qbitmap.h>
#include <qpainter.h>

// KDE includes.

#include <kdebug.h>

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

    int           originalWidth;
    int           originalHeight;
    int           originalBytesDepth;

    int           constrainWidth;
    int           constrainHeight;

    uchar*        previewData;
    int           previewWidth;
    int           previewHeight;
    
    QPixmap       qcheck;
    QPixmap       qpix;
    QBitmap       qmask;
};

ImageIface::ImageIface(int w, int h)
{
    d = new ImageIfacePriv;

    d->constrainWidth  = w;
    d->constrainHeight = h;
    d->previewWidth    = 0;
    d->previewHeight   = 0;
    d->previewData     = 0;
    
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
    if (d->previewData)
        delete [] d->previewData;
    
    delete d;
}

DColor ImageIface::getColorInfoFromOriginalImage(QPoint point)
{
    if ( !DImgInterface::instance()->getImage() || point.x() > originalWidth() || point.y() > originalHeight() )
    {
        kdWarning() << k_funcinfo << "Coordinate out of range or no image data available!" << endl;
        return DColor::DColor();
    }
    
    int bytesDepth = originalSixteenBit() ? 8 : 4;
    uchar *currentPointData = DImgInterface::instance()->getImage() + point.x()*bytesDepth +
                              (originalWidth() * point.y() * bytesDepth);
    return DColor::DColor(currentPointData, originalSixteenBit());
}

DColor ImageIface::getColorInfoFromPreviewImage(QPoint point)
{
    if ( !d->previewData || point.x() > previewWidth() || point.y() > previewHeight() )
    {
        kdWarning() << k_funcinfo << "Coordinate out of range or no image data available!" << endl;
        return DColor::DColor();
    }
        
    int bytesDepth = previewSixteenBit() ? 8 : 4;
    uchar *currentPointData = d->previewData + point.x()*bytesDepth +
                              (previewWidth() * point.y() * bytesDepth);
    return DColor::DColor(currentPointData, previewSixteenBit());
}

DColor ImageIface::getColorInfoFromTargetPreviewImage(QPoint point)
{
    if ( d->image.isNull() || point.x() > d->image.width() || point.y() > d->image.height() )
    {
        kdWarning() << k_funcinfo << "Coordinate out of range or no image data available!" << endl;
        return DColor::DColor();
    }
        
    uchar *currentPointData = d->image.bits() + point.x()*d->image.bytesDepth() +
                              (d->image.width() * point.y() * d->image.bytesDepth());
    return DColor::DColor(currentPointData, d->image.sixteenBit());
}

uchar* ImageIface::setPreviewImageSize(int w, int h)
{
    if (d->previewData)
        delete [] d->previewData;

    d->previewData     = 0;
    d->constrainWidth  = w;
    d->constrainHeight = h;

    return (getPreviewImage());
}

uchar* ImageIface::getPreviewImage()
{
    if (!d->previewData)
    {
        uchar* ptr      = DImgInterface::instance()->getImage();
        int w           = DImgInterface::instance()->origWidth();
        int h           = DImgInterface::instance()->origHeight();
        bool hasAlpha   = DImgInterface::instance()->hasAlpha();
        bool sixteenBit = DImgInterface::instance()->sixteenBit();

        if (!ptr || !w || !h)
            return 0;

        DImg im(w, h, sixteenBit, hasAlpha, ptr);
        QSize sz(im.width(), im.height());
        sz.scale(d->constrainWidth, d->constrainHeight, QSize::ScaleMin);

        d->image         = im.smoothScale(sz.width(), sz.height());
        d->previewWidth  = d->image.width();
        d->previewHeight = d->image.height();
        
        d->previewData = new uchar[d->image.numBytes()];
        memcpy(d->previewData, d->image.bits(), d->image.numBytes());
        
        d->qmask.resize(d->previewWidth, d->previewHeight);
        d->qpix.resize(d->previewWidth, d->previewHeight);
    }

    int size = d->previewWidth*d->previewHeight*d->image.bytesDepth();
    uchar* data = new uchar[size];
    memcpy(data, d->previewData, size);
    
    return data;
}

uchar* ImageIface::getOriginalImage()
{
    uchar *ptr = DImgInterface::instance()->getImage();
    int    w   = DImgInterface::instance()->origWidth();
    int    h   = DImgInterface::instance()->origHeight();
    int    bd  = DImgInterface::instance()->bytesDepth();

    if (!ptr || !w || !h) 
        return 0;
        
    uchar *origData = new uchar[w * h * bd];
    memcpy(origData, ptr, w * h * bd);

    return origData;
}

uchar* ImageIface::getImageSelection()
{
    return DImgInterface::instance()->getImageSelection();
}

void ImageIface::putPreviewImage(uchar* data)
{
    if (!data)
        return;

    uchar* origData = d->image.bits();
    int    w        = d->image.width();
    int    h        = d->image.height();
    int    bd       = d->image.bytesDepth();

    memcpy(origData, data, w * h * bd);
}

void ImageIface::putOriginalImage(const QString &caller, uchar* data, int w, int h)
{
    if (!data)
        return;

    DImgInterface::instance()->putImage(caller, data, w, h);
}

void ImageIface::putImageSelection(uchar* data)
{
    if (!data)
        return;
    
    DImgInterface::instance()->putImageSelection(data);
}

int ImageIface::previewWidth()
{
    return d->previewWidth;
}

int ImageIface::previewHeight()
{
    return d->previewHeight;
}

bool ImageIface::previewSixteenBit()
{
    return originalSixteenBit();
}

bool ImageIface::previewHasAlpha()
{
    return originalHasAlpha();
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

bool ImageIface::originalHasAlpha()
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
    DImg preview(previewWidth(), previewHeight(), previewSixteenBit(), previewHasAlpha(), getPreviewImage());
    BCGModifier cmod;
    cmod.setOverIndicator(overIndicator);
    cmod.setGamma(gamma);
    cmod.setBrightness(brightness);
    cmod.setContrast(contrast);
    cmod.applyBCG(preview);
    putPreviewImage(preview.bits());
}

void ImageIface::setOriginalBCG(double brightness, double contrast, double gamma)
{
    DImgInterface::instance()->setBCG(brightness, contrast, gamma);    
}

void ImageIface::convertOriginalColorDepth(int depth)
{
    DImgInterface::instance()->convertDepth(depth);
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
    if (!d->previewData)
    {
        uchar* ptr      = DImgInterface::instance()->getImage();
        int w           = DImgInterface::instance()->origWidth();
        int h           = DImgInterface::instance()->origHeight();
        bool hasAlpha   = DImgInterface::instance()->hasAlpha();
        bool sixteenBit = DImgInterface::instance()->sixteenBit();

        if (!ptr || !w || !h)
            return 0;

        DImg im(w, h, sixteenBit, hasAlpha, ptr);
        QSize sz(im.width(), im.height());
        sz.scale(d->constrainWidth, d->constrainHeight, QSize::ScaleMin);

        d->image         = im.smoothScale(sz.width(), sz.height());
        d->previewWidth  = d->image.width();
        d->previewHeight = d->image.height();
        
        d->previewData = new uchar[d->image.numBytes()];
        memcpy(d->previewData, d->image.bits(), d->image.numBytes());
        
        d->qmask.resize(d->previewWidth, d->previewHeight);
        d->qpix.resize(d->previewWidth, d->previewHeight);
    }
    
    int size = d->previewWidth*d->previewHeight*d->image.bytesDepth();
    uchar* data = new uchar[size];
    memcpy(data, d->previewData, size);
    
    return (uint*)data;
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
    if (d->previewData)
        delete [] d->previewData;

    d->previewData     = 0;
    d->constrainWidth  = w;
    d->constrainHeight = h;
        
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

