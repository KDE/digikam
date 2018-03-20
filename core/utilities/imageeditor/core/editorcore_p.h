/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-01-15
 * Description : DImg interface for image editor
 *
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

#ifndef EDITORCORE_P_H
#define EDITORCORE_P_H

namespace Digikam
{

class UndoManager;

class EditorCore::Private
{

public:

    class FileToSave
    {
    public:

        bool                    setExifOrientationTag;
        int                     historyStep;

        QString                 fileName;
        QString                 filePath;
        QString                 intendedFilePath;
        QString                 mimeType;
        QMap<QString, QVariant> ioAttributes;
        DImg                    image;
    };

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
        zoom(1.0),
        displayingWidget(0),
        currentFileToSave(0),
        undoMan(0),
        expoSettings(0),
        thread(0)
    {
    }

    QMap<QString, QVariant> ioAttributes(IOFileSettings* const iofileSettings, const QString& givenMimeType) const;

    void applyBuiltinFilter(const DImgBuiltinFilter& filter, UndoAction* const action);
    void applyReversibleBuiltinFilter(const DImgBuiltinFilter& filter);
    void putImageData(uchar* const data, int w, int h, bool sixteenBit);
    void resetValues();
    void saveNext();
    void loadCurrent();
    void load(const LoadingDescription& description);
    void saveAs(const QString& file, IOFileSettings* const iofileSettings,
                bool setExifOrientationTag, const QString& givenMimeType,
                const VersionFileOperation& operation, const QString& intendedFilePath);

public:

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
};

void EditorCore::Private::putImageData(uchar* const data, int w, int h, bool sixteenBit)
{
    if (image.isNull())
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "d->image is NULL";
        return;
    }

    if (!data)
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "New image is NULL";
        return;
    }

    if (w == -1 && h == -1)
    {
        // New image size
        w = origWidth;
        h = origHeight;
    }
    else
    {
        // New image size == original size
        origWidth  = w;
        origHeight = h;
    }

    image.putImageData(w, h, sixteenBit, image.hasAlpha(), data);
    image.setAttribute(QLatin1String("originalSize"), image.size());
}

void EditorCore::Private::resetValues()
{
    valid                  = false;
    currentDescription     = LoadingDescription();
    width                  = 0;
    height                 = 0;
    origWidth              = 0;
    origHeight             = 0;
    selX                   = 0;
    selY                   = 0;
    selW                   = 0;
    selH                   = 0;
    resolvedInitialHistory = DImageHistory();
    undoMan->clear();
}

void EditorCore::Private::saveNext()
{
    if (filesToSave.isEmpty() || currentFileToSave >= filesToSave.size())
    {
        return;
    }

    FileToSave& file = filesToSave[currentFileToSave];
    qCDebug(DIGIKAM_GENERAL_LOG) << "Saving file" << file.filePath << "at" << file.historyStep;

    if (file.historyStep != -1)
    {
        // intermediate. Need to get data from undo manager
        int currentStep = EditorCore::defaultInstance()->getImageHistory().size() - 1;
        //qCDebug(DIGIKAM_GENERAL_LOG) << "Requesting from undo manager data" << currentStep - file.historyStep << "steps back";
        undoMan->putImageDataAndHistory(&file.image, currentStep - file.historyStep);
    }

    QMap<QString, QVariant>::const_iterator it;

    for (it = file.ioAttributes.constBegin(); it != file.ioAttributes.constEnd(); ++it)
    {
        file.image.setAttribute(it.key(), it.value());
    }

    file.image.prepareMetadataToSave(file.intendedFilePath, file.mimeType, file.setExifOrientationTag);
    //qCDebug(DIGIKAM_GENERAL_LOG) << "Adjusting image" << file.mimeType << file.fileName << file.setExifOrientationTag << file.ioAttributes
    //         << "image:" << file.image.size() << file.image.isNull();

    thread->save(file.image, file.filePath, file.mimeType);
}

void EditorCore::Private::applyReversibleBuiltinFilter(const DImgBuiltinFilter& filter)
{
    applyBuiltinFilter(filter, new UndoActionReversible(EditorCore::defaultInstance(), filter));
}

void EditorCore::Private::applyBuiltinFilter(const DImgBuiltinFilter& filter, UndoAction* const action)
{
    undoMan->addAction(action);

    filter.apply(image);
    image.addFilterAction(filter.filterAction());

    // many operations change the image size
    origWidth  = image.width();
    origHeight = image.height();
    width      = origWidth;
    height     = origHeight;

    image.setAttribute(QLatin1String("originalSize"), image.size());
    EditorCore::defaultInstance()->setModified();
}

QMap<QString, QVariant> EditorCore::Private::ioAttributes(IOFileSettings* const iofileSettings,
                                                          const QString& mimeType) const
{
    QMap<QString, QVariant> attributes;

    // JPEG file format.
    if (mimeType.toUpper() == QLatin1String("JPG") || mimeType.toUpper() == QLatin1String("JPEG") ||
        mimeType.toUpper() == QLatin1String("JPE"))
    {
        attributes.insert(QLatin1String("quality"),     iofileSettings->JPEGCompression);
        attributes.insert(QLatin1String("subsampling"), iofileSettings->JPEGSubSampling);
    }

    // PNG file format.
    if (mimeType.toUpper() == QLatin1String("PNG"))
    {
        attributes.insert(QLatin1String("quality"), iofileSettings->PNGCompression);
    }

    // TIFF file format.
    if (mimeType.toUpper() == QLatin1String("TIFF") || mimeType.toUpper() == QLatin1String("TIF"))
    {
        attributes.insert(QLatin1String("compress"), iofileSettings->TIFFCompression);
    }

    // JPEG 2000 file format.
    if (mimeType.toUpper() == QLatin1String("JP2") || mimeType.toUpper() == QLatin1String("JPX") ||
        mimeType.toUpper() == QLatin1String("JPC") || mimeType.toUpper() == QLatin1String("PGX") ||
        mimeType.toUpper() == QLatin1String("J2K"))
    {
        if (iofileSettings->JPEG2000LossLess)
        {
            attributes.insert(QLatin1String("quality"), 100);    // LossLess compression
        }
        else
        {
            attributes.insert(QLatin1String("quality"), iofileSettings->JPEG2000Compression);
        }
    }

    // PGF file format.
    if (mimeType.toUpper() == QLatin1String("PGF"))
    {
        if (iofileSettings->PGFLossLess)
        {
            attributes.insert(QLatin1String("quality"), 0);    // LossLess compression
        }
        else
        {
            attributes.insert(QLatin1String("quality"), iofileSettings->PGFCompression);
        }
    }

    return attributes;
}

void EditorCore::Private::saveAs(const QString& filePath, IOFileSettings* const iofileSettings,
                                 bool setExifOrientationTag, const QString& givenMimeType,
                                 const VersionFileOperation& op, const QString& intendedFilePath)
{
    // No need to toggle off undo, redo or save action during saving using
    // signalUndoStateChanged(), this is will done by GUI implementation directly.

    EditorCore::defaultInstance()->emit signalSavingStarted(filePath);

    filesToSave.clear();
    currentFileToSave = 0;

    QString mimeType = givenMimeType;

    // This is possibly empty
    if (mimeType.isEmpty())
    {
        mimeType = EditorCore::defaultInstance()->getImageFormat();
    }

    if (op.tasks & VersionFileOperation::MoveToIntermediate ||
        op.tasks & VersionFileOperation::SaveAndDelete)
    {
        // The current file will stored away at a different name. Adjust history.
        image.getImageHistory().moveCurrentReferredImage(op.intermediateForLoadedFile.path,
                                                         op.intermediateForLoadedFile.fileName);
    }

    if (op.tasks & VersionFileOperation::Replace)
    {
        // The current file will be replaced. Remove hint at file path (file path will be a different image)
        image.getImageHistory().purgePathFromReferredImages(op.saveFile.path, op.saveFile.fileName);
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
        file.image                 = image.copyMetaData();
        filesToSave << file;
        qCDebug(DIGIKAM_GENERAL_LOG) << "Saving intermediate at history step" << file.historyStep
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
    primary.image                 = image;
    filesToSave << primary;

    qCDebug(DIGIKAM_GENERAL_LOG) << "Saving to :" << primary.filePath << "(" << primary.mimeType << ")";

    saveNext();
}

void EditorCore::Private::loadCurrent()
{
    thread->load(currentDescription,
                 SharedLoadSaveThread::AccessModeReadWrite,
                 SharedLoadSaveThread::LoadingPolicyFirstRemovePrevious);

    EditorCore::defaultInstance()->emit signalLoadingStarted(currentDescription.filePath);
}

void EditorCore::Private::load(const LoadingDescription& description)
{
    if (EditorToolIface::editorToolIface())
    {
        EditorToolIface::editorToolIface()->unLoadTool();
    }

    if (description != currentDescription)
    {
        resetValues();
        currentDescription = description;
        loadCurrent();
    }
    else
    {
        EditorCore::defaultInstance()->emit signalLoadingStarted(currentDescription.filePath);
        EditorCore::defaultInstance()->emit signalImageLoaded(currentDescription.filePath, true);
    }
}

}  // namespace Digikam

#endif // EDITORCORE_P_H
