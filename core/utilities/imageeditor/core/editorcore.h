/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-01-15
 * Description : DImg interface for image editor
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

#ifndef EDITOR_CORE_H
#define EDITOR_CORE_H

// Qt includes

#include <QObject>
#include <QString>
#include <QByteArray>

// Local includes

#include "digikam_export.h"
#include "dimg.h"
#include "dimagehistory.h"
#include "versionmanager.h"

class QWidget;
class QPixmap;

namespace Digikam
{

class UndoState;
class ICCSettingsContainer;
class ExposureSettingsContainer;
class IOFileSettings;
class LoadingDescription;
class DImgBuiltinFilter;
class UndoAction;
class UndoMetadataContainer;
class VersionFileOperation;

class DIGIKAM_EXPORT EditorCore : public QObject
{
    Q_OBJECT

public:

    static EditorCore* defaultInstance();
    static void setDefaultInstance(EditorCore* const instance);

public:

    EditorCore();
    ~EditorCore();

    void    load(const QString& filename, IOFileSettings* const iofileSettings);
    void    applyTransform(const IccTransform& transform);
    void    setSoftProofingEnabled(bool enabled);
    bool    softProofingEnabled() const;

    void                 setICCSettings(const ICCSettingsContainer& cmSettings);
    ICCSettingsContainer getICCSettings() const;

    void                       setExposureSettings(ExposureSettingsContainer* const expoSettings);
    ExposureSettingsContainer* getExposureSettings() const;

    void    setExifOrient(bool exifOrient);
    void    setDisplayingWidget(QWidget* const widget);

    void    undo();
    void    redo();
    void    restore();
    void    rollbackToOrigin();

    void    saveAs(const QString& file, IOFileSettings* const iofileSettings,
                   bool setExifOrientationTag, const QString& givenMimeType,
                   const QString& intendedFilePath);

    void    saveAs(const QString& file, IOFileSettings* const iofileSettings,
                   bool setExifOrientationTag, const QString& givenMimeType,
                   const VersionFileOperation& operation);

    void    setHistoryIsBranch(bool isBranching);
    void    setLastSaved(const QString& filePath);
    void    switchToLastSaved(const DImageHistory& resolvedCurrentHistory = DImageHistory());
    void    abortSaving();
    void    setModified();
    void    readMetadataFromFile(const QString& file);
    void    clearUndoManager();
    void    setUndoManagerOrigin();
    void    resetImage();

    QString ensureHasCurrentUuid() const;
    void    provideCurrentUuid(const QString& uuid);

    void    zoom(double val);

    /** Image properties
     */
    DImg    getImgSelection() const;
    DImg*   getImg()          const;
    bool    isValid()         const;
    bool    isReadOnly()      const;
    bool    hasAlpha()        const;
    bool    sixteenBit()      const;
    bool    exifRotated()     const;
    int     width()           const;
    int     height()          const;
    int     origWidth()       const;
    int     origHeight()      const;
    int     bytesDepth()      const;

    /** Image transforms
     */
    void    rotate90();
    void    rotate180();
    void    rotate270();
    void    flipHoriz();
    void    flipVert();
    void    convertDepth(int depth);
    void    crop(const QRect& rect);

    void    setSelectedArea(const QRect& rect);
    QRect   getSelectedArea() const;

    void    putIccProfile(const IccProfile& profile);
    void    putImgSelection(const QString& caller, const FilterAction& action, const DImg& img);
    void    putImg(const QString& caller, const FilterAction& action, const DImg& img);

    /// For internal usage by UndoManager
    void    setUndoImg(const UndoMetadataContainer& c, const DImg& img);

    void    imageUndoChanged(const UndoMetadataContainer& c);
    void    setFileOriginData(const QVariant& data);

    /** Convert a DImg image to a pixmap for screen using color
     *  managed view if necessary.
     */
    QPixmap               convertToPixmap(DImg& img)  const;

    QString               getImageFileName()          const;
    QString               getImageFilePath()          const;
    QString               getImageFormat()            const;
    QStringList           getUndoHistory()            const;
    QStringList           getRedoHistory()            const;
    UndoState             undoState()                 const;
    int                   availableUndoSteps()        const;
    int                   availableRedoSteps()        const;
    IccProfile            getEmbeddedICC()            const;
    MetaEngineData            getMetadata()               const;
    DImageHistory         getImageHistory()           const;
    DImageHistory         getInitialImageHistory()    const;
    DImageHistory         getImageHistoryOfFullRedo() const;
    DImageHistory         getResolvedInitialHistory() const;
    void                  setResolvedInitialHistory(const DImageHistory& history);

protected Q_SLOTS:

    void   slotImageLoaded(const LoadingDescription& loadingDescription, const DImg& img);
    void   slotImageSaved(const QString& filePath, bool success);
    void   slotLoadingProgress(const LoadingDescription& loadingDescription, float progress);
    void   slotSavingProgress(const QString& filePath, float progress);

Q_SIGNALS:

    void   signalModified();
    void   signalUndoStateChanged();
    void   signalFileOriginChanged(const QString& filePath);

    void   signalLoadingStarted(const QString& filename);
    void   signalLoadingProgress(const QString& filePath, float progress);
    void   signalImageLoaded(const QString& filePath, bool success);
    void   signalSavingStarted(const QString& filename);
    void   signalSavingProgress(const QString& filePath, float progress);
    void   signalImageSaved(const QString& filePath, bool success);

private Q_SLOTS:

    void slotLoadRawFromTool();
    void slotLoadRaw();

private:

    static EditorCore* m_defaultInstance;

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* EDITOR_CORE_H */
