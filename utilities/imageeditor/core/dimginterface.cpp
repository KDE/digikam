/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-01-15
 * Description : DImg interface for image editor
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2004-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "colorcorrectiondlg.h"
#include "dimgbuiltinfilter.h"
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
#include "dimg.h"
#include "dimgfiltergenerator.h"
#include "bcgfilter.h"
#include "equalizefilter.h"
#include "dimgfiltermanager.h"
#include "versionmanager.h"

namespace Digikam
{

class UndoManager;

class FileToSave
{
public:

    QString                 fileName;
    QString                 filePath;
    QString                 intendedFilePath;
    int                     historyStep;
    QString                 mimeType;
    QMap<QString, QVariant> ioAttributes;
    bool                    setExifOrientationTag;
    DImg                    image;
};

// --------------------------------------------------------------

class DImgInterface::Private
{

public:

    Private() :
        valid(false),
        rotatedOrFlipped(false),
        exifOrient(false),
        doSoftProofing(false),
        width(0),
        height(0),
        origWidth(0),
        origHeight(0),
        selX(0),
        selY(0),
        selW(0),
        selH(0),
        gamma(0.0),
        brightness(0.0),
        contrast(0.0),
        zoom(1.0),
        displayingWidget(0),
        currentFileToSave(0),
        undoMan(0),
        expoSettings(0),
        thread(0)
    {
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

    QList<FileToSave>          filesToSave;
    int                        currentFileToSave;

    DImg                       image;
    DImageHistory              resolvedInitialHistory;
    UndoManager*               undoMan;

    ICCSettingsContainer       cmSettings;

    ExposureSettingsContainer* expoSettings;

    SharedLoadSaveThread*      thread;
    LoadingDescription         currentDescription;
    LoadingDescription         nextRawDescription;

    IccTransform               monitorICCtrans;
};

// --------------------------------------------------------------

DImgInterface::UndoState::UndoState()
    : hasUndo(false),
      hasRedo(false),
      hasChanges(false),
      hasUndoableChanges(false)
{
}

DImgInterface::UndoState DImgInterface::undoState() const
{
    UndoState state;
    state.hasUndo = d->undoMan->anyMoreUndo();
    state.hasRedo = d->undoMan->anyMoreRedo();
    state.hasUndoableChanges = !d->undoMan->isAtOrigin();
    // Includes the edit step performed by RAW import, which is not undoable
    state.hasChanges = d->undoMan->hasChanges();

    return state;
}

// --------------------------------------------------------------

DImgInterface* DImgInterface::m_defaultInterface = 0;

DImgInterface* DImgInterface::defaultInterface()
{
    return m_defaultInterface;
}

void DImgInterface::setDefaultInterface(DImgInterface* const defaultInterface)
{
    m_defaultInterface = defaultInterface;
}

DImgInterface::DImgInterface()
    : QObject(), d(new Private)
{
    d->undoMan = new UndoManager(this);
    d->thread  = new SharedLoadSaveThread;

    connect( d->thread, SIGNAL(signalImageLoaded(LoadingDescription, DImg)),
             this, SLOT(slotImageLoaded(LoadingDescription, DImg)) );

    connect( d->thread, SIGNAL(signalImageSaved(QString, bool)),
             this, SLOT(slotImageSaved(QString, bool)) );

    connect( d->thread, SIGNAL(signalLoadingProgress(LoadingDescription, float)),
             this, SLOT(slotLoadingProgress(LoadingDescription, float)) );

    connect( d->thread, SIGNAL(signalSavingProgress(QString, float)),
             this, SLOT(slotSavingProgress(QString, float)) );
}

DImgInterface::~DImgInterface()
{
    delete d->undoMan;
    delete d->thread;
    delete d;

    if (m_defaultInterface == this)
    {
        m_defaultInterface = 0;
    }
}

void DImgInterface::setDisplayingWidget(QWidget* const widget)
{
    d->displayingWidget = widget;
}

void DImgInterface::load(const QString& filePath, IOFileSettingsContainer* const iofileSettings)
{
    LoadingDescription description(filePath, LoadingDescription::ConvertForEditor);

    if (DImg::fileFormat(filePath) == DImg::RAW)
    {
        description = LoadingDescription(filePath, iofileSettings->rawDecodingSettings,
                                         LoadingDescription::RawDecodingGlobalSettings,
                                         LoadingDescription::ConvertForEditor);

        if (EditorToolIface::editorToolIface() && iofileSettings->useRAWImport)
        {
            d->nextRawDescription = description;

            RawImport* rawImport = new RawImport(KUrl(filePath), this);
            EditorToolIface::editorToolIface()->loadTool(rawImport);

            connect(rawImport, SIGNAL(okClicked()),
                    this, SLOT(slotLoadRawFromTool()));

            connect(rawImport, SIGNAL(cancelClicked()),
                    this, SLOT(slotLoadRaw()));

            d->thread->stopLoading();
            return;
        }
    }
    else
    {
        d->nextRawDescription = LoadingDescription();
    }

    load(description);
}

void DImgInterface::slotLoadRawFromTool()
{
    if (EditorToolIface::editorToolIface())
    {
        RawImport* rawImport = dynamic_cast<RawImport*>(EditorToolIface::editorToolIface()->currentTool());

        if (rawImport)
        {
            d->nextRawDescription.rawDecodingSettings = rawImport->rawDecodingSettings();
            d->nextRawDescription.rawDecodingHint     = LoadingDescription::RawDecodingCustomSettings;
        }

        if (rawImport->hasPostProcessedImage())
        {
            resetValues();
            d->currentDescription = d->nextRawDescription;
            d->nextRawDescription = LoadingDescription();

            emit signalLoadingStarted(d->currentDescription.filePath);
            slotImageLoaded(d->currentDescription, rawImport->postProcessedImage());
            EditorToolIface::editorToolIface()->unLoadTool();
            emit signalImageLoaded(d->currentDescription.filePath, true);
        }
        else
        {
            slotLoadRaw();
        }
    }
}

void DImgInterface::slotLoadRaw()
{
    //    kDebug() << d->nextRawDescription.rawDecodingSettings;
    load(d->nextRawDescription);
    d->nextRawDescription = LoadingDescription();
}

void DImgInterface::load(const LoadingDescription& description)
{
    if (EditorToolIface::editorToolIface())
    {
        EditorToolIface::editorToolIface()->unLoadTool();
    }

    if (description != d->currentDescription)
    {
        resetValues();
        d->currentDescription = description;
        loadCurrent();
    }
    else
    {
        emit signalLoadingStarted(d->currentDescription.filePath);
        emit signalImageLoaded(d->currentDescription.filePath, true);
    }
}

void DImgInterface::applyTransform(const IccTransform& transform)
{
    if (!d->valid)
    {
        return;
    }

    d->currentDescription.postProcessingParameters.colorManagement = LoadingDescription::ApplyTransform;
    d->currentDescription.postProcessingParameters.setTransform(transform);
    loadCurrent();

    if (EditorToolIface::editorToolIface())
    {
        EditorToolIface::editorToolIface()->unLoadTool();
    }
}

void DImgInterface::loadCurrent()
{
    d->thread->load(d->currentDescription,
                    SharedLoadSaveThread::AccessModeReadWrite,
                    SharedLoadSaveThread::LoadingPolicyFirstRemovePrevious);
    emit signalLoadingStarted(d->currentDescription.filePath);
}

void DImgInterface::restore()
{
    LoadingDescription description = d->currentDescription;
    resetValues();
    load(description);
}

void DImgInterface::resetImage()
{
    if (EditorToolIface::editorToolIface())
    {
        EditorToolIface::editorToolIface()->unLoadTool();
    }

    resetValues();
    d->image.reset();
}

void DImgInterface::resetValues()
{
    d->valid                  = false;
    d->currentDescription     = LoadingDescription();
    d->width                  = 0;
    d->height                 = 0;
    d->origWidth              = 0;
    d->origHeight             = 0;
    d->selX                   = 0;
    d->selY                   = 0;
    d->selW                   = 0;
    d->selH                   = 0;
    d->gamma                  = 1.0f;
    d->contrast               = 1.0f;
    d->brightness             = 0.0f;
    d->resolvedInitialHistory = DImageHistory();
    d->undoMan->clear();
}

void DImgInterface::setICCSettings(const ICCSettingsContainer& cmSettings)
{
    d->cmSettings = cmSettings;
}

ICCSettingsContainer DImgInterface::getICCSettings() const
{
    return d->cmSettings;
}

void DImgInterface::setExposureSettings(ExposureSettingsContainer* const expoSettings)
{
    d->expoSettings = expoSettings;
}

ExposureSettingsContainer* DImgInterface::getExposureSettings() const
{
    return d->expoSettings;
}

void DImgInterface::slotImageLoaded(const LoadingDescription& loadingDescription, const DImg& img)
{
    if (loadingDescription != d->currentDescription)
    {
        return;
    }

    // RAW tool active? Discard previous loaded image
    if (!d->nextRawDescription.filePath.isNull())
    {
        return;
    }

    bool valRet = false;
    d->image    = img;

    if (!d->image.isNull())
    {
        d->valid      = true;
        valRet        = true;
        d->resolvedInitialHistory = d->image.getOriginalImageHistory();
        d->resolvedInitialHistory.clearReferredImages(); // default empty, real values set by higher level

        // Raw files are already rotated properly by dcraw. Only perform auto-rotation with non-RAW files.
        // We don't have a feedback from dcraw about auto-rotated RAW file during decoding.
        // Setting rotatedOrFlipped to true will reset the exif flag on save (the data is then already rotated)

        if (d->image.detectedFormat() == DImg::RAW)
        {
            d->rotatedOrFlipped = true;
        }
        else if (d->exifOrient)
        {
            // Do not rotate twice if already rotated, e.g. for full size preview.
            QVariant attribute(d->image.attribute("exifRotated"));

            if (!attribute.isValid() || !attribute.toBool())
            {
                d->rotatedOrFlipped = d->image.rotateAndFlip(LoadSaveThread::exifOrientation(d->image, loadingDescription.filePath));
            }
        }

        // set after rotation
        d->origWidth  = d->image.width();
        d->origHeight = d->image.height();
        d->width      = d->origWidth;
        d->height     = d->origHeight;

        updateColorManagement();
    }
    else
    {
        valRet = false;
    }

    emit signalImageLoaded(d->currentDescription.filePath, valRet);
    setModified();

    /*
     *  TODO: FilterManager test block -- to be removed later
     *
        FilterAction fa("digikam:BCGFilter", 1);
        fa.addParameter("contrast", 1);
        fa.addParameter("channel", 1);
        fa.addParameter("brightness", 1);
        fa.addParameter("gamma", 1.2);

        DImgThreadedFilter *f =  DImgFilterManager::instance()->createFilter("digikam:BCGFilter", 1);
        f->readParameters(fa);
        f->setupFilter(img);
        f->startFilterDirectly();
        delete f;
    */
}

void DImgInterface::updateColorManagement()
{
    IccManager manager(d->image);

    if (d->doSoftProofing)
    {
        d->monitorICCtrans = manager.displaySoftProofingTransform(d->cmSettings.defaultProofProfile, d->displayingWidget);
    }
    else
    {
        d->monitorICCtrans = manager.displayTransform(d->displayingWidget);
    }
}

void DImgInterface::setSoftProofingEnabled(bool enabled)
{
    d->doSoftProofing = enabled;
    updateColorManagement();
}

void DImgInterface::slotLoadingProgress(const LoadingDescription& loadingDescription, float progress)
{
    if (loadingDescription == d->currentDescription)
    {
        emit signalLoadingProgress(loadingDescription.filePath, progress);
    }
}

bool DImgInterface::exifRotated() const
{
    return d->rotatedOrFlipped;
}

void DImgInterface::setExifOrient(bool exifOrient)
{
    d->exifOrient = exifOrient;
}

void DImgInterface::undo()
{
    if (!d->undoMan->anyMoreUndo())
    {
        emit signalUndoStateChanged();
        return;
    }

    d->undoMan->undo();
    emit signalUndoStateChanged();
}

void DImgInterface::redo()
{
    if (!d->undoMan->anyMoreRedo())
    {
        emit signalUndoStateChanged();
        return;
    }

    d->undoMan->redo();
    emit signalUndoStateChanged();
}

void DImgInterface::rollbackToOrigin()
{
    d->undoMan->rollbackToOrigin();
    emit signalUndoStateChanged();
}

QMap<QString, QVariant> DImgInterface::ioAttributes(IOFileSettingsContainer* const iofileSettings,
                                                    const QString& mimeType) const
{
    QMap<QString, QVariant> attributes;

    // JPEG file format.
    if (mimeType.toUpper() == QString("JPG") || mimeType.toUpper() == QString("JPEG") ||
        mimeType.toUpper() == QString("JPE"))
    {
        attributes.insert("quality",     iofileSettings->JPEGCompression);
        attributes.insert("subsampling", iofileSettings->JPEGSubSampling);
    }

    // PNG file format.
    if (mimeType.toUpper() == QString("PNG"))
    {
        attributes.insert("quality", iofileSettings->PNGCompression);
    }

    // TIFF file format.
    if (mimeType.toUpper() == QString("TIFF") || mimeType.toUpper() == QString("TIF"))
    {
        attributes.insert("compress", iofileSettings->TIFFCompression);
    }

    // JPEG 2000 file format.
    if (mimeType.toUpper() == QString("JP2") || mimeType.toUpper() == QString("JPX") ||
        mimeType.toUpper() == QString("JPC") || mimeType.toUpper() == QString("PGX") ||
        mimeType.toUpper() == QString("J2K"))
    {
        if (iofileSettings->JPEG2000LossLess)
        {
            attributes.insert("quality", 100);    // LossLess compression
        }
        else
        {
            attributes.insert("quality", iofileSettings->JPEG2000Compression);
        }
    }

    // PGF file format.
    if (mimeType.toUpper() == QString("PGF"))
    {
        if (iofileSettings->PGFLossLess)
        {
            attributes.insert("quality", 0);    // LossLess compression
        }
        else
        {
            attributes.insert("quality", iofileSettings->PGFCompression);
        }
    }

    return attributes;
}

void DImgInterface::saveAs(const QString& filePath, IOFileSettingsContainer* const iofileSettings,
                           bool setExifOrientationTag, const QString& givenMimeType,
                           const QString& intendedFilePath)
{
    saveAs(filePath, iofileSettings, setExifOrientationTag, givenMimeType, VersionFileOperation(), intendedFilePath);
}

void DImgInterface::saveAs(const QString& filePath, IOFileSettingsContainer* const iofileSettings,
                           bool setExifOrientationTag, const QString& givenMimeType,
                           const VersionFileOperation& op)
{
    saveAs(filePath, iofileSettings, setExifOrientationTag, givenMimeType, op, op.saveFile.filePath());
}

void DImgInterface::saveAs(const QString& filePath, IOFileSettingsContainer* const iofileSettings,
                           bool setExifOrientationTag, const QString& givenMimeType,
                           const VersionFileOperation& op, const QString& intendedFilePath)
{
    // No need to toggle off undo, redo or save action during saving using
    // signalUndoStateChanged(), this is will done by GUI implementation directly.

    emit signalSavingStarted(filePath);

    d->filesToSave.clear();
    d->currentFileToSave = 0;

    QString mimeType = givenMimeType;

    // This is possibly empty
    if (mimeType.isEmpty())
    {
        mimeType = getImageFormat();
    }

    if (op.tasks & VersionFileOperation::MoveToIntermediate ||
        op.tasks & VersionFileOperation::SaveAndDelete)
    {
        // The current file will stored away at a different name. Adjust history.
        d->image.getImageHistory().moveCurrentReferredImage(op.intermediateForLoadedFile.path,
                                                            op.intermediateForLoadedFile.fileName);
    }

    if (op.tasks & VersionFileOperation::Replace)
    {
        // The current file will be replaced. Remove hint at file path (file path will be a different image)
        d->image.getImageHistory().purgePathFromReferredImages(op.saveFile.path, op.saveFile.fileName);
    }

    QMap<int, VersionFileInfo>::const_iterator it;

    for (it = op.intermediates.begin(); it != op.intermediates.end(); ++it)
    {
        FileToSave file;
        file.fileName              = it.value().fileName;
        file.filePath              = it.value().filePath();
        file.intendedFilePath      = file.filePath;
        file.mimeType              = it.value().format;
        file.ioAttributes          = ioAttributes(iofileSettings, it.value().format);
        file.historyStep           = it.key();
        file.setExifOrientationTag = setExifOrientationTag;
        file.image                 = d->image.copyMetaData();
        d->filesToSave << file;
        kDebug() << "Saving intermediate at history step" << file.historyStep
                 << "to" << file.filePath << "(" << file.mimeType << ")";
    }

    // This shall be the last in the list. If not, adjust slotImageSaved
    FileToSave primary;
    primary.fileName              = op.saveFile.fileName;
    primary.filePath              = filePath; // can be temporary file path
    primary.intendedFilePath      = intendedFilePath;
    primary.mimeType              = mimeType;
    primary.ioAttributes          = ioAttributes(iofileSettings, mimeType);
    primary.historyStep           = -1; // special value
    primary.setExifOrientationTag = setExifOrientationTag;
    primary.image                 = d->image;
    d->filesToSave << primary;

    kDebug() << "Saving to :" << primary.filePath << "(" << primary.mimeType << ")";

    saveNext();
}

void DImgInterface::saveNext()
{
    if (d->filesToSave.isEmpty() || d->currentFileToSave >= d->filesToSave.size())
    {
        return;
    }

    FileToSave& file = d->filesToSave[d->currentFileToSave];
    kDebug() << "Saving file" << file.filePath << "at" << file.historyStep;

    if (file.historyStep != -1)
    {
        // intermediate. Need to get data from undo manager
        int currentStep = getImageHistory().size() - 1;
        //kDebug() << "Requesting from undo manager data" << currentStep - file.historyStep << "steps back";
        d->undoMan->putImageDataAndHistory(&file.image, currentStep - file.historyStep);
    }

    QMap<QString, QVariant>::const_iterator it;

    for (it = file.ioAttributes.constBegin(); it != file.ioAttributes.constEnd(); ++it)
    {
        file.image.setAttribute(it.key(), it.value());
    }

    file.image.prepareMetadataToSave(file.intendedFilePath, file.mimeType,
                                     file.setExifOrientationTag);
    //kDebug() << "Adjusting image" << file.mimeType << file.fileName << file.setExifOrientationTag << file.ioAttributes
    //       << "image:" << file.image.size() << file.image.isNull();

    d->thread->save(file.image, file.filePath, file.mimeType);
}

void DImgInterface::slotImageSaved(const QString& filePath, bool success)
{
    if (d->filesToSave.isEmpty() || d->filesToSave[d->currentFileToSave].filePath != filePath)
    {
        return;
    }

    FileToSave& savedFile = d->filesToSave[d->currentFileToSave];

    if (success)
    {
        if (savedFile.historyStep == -1)
        {
            // Note: We operate on a temp file here, so we cannot
            // add it as referred image yet. Done in addLastSavedToHistory
            LoadingDescription description(filePath, LoadingDescription::ConvertForEditor);
            d->currentDescription = description;
        }
        else
        {
            HistoryImageId id = savedFile.image.addAsReferredImage(filePath);

            // for all images following in history, we need to insert the now saved file at the right place
            for (int i = d->currentFileToSave + 1; i < d->filesToSave.size(); ++i)
            {
                d->filesToSave[i].image.insertAsReferredImage(savedFile.historyStep, id);
            }
        }
    }
    else
    {
        kWarning() << "error saving image '" << QFile::encodeName(filePath).data();
    }

    d->currentFileToSave++;

    if (d->currentFileToSave == d->filesToSave.size())
    {
        d->filesToSave.clear();
        emit signalImageSaved(filePath, success);
    }
    else
    {
        saveNext();
    }
}

void DImgInterface::slotSavingProgress(const QString& filePath, float progress)
{
    if (!d->filesToSave.isEmpty() && d->filesToSave.at(d->currentFileToSave).filePath == filePath)
    {
        emit signalSavingProgress(filePath, progress);
    }
}

void DImgInterface::abortSaving()
{
    // failure will be reported by a signal
    if (!d->filesToSave.isEmpty())
    {
        d->thread->stopSaving(d->filesToSave.at(d->currentFileToSave).filePath);
        d->filesToSave.clear();
    }
}

QString DImgInterface::ensureHasCurrentUuid() const
{
    /*
     * 1) An image is loaded. The DImgLoader adds the HistoryImageId of the loaded file as "Current" entry.
     * 2) The loaded image has no UUID (created by camera etc.). Highler level calls ensureHasCurrentUuid
     *    before any saving is started
     * 3) We create a new UUID and add it to the image's history. When the new image is saved,
     *    it references the original by UUID. Because we, here, do not touch the original,
     *    it is out of scope to add the UUID to the original file's metadata.
     *    Higher level is responsible for this.
     * 4) When the image is saved, DImg::updateMetadata will create a new UUID for the saved
     *    image, which is then of course written to the newly saved file.
     */
    if (!d->image.getImageHistory().currentReferredImage().hasUuid())
    {
        // if there is no uuid in the image, we create one.
        QString uuid = d->image.createImageUniqueId();
        d->image.addCurrentUniqueImageId(uuid);
    }

    return d->image.getImageHistory().currentReferredImage().uuid();
}

void DImgInterface::provideCurrentUuid(const QString& uuid)
{
    // If the (original) image did not yet have a UUID, one is provided by higher level
    // Higher level decides how this UUID is stored; we dont touch the original here.
    if (!d->image.getImageHistory().currentReferredImage().hasUuid())
    {
        d->image.addCurrentUniqueImageId(uuid);
    }
}

void DImgInterface::setLastSaved(const QString& filePath)
{
    if (getImageFilePath() == filePath)
    {
        // if the file was overwritten, a complete undo, to the state of original loading,
        // does not return to a real image anymore - it's overwritten
        d->undoMan->clearPreviousOriginData();
    }

    // We cannot do it in slotImageSaved because we may operate on a temporary filePath.
    d->image.imageSavedAs(filePath);
}

void DImgInterface::switchToLastSaved(const DImageHistory& resolvedCurrentHistory)
{
    // Higher level wants to use the current DImg object to represent the file
    // it has previously been saved to.
    // setLastSaved shall have been called before.
    d->image.switchOriginToLastSaved();

    if (resolvedCurrentHistory.isNull())
    {
        d->resolvedInitialHistory = d->image.getOriginalImageHistory();
        d->resolvedInitialHistory.clearReferredImages();
    }
    else
    {
        d->resolvedInitialHistory = resolvedCurrentHistory;
    }

    setUndoManagerOrigin();
}

void DImgInterface::setHistoryIsBranch(bool isBranching)
{
    // The first added step (on top of the initial history) will be marked as branch
    d->image.setHistoryBranchAfter(d->resolvedInitialHistory, isBranching);
}

void DImgInterface::setModified()
{
    emit signalModified();
    emit signalUndoStateChanged();
}

void DImgInterface::readMetadataFromFile(const QString& file)
{
    DMetadata meta(file);
    // This can overwrite metadata changes introduced by imageplugins.
    // Currently, this is ProfileConversion and lensfun.
    // ProfileConversion's changes is redone when saving by DImgLoader.
    // Lensfun is not critical.
    // For a clean solution, we'd need to record a sort of metadata changeset in UndoMetadataContainer.
    d->image.setMetadata(meta.data());
    // If we are editing, and someone else at the same time, there's nothing we can do.
    if (!d->undoMan->hasChanges())
    {
        d->image.setImageHistory(DImageHistory::fromXml(meta.getImageHistory()));
    }
}

void DImgInterface::clearUndoManager()
{
    d->undoMan->clear();
    d->undoMan->setOrigin();
    emit signalUndoStateChanged();
}

void DImgInterface::setUndoManagerOrigin()
{
    d->undoMan->setOrigin();
    emit signalUndoStateChanged();
    emit signalFileOriginChanged(getImageFilePath());
}

bool DImgInterface::imageValid() const
{
    return d->valid;
}

int DImgInterface::width() const
{
    return d->width;
}

int DImgInterface::height() const
{
    return d->height;
}

int DImgInterface::origWidth() const
{
    return d->origWidth;
}

int DImgInterface::origHeight() const
{
    return d->origHeight;
}

int DImgInterface::bytesDepth() const
{
    return d->image.bytesDepth();
}

bool DImgInterface::sixteenBit() const
{
    return d->image.sixteenBit();
}

bool DImgInterface::hasAlpha() const
{
    return d->image.hasAlpha();
}

bool DImgInterface::isReadOnly() const
{
    if (d->image.isNull())
    {
        return true;
    }
    else
    {
        return d->image.isReadOnly();
    }
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

void DImgInterface::paintOnDevice(QPaintDevice* const p,
                                  int sx, int sy, int sw, int sh,
                                  int dx, int dy, int dw, int dh,
                                  int /*antialias*/)
{
    if (d->image.isNull())
    {
        return;
    }

    DImg img = d->image.smoothScaleSection(sx, sy, sw, sh, dw, dh);
    img.convertDepth(32);
    QPainter painter(p);

    if (d->cmSettings.enableCM && (d->cmSettings.useManagedView || d->doSoftProofing))
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
        QImage pureColorMask = img.pureColorMask(d->expoSettings);
        QPixmap pixMask      = QPixmap::fromImage(pureColorMask);
        painter.drawPixmap(dx, dy, pixMask, 0, 0, pixMask.width(), pixMask.height());
    }

    painter.end();
}

void DImgInterface::paintOnDevice(QPaintDevice* const p,
                                  int sx, int sy, int sw, int sh,
                                  int dx, int dy, int dw, int dh,
                                  int mx, int my, int mw, int mh,
                                  int /*antialias*/)
{
    if (d->image.isNull())
    {
        return;
    }

    DImg img = d->image.smoothScaleSection(sx, sy, sw, sh, dw, dh);
    img.convertDepth(32);
    QPainter painter(p);

    uint* data  = (uint*)img.bits();
    uchar r, g, b, a;

    for (int j = 0; j < (int)img.height(); ++j)
    {
        for (int i = 0; i < (int)img.width(); ++i)
        {
            if (i < (mx - dx) || i > (mx - dx + mw - 1) ||
                j < (my - dy) || j > (my - dy + mh - 1))
            {
                a = (*data >> 24) & 0xff;
                r = (*data >> 16) & 0xff;
                g = (*data >>  8) & 0xff;
                b = (*data) & 0xff;

                r += (uchar)((RCOL - r) * OPACITY);
                g += (uchar)((GCOL - g) * OPACITY);
                b += (uchar)((BCOL - b) * OPACITY);

                *data = (a << 24) | (r << 16) | (g << 8) | b;
            }

            ++data;
        }
    }

    if (d->cmSettings.enableCM && (d->cmSettings.useManagedView || d->doSoftProofing))
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
        QImage pureColorMask = img.pureColorMask(d->expoSettings);
        QPixmap pixMask      = QPixmap::fromImage(pureColorMask);
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

void DImgInterface::applyReversibleBuiltinFilter(const DImgBuiltinFilter& filter)
{
    applyBuiltinFilter(filter, new UndoActionReversible(this, filter));
}

void DImgInterface::applyBuiltinFilter(const DImgBuiltinFilter& filter, UndoAction* action)
{
    d->undoMan->addAction(action);

    filter.apply(d->image);
    d->image.addFilterAction(filter.filterAction());

    // many operations change the image size
    d->origWidth  = d->image.width();
    d->origHeight = d->image.height();

    setModified();
}

void DImgInterface::rotate90()
{
    applyReversibleBuiltinFilter(DImgBuiltinFilter(DImgBuiltinFilter::Rotate90));
}

void DImgInterface::rotate180()
{
    applyReversibleBuiltinFilter(DImgBuiltinFilter(DImgBuiltinFilter::Rotate180));
}

void DImgInterface::rotate270()
{
    applyReversibleBuiltinFilter(DImgBuiltinFilter(DImgBuiltinFilter::Rotate270));
}

void DImgInterface::flipHoriz()
{
    applyReversibleBuiltinFilter(DImgBuiltinFilter(DImgBuiltinFilter::FlipHorizontally));
}

void DImgInterface::flipVert()
{
    applyReversibleBuiltinFilter(DImgBuiltinFilter(DImgBuiltinFilter::FlipVertically));
}

void DImgInterface::crop(int x, int y, int w, int h)
{
    applyBuiltinFilter(DImgBuiltinFilter(DImgBuiltinFilter::Crop, QRect(x, y, w, h)),
                       new UndoActionIrreversible(this, "Crop"));
}

void DImgInterface::resize(int w, int h)
{
    applyBuiltinFilter(DImgBuiltinFilter(DImgBuiltinFilter::Resize, QSize(w, h)),
                       new UndoActionIrreversible(this, "Resize"));
}

void DImgInterface::convertDepth(int depth)
{
    applyBuiltinFilter(DImgBuiltinFilter(depth == 32 ? DImgBuiltinFilter::ConvertTo8Bit : DImgBuiltinFilter::ConvertTo16Bit),
                       new UndoActionIrreversible(this, "Convert Color Depth"));
}

DImg* DImgInterface::getImg() const
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

DImageHistory DImgInterface::getImageHistory() const
{
    return d->image.getImageHistory();
}

DImageHistory DImgInterface::getInitialImageHistory() const
{
    return d->image.getOriginalImageHistory();
}

DImageHistory DImgInterface::getImageHistoryOfFullRedo() const
{
    return d->undoMan->getImageHistoryOfFullRedo();
}

DImageHistory DImgInterface::getResolvedInitialHistory() const
{
    return d->resolvedInitialHistory;
}

void DImgInterface::setResolvedInitialHistory(const DImageHistory& history)
{
    d->resolvedInitialHistory = history;
}

void DImgInterface::putImage(const QString& caller, const FilterAction& action, uchar* const data, int w, int h)
{
    putImage(caller, action, data, w, h, d->image.sixteenBit());
}

void DImgInterface::putImage(const QString& caller, const FilterAction& action,
                             uchar* const data, int w, int h, bool sixteenBit)
{
    d->undoMan->addAction(new UndoActionIrreversible(this, caller));
    putImageData(data, w, h, sixteenBit);
    d->image.addFilterAction(action);
    setModified();
}

void DImgInterface::putImageData(uchar* const data, int w, int h, bool sixteenBit)
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

    d->image.putImageData(w, h, sixteenBit, d->image.hasAlpha(), data);
}

void DImgInterface::setUndoImageData(const UndoMetadataContainer& c,
                                     uchar* const data, int w, int h, bool sixteenBit)
{
    // called from UndoManager
    bool changesIcc = c.changesIccProfile(d->image);

    putImageData(data, w, h, sixteenBit);
    c.toImage(d->image);

    if (changesIcc)
    {
        updateColorManagement();
    }
}

void DImgInterface::imageUndoChanged(const UndoMetadataContainer& c)
{
    // called from UndoManager
    bool changesIcc = c.changesIccProfile(d->image);

    d->origWidth  = d->image.width();
    d->origHeight = d->image.height();
    c.toImage(d->image);

    if (changesIcc)
    {
        updateColorManagement();
    }
}

void DImgInterface::setFileOriginData(const QVariant& data)
{
    d->image.setFileOriginData(data);
    emit signalFileOriginChanged(getImageFilePath());
}

uchar* DImgInterface::getImageSelection() const
{
    if (!d->selW || !d->selH)
    {
        return 0;
    }

    if (!d->image.isNull())
    {
        DImg im = d->image.copy(d->selX, d->selY, d->selW, d->selH);
        return im.stripImageData();
    }

    return 0;
}

void DImgInterface::putImageSelection(const QString& caller, const FilterAction& action, uchar* const data)
{
    if (!data || d->image.isNull())
    {
        return;
    }

    d->undoMan->addAction(new UndoActionIrreversible(this, caller));

    d->image.bitBltImage(data, 0, 0, d->selW, d->selH, d->selX, d->selY, d->selW, d->selH, d->image.bytesDepth());

    d->image.addFilterAction(action);
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

QStringList DImgInterface::getUndoHistory() const
{
    return d->undoMan->getUndoHistory();
}

QStringList DImgInterface::getRedoHistory() const
{
    return d->undoMan->getRedoHistory();
}

int DImgInterface::availableUndoSteps() const
{
    return d->undoMan->availableUndoSteps();
}

int DImgInterface::availableRedoSteps() const
{
    return d->undoMan->availableRedoSteps();
}

IccProfile DImgInterface::getEmbeddedICC() const
{
    return d->image.getIccProfile();
}

KExiv2Data DImgInterface::getMetadata() const
{
    return d->image.getMetadata();
}

QString DImgInterface::getImageFilePath() const
{
    return d->image.originalFilePath();
}

QString DImgInterface::getImageFileName() const
{
    return getImageFilePath().section('/', -1);
}

QString DImgInterface::getImageFormat() const
{
    if (d->image.isNull())
    {
        return QString();
    }

    QString mimeType = d->image.format();

    // It is a bug in the loader if format attribute is not given
    if (mimeType.isEmpty())
    {
        kWarning() << "DImg object does not contain attribute \"format\"";
        mimeType = QImageReader::imageFormat(getImageFilePath());
    }

    return mimeType;
}

QPixmap DImgInterface::convertToPixmap(DImg& img) const
{
    QPixmap pix;

    if (d->cmSettings.enableCM && (d->cmSettings.useManagedView || d->doSoftProofing))
    {
        // do not use d->monitorICCtrans here, because img may have a different embedded profile
        IccManager manager(img);
        IccTransform transform;

        if (d->doSoftProofing)
        {
            transform = manager.displaySoftProofingTransform(d->cmSettings.defaultProofProfile, d->displayingWidget);
        }
        else
        {
            transform = manager.displayTransform(d->displayingWidget);
        }

        pix = img.convertToPixmap(transform);
    }
    else
    {
        pix = img.convertToPixmap();
    }

    // Show the Over/Under exposure pixels indicators

    if (d->expoSettings->underExposureIndicator || d->expoSettings->overExposureIndicator)
    {
        QPainter painter(&pix);
        QImage pureColorMask = img.pureColorMask(d->expoSettings);
        QPixmap pixMask      = QPixmap::fromImage(pureColorMask);
        painter.drawPixmap(0, 0, pixMask, 0, 0, pixMask.width(), pixMask.height());
    }

    return pix;
}

}  // namespace Digikam
