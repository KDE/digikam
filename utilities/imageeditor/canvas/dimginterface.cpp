/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-01-15
 * Description : DImg interface for image editor
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
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

#define OPACITY  0.7
#define RCOL     0xAA
#define GCOL     0xAA
#define BCOL     0xAA


#include "dimginterface.h"
#include "dimginterface.moc"

// C++ includes

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>

// Qt includes

#include <QWidget>
#include <QImage>
#include <QPixmap>
#include <QBitmap>
#include <QColor>
#include <QFile>
#include <QVariant>
#include <QImageReader>
#include <QPainter>

// KDE includes

#include <kcursor.h>
#include <kmessagebox.h>
#include <kdebug.h>

// Local includes

#include "bcgmodifier.h"
#include "colorcorrectiondlg.h"
#include "undomanager.h"
#include "undoaction.h"
#include "iccmanager.h"
#include "iccsettingscontainer.h"
#include "icctransform.h"
#include "exposurecontainer.h"
#include "iofilesettingscontainer.h"
#include "sharedloadsavethread.h"
#include "dmetadata.h"
#include "rawimport.h"
#include "editortooliface.h"

namespace Digikam
{

class UndoManager;

class DImgInterfacePrivate
{

public:

    DImgInterfacePrivate()
    {
        undoMan          = 0;
        cmSettings       = 0;
        expoSettings     = 0;
        thread           = 0;
        width            = 0;
        height           = 0;
        origWidth        = 0;
        origHeight       = 0;
        selX             = 0;
        selY             = 0;
        selW             = 0;
        selH             = 0;
        zoom             = 1.0;
        displayingWidget = 0;
        exifOrient       = false;
        valid            = false;
        rotatedOrFlipped = false;
        doSoftProofing   = false;
    }

    bool                       valid;
    bool                       rotatedOrFlipped;
    bool                       exifOrient;
    bool                       doSoftProofing;

    int                        width;
    int                        height;
    int                        origWidth;
    int                        origHeight;
    int                        selX;
    int                        selY;
    int                        selW;
    int                        selH;

    float                      gamma;
    float                      brightness;
    float                      contrast;

    double                     zoom;

    QWidget*                   displayingWidget;

    QString                    filename;
    QString                    savingFilename;

    DImg                       image;

    UndoManager*               undoMan;

    ICCSettingsContainer*      cmSettings;

    ExposureSettingsContainer* expoSettings;

    SharedLoadSaveThread*      thread;
    LoadingDescription         currentDescription;
    LoadingDescription         nextRawDescription;

    IccTransform               monitorICCtrans;
};

DImgInterface* DImgInterface::m_defaultInterface = 0;

DImgInterface* DImgInterface::defaultInterface()
{
    return m_defaultInterface;
}

void DImgInterface::setDefaultInterface(DImgInterface *defaultInterface)
{
    m_defaultInterface = defaultInterface;
}

DImgInterface::DImgInterface()
             : QObject(), d(new DImgInterfacePrivate)
{
    d->undoMan = new UndoManager(this);
    d->thread  = new SharedLoadSaveThread;

    connect( d->thread, SIGNAL(signalImageLoaded(const LoadingDescription &, const DImg&)),
             this, SLOT(slotImageLoaded(const LoadingDescription &, const DImg&)) );

    connect( d->thread, SIGNAL(signalImageSaved(const QString&, bool)),
             this, SLOT(slotImageSaved(const QString&, bool)) );

    connect( d->thread, SIGNAL(signalLoadingProgress(const LoadingDescription &, float)),
             this, SLOT(slotLoadingProgress(const LoadingDescription &, float)) );

    connect( d->thread, SIGNAL(signalSavingProgress(const QString&, float)),
             this, SLOT(slotSavingProgress(const QString&, float)) );
}

DImgInterface::~DImgInterface()
{
    delete d->undoMan;
    delete d->thread;
    delete d;
    if (m_defaultInterface == this)
        m_defaultInterface = 0;
}

void DImgInterface::setDisplayingWidget(QWidget *widget)
{
    d->displayingWidget = widget;
}

void DImgInterface::load(const QString& filename, IOFileSettingsContainer *iofileSettings)
{
    LoadingDescription description(filename, iofileSettings->rawDecodingSettings, LoadingDescription::ConvertForEditor);

    if (iofileSettings->useRAWImport && DImg::fileFormat(filename) == DImg::RAW)
    {
        d->nextRawDescription = description;

        RawImport *rawImport = new RawImport(KUrl(filename), this);
        EditorToolIface::editorToolIface()->loadTool(rawImport);

        connect(rawImport, SIGNAL(okClicked()),
                this, SLOT(slotLoadRawFromTool()));

        connect(rawImport, SIGNAL(cancelClicked()),
                this, SLOT(slotLoadRaw()));

        d->thread->stopLoading();
        return;
    }
    else
    {
        d->nextRawDescription = LoadingDescription();
    }

    load(description);
}

void DImgInterface::slotLoadRawFromTool()
{
    RawImport *rawImport = dynamic_cast<RawImport*>(EditorToolIface::editorToolIface()->currentTool());
    if (rawImport)
        d->nextRawDescription.rawDecodingSettings = rawImport->rawDecodingSettings();
    load(d->nextRawDescription);
    d->nextRawDescription = LoadingDescription();
}

void DImgInterface::slotLoadRaw()
{
    load(d->nextRawDescription);
    d->nextRawDescription = LoadingDescription();
}

void DImgInterface::load(const LoadingDescription& description)
{
    if (description != d->currentDescription)
    {
        resetValues();
        d->currentDescription = description;
        d->filename           = d->currentDescription.filePath;

        loadCurrent();
    }

    EditorToolIface::editorToolIface()->unLoadTool();
}

void DImgInterface::applyTransform(const IccTransform& transform)
{
    if (!d->valid)
        return;

    d->currentDescription.postProcessingParameters.colorManagement = LoadingDescription::ApplyTransform;
    d->currentDescription.postProcessingParameters.setTransform(transform);
    loadCurrent();

    EditorToolIface::editorToolIface()->unLoadTool();
}

void DImgInterface::loadCurrent()
{
    d->thread->load(d->currentDescription,
                    SharedLoadSaveThread::AccessModeReadWrite,
                    SharedLoadSaveThread::LoadingPolicyFirstRemovePrevious);
    emit signalLoadingStarted(d->filename);
}

void DImgInterface::restore()
{
    LoadingDescription description = d->currentDescription;
    resetValues();
    load(description);
}

void DImgInterface::resetImage()
{
    EditorToolIface::editorToolIface()->unLoadTool();
    resetValues();
    d->image.reset();
}

void DImgInterface::resetValues()
{
    d->valid          = false;
    d->filename.clear();
    d->currentDescription = LoadingDescription();
    d->width          = 0;
    d->height         = 0;
    d->origWidth      = 0;
    d->origHeight     = 0;
    d->selX           = 0;
    d->selY           = 0;
    d->selW           = 0;
    d->selH           = 0;
    d->gamma          = 1.0f;
    d->contrast       = 1.0f;
    d->brightness     = 0.0f;

    d->undoMan->clear();
}

void DImgInterface::setICCSettings(ICCSettingsContainer *cmSettings)
{
    d->cmSettings = cmSettings;
}

void DImgInterface::setExposureSettings(ExposureSettingsContainer *expoSettings)
{
    d->expoSettings = expoSettings;
}

void DImgInterface::slotImageLoaded(const LoadingDescription& loadingDescription, const DImg& img)
{
    const QString &fileName = loadingDescription.filePath;

    if (fileName != d->filename)
        return;

    // RAW tool active? Discard previous loaded image
    if (!d->nextRawDescription.filePath.isNull())
        return;

    bool valRet = false;
    d->image    = img;

    if (!d->image.isNull())
    {
        d->origWidth  = d->image.width();
        d->origHeight = d->image.height();
        d->valid      = true;
        d->width      = d->origWidth;
        d->height     = d->origHeight;
        valRet        = true;

        // Raw files are already rotated properly by dcraw. Only perform auto-rotation with JPEG/PNG/TIFF file.
        // We don't have a feedback from dcraw about auto-rotated RAW file during decoding. Well set transformed
        // flag as well.

        if (d->image.attribute("format").toString() == QString("RAW"))
            d->rotatedOrFlipped = true;

        if (d->exifOrient &&
            (d->image.attribute("format").toString() == QString("JPEG") ||
             d->image.attribute("format").toString() == QString("PNG")  ||
             d->image.attribute("format").toString() == QString("TIFF")))
             exifRotate(d->filename);

        updateColorManagement();
    }
    else
    {
        valRet = false;
    }

    emit signalImageLoaded(d->filename, valRet);
    setModified();
}

void DImgInterface::updateColorManagement()
{
    IccManager manager(d->image);
    if (d->doSoftProofing)
        d->monitorICCtrans = manager.displaySoftProofingTransform(d->cmSettings->defaultProofProfile, d->displayingWidget);
    else
        d->monitorICCtrans = manager.displayTransform(d->displayingWidget);
}

void DImgInterface::setSoftProofingEnabled(bool enabled)
{
    d->doSoftProofing = enabled;
    updateColorManagement();
}

void DImgInterface::slotLoadingProgress(const LoadingDescription& loadingDescription, float progress)
{
    if (loadingDescription.filePath == d->filename)
        emit signalLoadingProgress(loadingDescription.filePath, progress);
}

bool DImgInterface::exifRotated()
{
    return d->rotatedOrFlipped;
}

void DImgInterface::exifRotate(const QString& filename)
{
    // Rotate image based on EXIF rotate tag

    DMetadata metadata(filename);
    DMetadata::ImageOrientation orientation = metadata.getImageOrientation();

    if (orientation != DMetadata::ORIENTATION_NORMAL)
    {
        switch (orientation)
        {
            case DMetadata::ORIENTATION_NORMAL:
            case DMetadata::ORIENTATION_UNSPECIFIED:
                break;

            case DMetadata::ORIENTATION_HFLIP:
                flipHoriz(false);
                break;

            case DMetadata::ORIENTATION_ROT_180:
                rotate180(false);
                break;

            case DMetadata::ORIENTATION_VFLIP:
                flipVert(false);
                break;

            case DMetadata::ORIENTATION_ROT_90_HFLIP:
                rotate90(false);
                flipHoriz(false);
                break;

            case DMetadata::ORIENTATION_ROT_90:
                rotate90(false);
                break;

            case DMetadata::ORIENTATION_ROT_90_VFLIP:
                rotate90(false);
                flipVert(false);
                break;

            case DMetadata::ORIENTATION_ROT_270:
                rotate270(false);
                break;
        }

        d->rotatedOrFlipped = true;
    }
}

void DImgInterface::setExifOrient(bool exifOrient)
{
    d->exifOrient = exifOrient;
}

void DImgInterface::undo()
{
    if (!d->undoMan->anyMoreUndo())
    {
        emit signalUndoStateChanged(false, d->undoMan->anyMoreRedo(), !d->undoMan->isAtOrigin());
        return;
    }

    d->undoMan->undo();
    emit signalUndoStateChanged(d->undoMan->anyMoreUndo(), d->undoMan->anyMoreRedo(), !d->undoMan->isAtOrigin());
}

void DImgInterface::redo()
{
    if (!d->undoMan->anyMoreRedo())
    {
        emit signalUndoStateChanged(d->undoMan->anyMoreUndo(), false, !d->undoMan->isAtOrigin());
        return;
    }

    d->undoMan->redo();
    emit signalUndoStateChanged(d->undoMan->anyMoreUndo(), d->undoMan->anyMoreRedo(), !d->undoMan->isAtOrigin());
}

void DImgInterface::saveAs(const QString& fileName, IOFileSettingsContainer *iofileSettings,
                           bool setExifOrientationTag, const QString& givenMimeType)
{
    // No need to toggle off undo, redo or save action during saving using
    // signalUndoStateChanged(), this is will done by GUI implementation directly.

    // Try hard to find a mimetype.
    QString mimeType = givenMimeType;

    // This is possibly empty
    if (mimeType.isEmpty())
        mimeType = getImageFormat();

    kDebug() << "Saving to :" << QFile::encodeName(fileName).data() << " ("
                  << mimeType << ")";

    // JPEG file format.
    if ( mimeType.toUpper() == QString("JPG") || mimeType.toUpper() == QString("JPEG") ||
         mimeType.toUpper() == QString("JPE"))
    {
       d->image.setAttribute("quality",     iofileSettings->JPEGCompression);
       d->image.setAttribute("subsampling", iofileSettings->JPEGSubSampling);
    }

    // PNG file format.
    if ( mimeType.toUpper() == QString("PNG") )
       d->image.setAttribute("quality", iofileSettings->PNGCompression);

    // TIFF file format.
    if ( mimeType.toUpper() == QString("TIFF") || mimeType.toUpper() == QString("TIF") )
       d->image.setAttribute("compress", iofileSettings->TIFFCompression);

    // JPEG 2000 file format.
    if ( mimeType.toUpper() == QString("JP2") || mimeType.toUpper() == QString("JPX") ||
         mimeType.toUpper() == QString("JPC") || mimeType.toUpper() == QString("PGX") ||
         mimeType.toUpper() == QString("J2K"))
    {
        if (iofileSettings->JPEG2000LossLess)
            d->image.setAttribute("quality", 100);    // LossLess compression
        else
            d->image.setAttribute("quality", iofileSettings->JPEG2000Compression);
    }

    // PGF file format.
    if ( mimeType.toUpper() == QString("PGF"))
    {
        if (iofileSettings->PGFLossLess)
            d->image.setAttribute("quality", 0);    // LossLess compression
        else
            d->image.setAttribute("quality", iofileSettings->PGFCompression);
    }

    d->savingFilename = fileName;

    d->image.updateMetadata(mimeType, getImageFileName(), setExifOrientationTag);

    d->thread->save(d->image, fileName, mimeType);
}

void DImgInterface::slotImageSaved(const QString& filePath, bool success)
{
    if (filePath != d->savingFilename)
        return;

    if (!success)
        kWarning() << "error saving image '" << QFile::encodeName(filePath).data();

    emit signalImageSaved(filePath, success);
    emit signalUndoStateChanged(d->undoMan->anyMoreUndo(), d->undoMan->anyMoreRedo(), !d->undoMan->isAtOrigin());
}

void DImgInterface::slotSavingProgress(const QString& filePath, float progress)
{
    if (filePath == d->savingFilename)
        emit signalSavingProgress(filePath, progress);
}

void DImgInterface::abortSaving()
{
    // failure will be reported by a signal
    d->thread->stopSaving(d->savingFilename);
}

void DImgInterface::switchToLastSaved(const QString& newFilename)
{
    // Higher level wants to use the current DImg object to represent the file
    // it has previously been saved to.
    d->filename = newFilename;

    // Currently the only place where a DImg is connected to the file it originates from
    // is the format attribute.
    QVariant savedformat = d->image.attribute("savedformat");
    if (!savedformat.isNull())
        d->image.setAttribute("format", savedformat.toString());
    QVariant readonly = d->image.attribute("savedformat-isreadonly");
    if (!readonly.isNull())
        d->image.setAttribute("isreadonly", readonly.toBool());
}

void DImgInterface::setModified()
{
    emit signalModified();
    emit signalUndoStateChanged(d->undoMan->anyMoreUndo(), d->undoMan->anyMoreRedo(), !d->undoMan->isAtOrigin());
}

void DImgInterface::readMetadataFromFile(const QString& file)
{
    DMetadata meta(file);
    d->image.setMetadata(meta.data());
}

void DImgInterface::clearUndoManager()
{
    d->undoMan->clear();
    d->undoMan->setOrigin();
    emit signalUndoStateChanged(false, false, false);
}

void DImgInterface::setUndoManagerOrigin()
{
    d->undoMan->setOrigin();
    emit signalUndoStateChanged(d->undoMan->anyMoreUndo(), d->undoMan->anyMoreRedo(), !d->undoMan->isAtOrigin());
}

void DImgInterface::updateUndoState()
{
    emit signalUndoStateChanged(d->undoMan->anyMoreUndo(), d->undoMan->anyMoreRedo(), !d->undoMan->isAtOrigin());
}

bool DImgInterface::imageValid()
{
    return d->valid;
}

int DImgInterface::width()
{
    return d->width;
}

int DImgInterface::height()
{
    return d->height;
}

int DImgInterface::origWidth()
{
    return d->origWidth;
}

int DImgInterface::origHeight()
{
    return d->origHeight;
}

int DImgInterface::bytesDepth()
{
    return d->image.bytesDepth();
}

bool DImgInterface::sixteenBit()
{
    return d->image.sixteenBit();
}

bool DImgInterface::hasAlpha()
{
    return d->image.hasAlpha();
}

bool DImgInterface::isReadOnly()
{
    if (d->image.isNull())
        return true;
    else
        return d->image.isReadOnly();
}

void DImgInterface::setSelectedArea(int x, int y, int w, int h)
{
    d->selX = x;
    d->selY = y;
    d->selW = w;
    d->selH = h;
}

void DImgInterface::getSelectedArea(int& x, int& y, int& w, int& h)
{
    x = d->selX;
    y = d->selY;
    w = d->selW;
    h = d->selH;
}

void DImgInterface::paintOnDevice(QPaintDevice* p,
                                  int sx, int sy, int sw, int sh,
                                  int dx, int dy, int dw, int dh,
                                  int /*antialias*/)
{
    if (d->image.isNull())
        return;

    DImg img = d->image.smoothScaleSection(sx, sy, sw, sh, dw, dh);
    img.convertDepth(32);
    QPainter painter(p);

    if (d->cmSettings->enableCM && (d->cmSettings->useManagedView || d->doSoftProofing))
    {
        QPixmap pix(img.convertToPixmap(d->monitorICCtrans));
        painter.drawPixmap(dx, dy, pix, 0, 0, pix.width(), pix.height());
    }
    else
    {
        QPixmap pix(img.convertToPixmap());
        painter.drawPixmap(dx, dy, pix, 0, 0, pix.width(), pix.height());
    }

    // Show the Over/Under exposure pixels indicators

    if (d->expoSettings->underExposureIndicator || d->expoSettings->overExposureIndicator)
    {
        QImage pureColorMask = d->image.copy(sx, sy, sw, sh).pureColorMask(d->expoSettings);
        QPixmap pixMask      = QPixmap::fromImage(pureColorMask.scaled(dw, dh));
        painter.drawPixmap(dx, dy, pixMask, 0, 0, pixMask.width(), pixMask.height());
    }

    painter.end();
}

void DImgInterface::paintOnDevice(QPaintDevice* p,
                                  int sx, int sy, int sw, int sh,
                                  int dx, int dy, int dw, int dh,
                                  int mx, int my, int mw, int mh,
                                  int /*antialias*/)
{
    if (d->image.isNull())
        return;

    DImg img = d->image.smoothScaleSection(sx, sy, sw, sh, dw, dh);
    img.convertDepth(32);
    QPainter painter(p);

    uint* data  = (uint*)img.bits();
    uchar r, g, b, a;

    for (int j=0; j < (int)img.height(); ++j)
    {
        for (int i=0; i < (int)img.width(); ++i)
        {
            if (i < (mx-dx) || i > (mx-dx+mw-1) ||
                j < (my-dy) || j > (my-dy+mh-1))
            {
                a = (*data >> 24) & 0xff;
                r = (*data >> 16) & 0xff;
                g = (*data >>  8) & 0xff;
                b = (*data      ) & 0xff;

                r += (uchar)((RCOL - r) * OPACITY);
                g += (uchar)((GCOL - g) * OPACITY);
                b += (uchar)((BCOL - b) * OPACITY);

                *data = (a << 24) | (r << 16) | (g << 8) | b;
            }

            ++data;
        }
    }

    if (d->cmSettings->enableCM && (d->cmSettings->useManagedView || d->doSoftProofing))
    {
        QPixmap pix(img.convertToPixmap(d->monitorICCtrans));
        painter.drawPixmap(dx, dy, pix, 0, 0, pix.width(), pix.height());
    }
    else
    {
        QPixmap pix(img.convertToPixmap());
        painter.drawPixmap(dx, dy, pix, 0, 0, pix.width(), pix.height());
    }

    // Show the Over/Under exposure pixels indicators

    if (d->expoSettings->underExposureIndicator || d->expoSettings->overExposureIndicator)
    {
        QImage pureColorMask = d->image.copy(sx, sy, sw, sh).pureColorMask(d->expoSettings);
        QPixmap pixMask      = QPixmap::fromImage(pureColorMask.scaled(dw, dh));
        painter.drawPixmap(dx, dy, pixMask, 0, 0, pixMask.width(), pixMask.height());
    }

    painter.end();
}

void DImgInterface::zoom(double val)
{
    d->zoom   = val;
    d->width  = (int)(d->origWidth  * val);
    d->height = (int)(d->origHeight * val);
}

void DImgInterface::rotate90(bool saveUndo)
{
    if (saveUndo)
    {
        d->undoMan->addAction(new UndoActionRotate(this, UndoActionRotate::R90));
    }

    d->image.rotate(DImg::ROT90);
    d->origWidth  = d->image.width();
    d->origHeight = d->image.height();

    setModified();
}

void DImgInterface::rotate180(bool saveUndo)
{
    if (saveUndo)
    {
        d->undoMan->addAction(new UndoActionRotate(this, UndoActionRotate::R180));
    }

    d->image.rotate(DImg::ROT180);
    d->origWidth  = d->image.width();
    d->origHeight = d->image.height();

    setModified();
}

void DImgInterface::rotate270(bool saveUndo)
{
    if (saveUndo)
    {
        d->undoMan->addAction(new UndoActionRotate(this, UndoActionRotate::R270));
    }

    d->image.rotate(DImg::ROT270);
    d->origWidth  = d->image.width();
    d->origHeight = d->image.height();

    setModified();
}

void DImgInterface::flipHoriz(bool saveUndo)
{
    if (saveUndo)
    {
        d->undoMan->addAction(new UndoActionFlip(this, UndoActionFlip::Horizontal));
    }

    d->image.flip(DImg::HORIZONTAL);

    setModified();
}

void DImgInterface::flipVert(bool saveUndo)
{
    if (saveUndo)
    {
        d->undoMan->addAction(new UndoActionFlip(this, UndoActionFlip::Vertical));
    }

    d->image.flip(DImg::VERTICAL);

    setModified();
}

void DImgInterface::crop(int x, int y, int w, int h)
{
    d->undoMan->addAction(new UndoActionIrreversible(this, "Crop"));

    d->image.crop(x, y, w, h);

    d->origWidth  = d->image.width();
    d->origHeight = d->image.height();

    setModified();
}

void DImgInterface::resize(int w, int h)
{
    d->undoMan->addAction(new UndoActionIrreversible(this, "Resize"));

    d->image.resize(w, h);

    d->origWidth  = d->image.width();
    d->origHeight = d->image.height();

    setModified();
}

void DImgInterface::convertDepth(int depth)
{
    d->undoMan->addAction(new UndoActionIrreversible(this, "Convert Color Depth"));
    d->image.convertDepth(depth);

    setModified();
}

DImg* DImgInterface::getImg()
{
    if (!d->image.isNull())
    {
        return &d->image;
    }
    else
    {
        kWarning() << "d->image is NULL";
        return 0;
    }
}

uchar* DImgInterface::getImage()
{
    if (!d->image.isNull())
    {
        return d->image.bits();
    }
    else
    {
        kWarning() << "d->image is NULL";
        return 0;
    }
}

void DImgInterface::putImage(const QString& caller, uchar* data, int w, int h)
{
    putImage(caller, data, w, h, d->image.sixteenBit());
}

void DImgInterface::putImage(const QString& caller, uchar* data, int w, int h, bool sixteenBit)
{
    d->undoMan->addAction(new UndoActionIrreversible(this, caller));
    putImage(data, w, h, sixteenBit);
}

void DImgInterface::putImage(uchar* data, int w, int h)
{
    putImage(data, w, h, d->image.sixteenBit());
}

void DImgInterface::putImage(uchar* data, int w, int h, bool sixteenBit)
{
    if (d->image.isNull())
    {
       kWarning() << "d->image is NULL";
       return;
    }

    if (!data)
    {
       kWarning() << "New image is NULL";
       return;
    }

    if (w == -1 && h == -1)
    {
        // New image size
        w = d->origWidth;
        h = d->origHeight;
    }
    else
    {
        // New image size == original size
        d->origWidth  = w;
        d->origHeight = h;
    }

    //kDebug() << data << " " << w << " " << h;
    d->image.putImageData(w, h, sixteenBit, d->image.hasAlpha(), data);

    setModified();
}

void DImgInterface::putIccProfile(const IccProfile& profile)
{
    if (d->image.isNull())
    {
        kWarning() << "d->image is NULL";
        return;
    }

    //kDebug() << "Embedding profile: " << profile;
    d->image.setIccProfile(profile);
    updateColorManagement();
    setModified();
}

uchar* DImgInterface::getImageSelection()
{
    if (!d->selW || !d->selH)
        return 0;

    if (!d->image.isNull())
    {
        DImg im = d->image.copy(d->selX, d->selY, d->selW, d->selH);
        return im.stripImageData();
    }

    return 0;
}

void DImgInterface::putImageSelection(const QString& caller, uchar* data)
{
    if (!data || d->image.isNull())
        return;

    d->undoMan->addAction(new UndoActionIrreversible(this, caller));

    d->image.bitBltImage(data, 0, 0, d->selW, d->selH, d->selX, d->selY, d->selW, d->selH, d->image.bytesDepth());

    setModified();
}

void DImgInterface::getUndoHistory(QStringList& titles)
{
    d->undoMan->getUndoHistory(titles);
}

void DImgInterface::getRedoHistory(QStringList& titles)
{
    d->undoMan->getRedoHistory(titles);
}

IccProfile DImgInterface::getEmbeddedICC()
{
    return d->image.getIccProfile();
}

KExiv2Data DImgInterface::getMetadata()
{
    return d->image.getMetadata();
}

QString DImgInterface::getImageFilePath()
{
    return d->filename;
}

QString DImgInterface::getImageFileName()
{
    return d->filename.section( '/', -1 );
}

QString DImgInterface::getImageFormat()
{
    if (d->image.isNull())
        return QString();

    QString mimeType = d->image.attribute("format").toString();
    // It is a bug in the loader if format attribute is not given
    if (mimeType.isEmpty())
    {
        kWarning() << "DImg object does not contain attribute \"format\"";
        mimeType = QImageReader::imageFormat(d->filename);
    }
    return mimeType;
}

ICCSettingsContainer* DImgInterface::getICCSettings()
{
    return d->cmSettings;
}

QPixmap DImgInterface::convertToPixmap(DImg& img)
{
    if (d->cmSettings->enableCM && (d->cmSettings->useManagedView || d->doSoftProofing))
    {
        // dont use d->monitorICCtrans here, because img may have a different embedded profile
        IccManager manager(img);
        IccTransform transform;

        if (d->doSoftProofing)
            transform = manager.displaySoftProofingTransform(d->cmSettings->defaultProofProfile, d->displayingWidget);
        else
            transform = manager.displayTransform(d->displayingWidget);

        return img.convertToPixmap(transform);
    }

    return img.convertToPixmap();
}

QColor DImgInterface::underExposureColor()
{
    return d->expoSettings->underExposureColor;
}

QColor DImgInterface::overExposureColor()
{
    return d->expoSettings->overExposureColor;
}

}  // namespace Digikam
