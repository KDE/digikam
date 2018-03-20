/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-02-14
 * Description : image data interface for image tools
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2004-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "imageiface.h"

// Qt includes

#include <QWidget>
#include <QSize>
#include <QPixmap>
#include <QBitmap>
#include <QPainter>

// Local includes

#include "digikam_debug.h"
#include "exposurecontainer.h"
#include "iccmanager.h"
#include "iccsettingscontainer.h"
#include "icctransform.h"
#include "editorcore.h"
#include "dmetadata.h"

namespace Digikam
{

class ImageIface::Private
{
public:

    Private() :
        previewType(FullImage),
        originalWidth(0),
        originalHeight(0),
        originalBytesDepth(0),
        constrainWidth(0),
        constrainHeight(0),
        previewWidth(0),
        previewHeight(0),
        core(EditorCore::defaultInstance())
    {
    }

    QPixmap checkPixmap();

    uchar* previewImageData();

public:

    ImageIface::PreviewType previewType;

    int                     originalWidth;
    int                     originalHeight;
    int                     originalBytesDepth;

    int                     constrainWidth;
    int                     constrainHeight;

    int                     previewWidth;
    int                     previewHeight;

    QPixmap                 qcheck;

    DImg                    previewImage;
    DImg                    targetPreviewImage;
    EditorCore* const       core;
};

QPixmap ImageIface::Private::checkPixmap()
{
    if (qcheck.isNull())
    {
        qcheck = QPixmap(8, 8);

        QPainter p;
        p.begin(&qcheck);
        p.fillRect(0, 0, 4, 4, QColor(144, 144, 144));
        p.fillRect(4, 4, 4, 4, QColor(144, 144, 144));
        p.fillRect(0, 4, 4, 4, QColor(100, 100, 100));
        p.fillRect(4, 0, 4, 4, QColor(100, 100, 100));
        p.end();
    }

    return qcheck;
}

uchar* ImageIface::Private::previewImageData()
{
    if (previewImage.isNull())
    {
        DImg* im = 0;

        if (previewType == FullImage)
        {
            im = core->getImg();

            if (!im || im->isNull())
            {
                return 0;
            }
        }
        else  // ImageSelection
        {
            im = new DImg(core->getImgSelection());

            if (!im)
            {
                return 0;
            }

            if (im->isNull())
            {
                delete im;
                return 0;
            }

            im->setIccProfile(core->getEmbeddedICC());
        }

        QSize sz(im->width(), im->height());
        sz.scale(constrainWidth, constrainHeight, Qt::KeepAspectRatio);

        previewImage       = im->smoothScale(sz.width(), sz.height());
        previewWidth       = previewImage.width();
        previewHeight      = previewImage.height();

        // only create another copy if needed, in setPreviewImage
        targetPreviewImage = previewImage;

        if (previewType == ImageSelection)
        {
            delete im;
        }
    }

    DImg previewData = previewImage.copyImageData();
    return previewData.stripImageData();
}

// ------------------------------------------------------------------------------------------------------

ImageIface::ImageIface(const QSize& size)
    : d(new Private)
{
    d->constrainWidth     = size.width();
    d->constrainHeight    = size.height();
    d->originalWidth      = d->core->origWidth();
    d->originalHeight     = d->core->origHeight();
    d->originalBytesDepth = d->core->bytesDepth();
}

ImageIface::~ImageIface()
{
    delete d;
}

void ImageIface::setPreviewType(PreviewType type)
{
    d->previewType = type;
}

ImageIface::PreviewType ImageIface::previewType() const
{
    return d->previewType;
}

DImg *ImageIface::previewReference()
{
    return &(d->previewImage);
}

DColor ImageIface::colorInfoFromOriginal(const QPoint& point) const
{
    if (!original() || original()->isNull())
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "No image data available!";
        return DColor();
    }

    if (point.x() > originalSize().width() || point.y() > originalSize().height())
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "Coordinate out of range!";
        return DColor();
    }

    return original()->getPixelColor(point.x(), point.y());
}

DColor ImageIface::colorInfoFromPreview(const QPoint& point) const
{
    if (d->previewImage.isNull() || point.x() > d->previewWidth || point.y() > d->previewHeight)
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "Coordinate out of range or no image data available!";
        return DColor();
    }

    return d->previewImage.getPixelColor(point.x(), point.y());
}

DColor ImageIface::colorInfoFromTargetPreview(const QPoint& point) const
{
    if (d->targetPreviewImage.isNull() || point.x() > d->previewWidth || point.y() > d->previewHeight)
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "Coordinate out of range or no image data available!";
        return DColor();
    }

    return d->targetPreviewImage.getPixelColor(point.x(), point.y());
}

DImg ImageIface::setPreviewSize(const QSize& size) const
{
    d->previewImage.reset();
    d->targetPreviewImage.reset();

    d->constrainWidth  = size.width();
    d->constrainHeight = size.height();

    return preview();
}

DImg ImageIface::preview() const
{
    // NOTE: corrects the values for width and height of the preview image from the image data interface
    // See Bug #320382 for details.
    uchar* const data = d->previewImageData();
    return DImg(d->previewWidth, d->previewHeight, previewSixteenBit(), previewHasAlpha(), data);
}

DImg* ImageIface::original() const
{
    return d->core->getImg();
}

DImg ImageIface::selection() const
{
    return d->core->getImgSelection();
}

void ImageIface::setPreviewIccProfile(const IccProfile& profile)
{
    d->targetPreviewImage.detach();
    d->targetPreviewImage.setIccProfile(profile);
}

void ImageIface::setOriginalIccProfile(const IccProfile& profile)
{
    d->core->putIccProfile(profile);
}

QSize ImageIface::previewSize() const
{
    return QSize(d->previewWidth, d->previewHeight);
}

bool ImageIface::previewSixteenBit() const
{
    return originalSixteenBit();
}

bool ImageIface::previewHasAlpha() const
{
    return originalHasAlpha();
}

QSize ImageIface::originalSize() const
{
    return QSize(d->core->origWidth(), d->core->origHeight());
}

bool ImageIface::originalSixteenBit() const
{
    return d->core->sixteenBit();
}

bool ImageIface::originalHasAlpha() const
{
    return d->core->hasAlpha();
}

QRect ImageIface::selectionRect() const
{
    return (d->core->getSelectedArea());
}

void ImageIface::convertOriginalColorDepth(int depth)
{
    d->core->convertDepth(depth);
}

QPixmap ImageIface::convertToPixmap(DImg& img) const
{
    return d->core->convertToPixmap(img);
}

IccProfile ImageIface::originalIccProfile() const
{
    return d->core->getEmbeddedICC();
}

MetaEngineData ImageIface::originalMetadata() const
{
    DImg* const img = original();

    if (img)
        return (img->getMetadata());

    return MetaEngineData();
}

void ImageIface::setOriginalMetadata(const MetaEngineData& meta)
{
    DImg* const img = original();

    if (img)
        img->setMetadata(meta);
}

PhotoInfoContainer ImageIface::originalPhotoInfo() const
{
    return (DMetadata(originalMetadata()).getPhotographInformation());
}

void ImageIface::paint(QPaintDevice* const device, const QRect& rect, QPainter* const painter)
{
    int       x = rect.x();
    int       y = rect.y();
    int       w = rect.width();
    int       h = rect.height();
    QPainter* p = 0;
    QPainter  localPainter;

    if (painter)
    {
        p = painter;
    }
    else
    {
        p = &localPainter;
        p->begin(device);
    }

    int width  = w > 0 ? qMin(d->previewWidth,  w) : d->previewWidth;
    int height = h > 0 ? qMin(d->previewHeight, h) : d->previewHeight;

    if (!d->targetPreviewImage.isNull())
    {
        if (d->targetPreviewImage.hasAlpha())
        {
            p->drawTiledPixmap(x, y, width, height, d->checkPixmap());
        }

        QPixmap              pixImage;
        bool                 doSoftProofing = d->core->softProofingEnabled();
        ICCSettingsContainer iccSettings    = d->core->getICCSettings();

        if (iccSettings.enableCM && (iccSettings.useManagedView || doSoftProofing))
        {
            IccManager   manager(d->targetPreviewImage);
            IccTransform monitorICCtrans;

            if (doSoftProofing)
            {
                monitorICCtrans = manager.displaySoftProofingTransform(iccSettings.defaultProofProfile);
            }
            else
            {
                monitorICCtrans = manager.displayTransform();
            }

            pixImage = d->targetPreviewImage.convertToPixmap(monitorICCtrans);
        }
        else
        {
            pixImage = d->targetPreviewImage.convertToPixmap();
        }

        p->drawPixmap(x, y, pixImage, 0, 0, width, height);

        // Show the Over/Under exposure pixels indicators

        ExposureSettingsContainer* const expoSettings = d->core->getExposureSettings();

        if (expoSettings && (expoSettings->underExposureIndicator || expoSettings->overExposureIndicator))
        {
            QImage pureColorMask = d->targetPreviewImage.pureColorMask(expoSettings);
            QPixmap pixMask      = QPixmap::fromImage(pureColorMask);
            p->drawPixmap(x, y, pixMask, 0, 0, width, height);
        }
    }

    if (!painter)
    {
        p->end();
    }
}

void ImageIface::setSelection(const QString& caller, const FilterAction& action, const DImg& img)
{
    if (img.hasAlpha()   != originalHasAlpha()     ||
        img.sixteenBit() != originalSixteenBit()   ||
        img.size()       != selectionRect().size()
       )
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Properties of image to overwrite selection differs than original image";
        return;
    }

    if (img.isNull())
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "No image data to handle";
        return;
    }

    d->core->putImgSelection(caller, action, img);
}

void ImageIface::setPreview(const DImg& img)
{
    if (img.hasAlpha()   != previewHasAlpha()   ||
        img.sixteenBit() != previewSixteenBit()
       )
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Properties of image differs than preview";
        return;
    }

    uchar* const data = img.bits();

    if (!data)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "No preview image data to handle";
        return;
    }

    d->targetPreviewImage.detach();
    d->targetPreviewImage.putImageData(data);
}

void ImageIface::setOriginal(const QString& caller, const FilterAction& action, const DImg& img)
{
    if (img.isNull())
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "No image data to handle";
        return;
    }

    d->core->putImg(caller, action, img);
}

}   // namespace Digikam
