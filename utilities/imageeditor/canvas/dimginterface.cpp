/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *         Gilles Caulier <caulier dot gilles at kdemail dot net> 
 * Date  : 2003-01-15
 * Description : DImg interface for image editor
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

// C++ includes.

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>

// Qt includes.

#include <qwidget.h>
#include <qstring.h>
#include <qpixmap.h>
#include <qbitmap.h>
#include <qcolor.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qvariant.h>

// KDE includes.

#include <kdebug.h>
#include <kcursor.h>
#include <kmessagebox.h>

// Local includes.

#include "bcgmodifier.h"
#include "icctransform.h"
#include "colorcorrectiondlg.h"
#include "undomanager.h"
#include "undoaction.h"
#include "iccsettingscontainer.h"
#include "iofilesettingscontainer.h"
#include "sharedloadsavethread.h"
#include "dmetadata.h"
#include "dimginterface.h"

namespace Digikam
{

class UndoManager;

class DImgInterfacePrivate
{

public:

    DImgInterfacePrivate()
    {
        parent           = 0;
        undoMan          = 0;
        cmSettings       = 0;
        iofileSettings   = 0;
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
        exifOrient       = false;
        valid            = false;
        rotatedOrFlipped = false;
    }

    bool                     valid;
    bool                     rotatedOrFlipped;
    bool                     exifOrient;
    bool                     changedBCG;
    
    int                      width;
    int                      height;
    int                      origWidth;
    int                      origHeight;
    int                      selX;
    int                      selY;
    int                      selW;
    int                      selH;
    
    float                    gamma;
    float                    brightness;
    float                    contrast;

    double                   zoom;
    
    // Used by ICC color profile dialog.
    QWidget                 *parent;

    QString                  filename;
    QString                  savingFilename;

    DImg                     image;

    UndoManager             *undoMan;
    
    BCGModifier              cmod;

    ICCSettingsContainer    *cmSettings;

    IOFileSettingsContainer *iofileSettings;

    SharedLoadSaveThread    *thread;

    IccTransform             monitorICCtrans;
};

DImgInterface* DImgInterface::instance()
{
    if (!m_instance)
    {
        new DImgInterface();
    }

    return m_instance;
}

DImgInterface* DImgInterface::m_instance = 0;

DImgInterface::DImgInterface()
             : QObject()
{
    m_instance = this;
    
    d = new DImgInterfacePrivate;

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
    m_instance = 0;
}

void DImgInterface::load(const QString& filename, IOFileSettingsContainer *iofileSettings,
                         QWidget *parent)
{
    // store here in case filename == d->fileName, and is then reset by resetValues
    QString newFileName = filename;

    resetValues();

    d->filename       = newFileName;
    d->iofileSettings = iofileSettings;
    d->parent         = parent;

    d->thread->load( LoadingDescription(d->filename, iofileSettings->rawDecodingSettings),
                     SharedLoadSaveThread::AccessModeReadWrite,
                     SharedLoadSaveThread::LoadingPolicyFirstRemovePrevious);
}

void DImgInterface::resetImage()
{
    resetValues();
    d->image.reset();
}

void DImgInterface::resetValues()
{
    d->valid          = false;
    d->filename       = QString();
    d->width          = 0;
    d->height         = 0;
    d->origWidth      = 0;
    d->origHeight     = 0;
    d->selX           = 0;
    d->selY           = 0;
    d->selW           = 0;
    d->selH           = 0;
    d->gamma          = 1.0;
    d->contrast       = 1.0;
    d->brightness     = 0.0;
    d->changedBCG     = false;
    d->iofileSettings = 0;
    d->parent         = 0;

    d->cmod.reset();
    d->undoMan->clear();
}

void DImgInterface::setICCSettings(ICCSettingsContainer *cmSettings)
{
    d->cmSettings = cmSettings;
    d->monitorICCtrans.setProfiles(d->cmSettings->inputSetting, d->cmSettings->monitorSetting);
}

void DImgInterface::slotImageLoaded(const LoadingDescription &loadingDescription, const DImg& img)
{
    const QString &fileName = loadingDescription.filePath;

    if (fileName != d->filename)
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

        // Raw files are already rotated properlly by dcraw. Only perform auto-rotation with JPEG/PNG/TIFF file.
        // We don't have a feedback from dcraw about auto-rotated RAW file during decoding. Well set transformed 
        // flag as well.

        if (d->image.attribute("format").toString() == QString("RAW"))
            d->rotatedOrFlipped = true;
    
        if (d->exifOrient && 
	    (d->image.attribute("format").toString() == QString("JPEG") ||
	     d->image.attribute("format").toString() == QString("PNG")  ||
	     d->image.attribute("format").toString() == QString("TIFF")))
    	    exifRotate(d->filename);
        
        if (d->cmSettings->enableCMSetting)
        {
            if ((d->image.attribute("format").toString() == QString("RAW"))  && (d->cmSettings->CMInRawLoadingSetting) )
            {
                // With RAW files, we load the Color Management image plugin.
                emit signalColorManagementTool();        
            }
            else if (QFile::exists(d->cmSettings->workspaceSetting) && 
                     QFile::exists(d->cmSettings->inputSetting))
            {
                IccTransform trans;
    
                // First possibility: image has no embedded profile
                if(d->image.getICCProfil().isNull())
                {
                    // Ask or apply?
                    if (d->cmSettings->askOrApplySetting)
                    {
                        if (d->parent) d->parent->setCursor( KCursor::waitCursor() );
                        trans.setProfiles( QFile::encodeName(d->cmSettings->inputSetting),
                                           QFile::encodeName(d->cmSettings->workspaceSetting));
                        trans.apply( d->image );
                        d->image.getICCProfilFromFile(QFile::encodeName(d->cmSettings->workspaceSetting));
                        if (d->parent) d->parent->unsetCursor();
                    }
                    else
                    {
                        // To repaint image in canvas before to ask about to apply ICC profile.
                        emit signalImageLoaded(d->filename, valRet);
    
                        DImg preview = d->image.smoothScale(240, 180, QSize::ScaleMin);
                        trans.setProfiles(QFile::encodeName(d->cmSettings->inputSetting),
                                          QFile::encodeName(d->cmSettings->workspaceSetting));
                        ColorCorrectionDlg dlg(d->parent, &preview, &trans, fileName);

                        switch (dlg.exec())
                        {
                            case QDialog::Accepted:
                                if (d->parent) d->parent->setCursor( KCursor::waitCursor() );
                                trans.apply( d->image );
                                d->image.getICCProfilFromFile(QFile::encodeName(d->cmSettings->workspaceSetting));
                                if (d->parent) d->parent->unsetCursor();
                            break;
                            case -1:
                                if (d->parent) d->parent->setCursor( KCursor::waitCursor() );
                                d->image.getICCProfilFromFile(QFile::encodeName(d->cmSettings->workspaceSetting));
                                if (d->parent) d->parent->unsetCursor();
                                kdDebug() << "dimginterface.cpp: Apply pressed" << endl;
                            break;
                        }
                    }
                }
                // Second possibility: image has an embedded profile
                else
                {
                    trans.getEmbeddedProfile( d->image );
    
                    // Ask or apply?
                    if (d->cmSettings->askOrApplySetting)
                    {
                        if (d->parent) d->parent->setCursor( KCursor::waitCursor() );
                        trans.setProfiles(QFile::encodeName(d->cmSettings->workspaceSetting));
                        trans.apply( d->image );
                        if (d->parent) d->parent->unsetCursor();
                    }
                    else
                    {
                        if (trans.getEmbeddedProfileDescriptor()
                            != trans.getProfileDescription( d->cmSettings->workspaceSetting ))
                        {
                            // Embedded profile and default workspace profile are different: ask to user!
                        
                            kdDebug() << "Embedded profile: " << trans.getEmbeddedProfileDescriptor() << endl;
    
                            // To repaint image in canvas before to ask about to apply ICC profile.
                            emit signalImageLoaded(d->filename, valRet);
    
                            DImg preview = d->image.smoothScale(240, 180, QSize::ScaleMin);
                            trans.setProfiles(QFile::encodeName(d->cmSettings->workspaceSetting));
                            ColorCorrectionDlg dlg(d->parent, &preview, &trans, fileName);
    
                            switch (dlg.exec())
                            {
                                case QDialog::Accepted:
                                    if (d->parent) d->parent->setCursor( KCursor::waitCursor() );
                                    trans.apply( d->image );
                                    d->image.getICCProfilFromFile(QFile::encodeName(d->cmSettings->workspaceSetting));
                                    if (d->parent) d->parent->unsetCursor();
                                break;
                                case -1:
                                    if (d->parent) d->parent->setCursor( KCursor::waitCursor() );
                                    d->image.getICCProfilFromFile(QFile::encodeName(d->cmSettings->workspaceSetting));
                                    if (d->parent) d->parent->unsetCursor();
                                    kdDebug() << "dimginterface.cpp: Apply pressed" << endl;
                                break;
                            }
                        }
                    }
                }
            }
            else
            {
                QString message = i18n("ICC profiles path seems to be invalid. "
                                       "No color transform will be done. "
                                       "Please, check the color management "
                                       "configuration it in digiKam setup.");
                KMessageBox::information(d->parent, message);
            }
        }
    }
    else
    {
        valRet = false;
    }

    emit signalImageLoaded(d->filename, valRet);
    setModified();

    // Note: in showfoto, we using a null filename to clear canvas.
    if (!valRet && fileName != QString::null)
    {
        QFileInfo fi(fileName);
        QString message = i18n("Failed to load image \"%1\"").arg(fi.fileName());
        KMessageBox::error(d->parent, message);
        kdWarning() << "Failed to load image " << fi.fileName() << endl;
    }
}

void DImgInterface::slotLoadingProgress(const LoadingDescription &loadingDescription, float progress)
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

    if(orientation != DMetadata::ORIENTATION_NORMAL) 
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

void DImgInterface::restore()
{
    d->undoMan->clear();
    load(d->filename, d->iofileSettings);
}

/*
This code is unused and untested
void DImgInterface::save(const QString& file, IOFileSettingsContainer *iofileSettings)
{
    d->cmod.reset();
    d->cmod.setGamma(d->gamma);
    d->cmod.setBrightness(d->brightness);
    d->cmod.setContrast(d->contrast);
    d->cmod.applyBCG(d->image);

    d->cmod.reset();
    d->gamma      = 1.0;
    d->contrast   = 1.0;
    d->brightness = 0.0;

    QString currentMimeType(QImageIO::imageFormat(d->filename));

    d->needClearUndoManager = true;

    saveAction(file, iofileSettings, currentMimeType);
}
*/

void DImgInterface::saveAs(const QString& fileName, IOFileSettingsContainer *iofileSettings,
                           bool setExifOrientationTag, const QString& givenMimeType)
{
    // No need to toggle off undo, redo or save action during saving using 
    // signalUndoStateChanged(), this is will done by GUI implementation directly.

    if (d->changedBCG)
    {
        d->cmod.reset();
        d->cmod.setGamma(d->gamma);
        d->cmod.setBrightness(d->brightness);
        d->cmod.setContrast(d->contrast);
        d->cmod.applyBCG(d->image);
    }

    // Try hard to find a mimetype.
    QString mimeType = givenMimeType;

    // This is possibly empty
    if (mimeType.isEmpty())
    {
        mimeType = d->image.attribute("format").toString();
        // It is a bug if format attribute is not given
        if (mimeType.isEmpty())
        {
            kdWarning() << "DImg object does not contain attribute \"format\"" << endl;
            mimeType = QImageIO::imageFormat(d->filename);
        }
    }

    kdDebug() << "Saving to :" << QFile::encodeName(fileName).data() << " (" 
              << mimeType << ")" << endl;

    if ( mimeType.upper() == QString("JPG") || mimeType.upper() == QString("JPEG") || 
         mimeType.upper() == QString("JPE")) 
       d->image.setAttribute("quality", iofileSettings->JPEGCompression);

    if ( mimeType.upper() == QString("PNG") ) 
       d->image.setAttribute("quality", iofileSettings->PNGCompression);

    if ( mimeType.upper() == QString("TIFF") || mimeType.upper() == QString("TIF") ) 
       d->image.setAttribute("compress", iofileSettings->TIFFCompression);

    d->savingFilename = fileName;

    // Get image Exif/Iptc data.
    DMetadata meta;
    meta.setExif(d->image.getExif());
    meta.setIptc(d->image.getIptc());

    // Update Iptc preview.
    QImage preview = d->image.smoothScale(800, 600, QSize::ScaleMin).copyQImage();

    // TODO: see B.K.O #130525. a JPEG segment is limited to 64K. If the IPTC byte array is
    // bigger than 64K duing of image preview tag size, the target JPEG image will be
    // broken. Note that IPTC image preview tag is limited to 256K!!!
    // Temp. solution to disable IPTC preview record in JPEG file until a right solution 
    // will be found into Exiv2.
    // Note : There is no limitation with TIFF and PNG about IPTC byte array size.

    if ( mimeType.upper() != QString("JPG") && mimeType.upper() != QString("JPEG") && 
         mimeType.upper() != QString("JPE")) 
        meta.setImagePreview(preview);

    // Update Exif thumbnail.
    QImage thumb = preview.smoothScale(160, 120, QImage::ScaleMin);
    meta.setExifThumbnail(thumb);

    // Update Exif Image dimensions.
    meta.setImageDimensions(d->image.size());

    // Update Exif Document Name tag with the original file name.
    meta.setExifTagString("Exif.Image.DocumentName", getImageFileName());

    // Update Exif Orientation tag if necessary.
    if( setExifOrientationTag )
        meta.setImageOrientation(DMetadata::ORIENTATION_NORMAL);

    // Store new Exif/Iptc data into image.
    d->image.setExif(meta.getExif());
    d->image.setIptc(meta.getIptc());

    d->thread->save(d->image, fileName, mimeType);
}

void DImgInterface::slotImageSaved(const QString& filePath, bool success)
{
    if (filePath != d->savingFilename)
        return;

    if (!success)
        kdWarning() << "error saving image '" << QFile::encodeName(filePath).data() << endl;

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
    QString savedformat = d->image.attribute("savedformat").toString();
    if (!savedformat.isEmpty())
        d->image.setAttribute("format", savedformat);
}

void DImgInterface::setModified()
{
    emit signalModified();
    emit signalUndoStateChanged(d->undoMan->anyMoreUndo(), d->undoMan->anyMoreRedo(), !d->undoMan->isAtOrigin());
}

void DImgInterface::readMetadataFromFile(const QString &file)
{
    DMetadata meta(file);

    //TODO: code is essentially the same as DImgLoader::readMetadata,
    //      put both in a common place.
    if (!meta.getComments().isNull())
        d->image.setComments(meta.getComments());
    if (!meta.getExif().isNull())
        d->image.setExif(meta.getExif());
    if (!meta.getIptc().isNull())
        d->image.setIptc(meta.getIptc());
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
    return !d->image.isNull();
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
    d->cmod.applyBCG(img);
    img.convertDepth(32);

    if (d->cmSettings->enableCMSetting && d->cmSettings->managedViewSetting)
    {
        QPixmap pix(img.convertToPixmap(&d->monitorICCtrans));
        bitBlt(p, dx, dy, &pix, 0, 0);
    }
    else
    {
        QPixmap pix(img.convertToPixmap());
        bitBlt(p, dx, dy, &pix, 0, 0);
    }
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
    d->cmod.applyBCG(img);
    img.convertDepth(32);

    uint* data  = (uint*)img.bits();

    uchar r, g, b, a;
    uchar color = 0xAA;

    for (int j=0; j < (int)img.height(); j++)
    {
        for (int i=0; i < (int)img.width(); i++)
        {
            if (i < (mx-dx) || i > (mx-dx+mw-1) ||
                j < (my-dy) || j > (my-dy+mh-1))
            {
                a = (*data >> 24) & 0xff;
                r = (*data >> 16) & 0xff;
                g = (*data >>  8) & 0xff;
                b = (*data      ) & 0xff;

                r = ((r-color) >> 2 + 1) + color;
                g = ((g-color) >> 2 + 1) + color;
                b = ((b-color) >> 2 + 1) + color;

                *data = (a << 24) | (r << 16) | (g << 8) | b;
            }

            data++;
        }
    }

    if (d->cmSettings->enableCMSetting && d->cmSettings->managedViewSetting)
    {
        QPixmap pix(img.convertToPixmap(&d->monitorICCtrans));
        bitBlt(p, dx, dy, &pix, 0, 0);
    }
    else
    {
        QPixmap pix(img.convertToPixmap());
        bitBlt(p, dx, dy, &pix, 0, 0);
    }
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

void DImgInterface::changeGamma(double gamma)
{
    d->undoMan->addAction(new UndoActionBCG(this, d->gamma, d->brightness,
                                            d->contrast, gamma, d->brightness,
                                            d->contrast));

    d->gamma += gamma/10.0;

    d->cmod.reset();
    d->cmod.setGamma(d->gamma);
    d->cmod.setBrightness(d->brightness);
    d->cmod.setContrast(d->contrast);
    d->changedBCG = true;

    setModified();
}

void DImgInterface::changeBrightness(double brightness)
{
    d->undoMan->addAction(new UndoActionBCG(this, d->gamma, d->brightness,
                                            d->contrast, d->gamma, brightness,
                                            d->contrast));

    d->brightness += brightness/100.0;

    d->cmod.reset();
    d->cmod.setGamma(d->gamma);
    d->cmod.setBrightness(d->brightness);
    d->cmod.setContrast(d->contrast);
    d->changedBCG = true;

    setModified();
}

void DImgInterface::changeContrast(double contrast)
{
    d->undoMan->addAction(new UndoActionBCG(this, d->gamma, d->brightness,
                                            d->contrast, d->gamma, d->brightness,
                                            contrast));

    d->contrast += contrast/100.0;

    d->cmod.reset();
    d->cmod.setGamma(d->gamma);
    d->cmod.setBrightness(d->brightness);
    d->cmod.setContrast(d->contrast);
    d->changedBCG = true;

    setModified();
}

void DImgInterface::changeBCG(double gamma, double brightness, double contrast)
{
    d->gamma      = gamma;
    d->brightness = brightness;
    d->contrast   = contrast;

    d->cmod.reset();
    d->cmod.setGamma(d->gamma);
    d->cmod.setBrightness(d->brightness);
    d->cmod.setContrast(d->contrast);
    d->changedBCG = true;

    setModified();
}

void DImgInterface::setBCG(double brightness, double contrast, double gamma)
{
    d->undoMan->addAction(new UndoActionIrreversible(this, "Brightness, Contrast, Gamma"));

    d->cmod.reset();
    d->cmod.setGamma(gamma);
    d->cmod.setBrightness(brightness);
    d->cmod.setContrast(contrast);
    d->cmod.applyBCG(d->image);

    d->cmod.reset();
    d->gamma      = 1.0;
    d->contrast   = 1.0;
    d->brightness = 0.0;
    d->changedBCG = false;

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
        kdWarning() << k_funcinfo << "d->image is NULL" << endl;
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
        kdWarning() << k_funcinfo << "d->image is NULL" << endl;
        return 0;
    }
}

void DImgInterface::putImage(const QString &caller, uchar* data, int w, int h)
{
    putImage(caller, data, w, h, d->image.sixteenBit());
}

void DImgInterface::putImage(const QString &caller, uchar* data, int w, int h, bool sixteenBit)
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
       kdWarning() << k_funcinfo << "d->image is NULL" << endl;
       return;
    }

    if (!data)
    {
       kdWarning() << k_funcinfo << "New image is NULL" << endl;
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

    //kdDebug() << k_funcinfo << data << " " << w << " " << h << endl;
    d->image.putImageData(w, h, sixteenBit, d->image.hasAlpha(), data);

    setModified();
}

void DImgInterface::setEmbeddedICCToOriginalImage( QString profilePath)
{
    if (d->image.isNull())
    {
        kdWarning() << k_funcinfo << "d->image is NULL" << endl;
        return;
    }
     
     kdDebug() << k_funcinfo << "Embedding profile: " << profilePath << endl;
     d->image.getICCProfilFromFile( QFile::encodeName(profilePath));
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

void DImgInterface::putImageSelection(const QString &caller, uchar* data)
{
    if (!data || d->image.isNull())
        return;

    d->undoMan->addAction(new UndoActionIrreversible(this, caller));

    d->image.bitBltImage(data, 0, 0, d->selW, d->selH, d->selX, d->selY, d->selW, d->selH, d->image.bytesDepth());

    setModified();
}

void DImgInterface::getUndoHistory(QStringList &titles)
{
    d->undoMan->getUndoHistory(titles);
}

void DImgInterface::getRedoHistory(QStringList &titles)
{
    d->undoMan->getRedoHistory(titles);
}

QByteArray DImgInterface::getEmbeddedICC()
{
    return d->image.getICCProfil();
}

QByteArray DImgInterface::getExif()
{
    return d->image.getExif();
}

QByteArray DImgInterface::getIptc()
{
    return d->image.getIptc();
}

QString DImgInterface::getImageFileName()
{
    return d->filename.section( '/', -1 );
}

ICCSettingsContainer* DImgInterface::getICCSettings()
{
    return d->cmSettings;
}

}  // namespace Digikam

#include "dimginterface.moc"
