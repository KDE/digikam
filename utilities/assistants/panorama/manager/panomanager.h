/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-05-23
 * Description : a tool to create panorama by fusion of several images.
 *
 * Copyright (C) 2011-2016 by Benjamin Girault <benjamin dot girault at gmail dot com>
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef PANO_MANAGER_H
#define PANO_MANAGER_H

// Qt includes

#include <QObject>
#include <QPointer>

// Local includes

#include "panoactions.h"
#include "ptotype.h"
#include "digikam_export.h"

namespace Digikam
{

class PanoActionThread;
class AutoOptimiserBinary;
class CPCleanBinary;
class CPFindBinary;
class EnblendBinary;
class MakeBinary;
class NonaBinary;
class PanoModifyBinary;
class Pto2MkBinary;
class HuginExecutorBinary;

class DIGIKAM_EXPORT PanoManager : public QObject
{
    Q_OBJECT

public:

    explicit PanoManager(QObject* const parent = 0);
    ~PanoManager();

public:

    static QPointer<PanoManager> internalPtr;
    static PanoManager*          instance();
    static bool                  isCreated();

public:
    
    bool checkBinaries();

    void checkForHugin2015();
    bool hugin2015() const;

    void setGPano(bool gPano);
    bool gPano() const;

//     void setHDR(bool hdr);
//     bool hdr() const;

    void setFileFormatJPEG();
    void setFileFormatTIFF();
    void setFileFormatHDR();
    PanoramaFileType format() const;

    void setItemsList(const QList<QUrl>& urls);
    QList<QUrl>& itemsList() const;

    QUrl&                   basePtoUrl() const;
    QSharedPointer<PTOType> basePtoData();
    void                    resetBasePto();
    QUrl&                   cpFindPtoUrl() const;
    QSharedPointer<PTOType> cpFindPtoData();
    void                    resetCpFindPto();
    QUrl&                   cpCleanPtoUrl() const;
    QSharedPointer<PTOType> cpCleanPtoData();
    void                    resetCpCleanPto();
    QUrl&                   autoOptimisePtoUrl() const;
    QSharedPointer<PTOType> autoOptimisePtoData();
    void                    resetAutoOptimisePto();
    QUrl&                   viewAndCropOptimisePtoUrl() const;
    QSharedPointer<PTOType> viewAndCropOptimisePtoData();
    void                    resetViewAndCropOptimisePto();
    QUrl&                   previewPtoUrl() const;
    QSharedPointer<PTOType> previewPtoData();
    void                    resetPreviewPto();
    QUrl&                   panoPtoUrl() const;
    QSharedPointer<PTOType> panoPtoData();
    void                    resetPanoPto();

    QUrl&                   previewMkUrl() const;
    void                    resetPreviewMkUrl();
    QUrl&                   previewUrl() const;
    void                    resetPreviewUrl();
    QUrl&                   mkUrl() const;
    void                    resetMkUrl();
    QUrl&                   panoUrl() const;
    void                    resetPanoUrl();

    PanoramaItemUrlsMap&    preProcessedMap()     const;
    PanoActionThread*       thread()              const;
    AutoOptimiserBinary&    autoOptimiserBinary() const;
    CPCleanBinary&          cpCleanBinary()       const;
    CPFindBinary&           cpFindBinary()        const;
    EnblendBinary&          enblendBinary()       const;
    MakeBinary&             makeBinary()          const;
    NonaBinary&             nonaBinary()          const;
    PanoModifyBinary&       panoModifyBinary()    const;
    Pto2MkBinary&           pto2MkBinary()        const;
    HuginExecutorBinary&    huginExecutorBinary() const;

    void run();

private Q_SLOTS:

    void setPreProcessedMap(const PanoramaItemUrlsMap& urls);

private:

    void startWizard();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // PANO_MANAGER_H
