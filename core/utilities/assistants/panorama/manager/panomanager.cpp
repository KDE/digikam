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

#include "panomanager.h"

// Qt includes

#include <QFile>
#include <QSharedPointer>

// KDE includes

#include <kconfig.h>
#include <kconfiggroup.h>

// Local includes

#include "digikam_debug.h"
#include "panowizard.h"
#include "panoactionthread.h"
#include "autooptimiserbinary.h"
#include "cpcleanbinary.h"
#include "cpfindbinary.h"
#include "enblendbinary.h"
#include "makebinary.h"
#include "nonabinary.h"
#include "panomodifybinary.h"
#include "pto2mkbinary.h"
#include "huginexecutorbinary.h"
#include "ptofile.h"

namespace Digikam
{

class PanoManager::Private
{
public:

    explicit Private()
      : basePtoData(0),
        cpFindPtoData(0),
        cpCleanPtoData(0),
        autoOptimisePtoData(0),
        viewAndCropOptimisePtoData(0),
        previewPtoData(0),
        panoPtoData(0),
        hugin2015(false),
        thread(0),
        wizard(0),
        group(config.group("Panorama Settings"))
    {
        gPano    = group.readEntry("GPano", false);
//      hdr      = group.readEntry("HDR", false);
        fileType = (PanoramaFileType) group.readEntry("File Type", (int) JPEG);
    }

    ~Private()
    {
//      group.writeEntry("HDR", hdr);
        group.writeEntry("GPano", gPano);
        group.writeEntry("File Type", (int) fileType);
        config.sync();
    }

    QList<QUrl>                    inputUrls;

    QUrl                           basePtoUrl;
    QSharedPointer<PTOType>        basePtoData;
    QUrl                           cpFindPtoUrl;
    QSharedPointer<PTOType>        cpFindPtoData;
    QUrl                           cpCleanPtoUrl;
    QSharedPointer<PTOType>        cpCleanPtoData;
    QUrl                           autoOptimisePtoUrl;
    QSharedPointer<PTOType>        autoOptimisePtoData;
    QUrl                           viewAndCropOptimisePtoUrl;
    QSharedPointer<PTOType>        viewAndCropOptimisePtoData;
    QUrl                           previewPtoUrl;
    QSharedPointer<PTOType>        previewPtoData;
    QUrl                           panoPtoUrl;
    QSharedPointer<PTOType>        panoPtoData;

    QUrl                           previewMkUrl;
    QUrl                           previewUrl;
    QUrl                           mkUrl;
    QUrl                           panoUrl;

    bool                           hugin2015;
    bool                           gPano;
//     bool                           hdr;

    PanoramaFileType               fileType;

    PanoramaItemUrlsMap            preProcessedUrlsMap;

    PanoActionThread*              thread;

    AutoOptimiserBinary            autoOptimiserBinary;
    CPCleanBinary                  cpCleanBinary;
    CPFindBinary                   cpFindBinary;
    EnblendBinary                  enblendBinary;
    MakeBinary                     makeBinary;
    NonaBinary                     nonaBinary;
    PanoModifyBinary               panoModifyBinary;
    Pto2MkBinary                   pto2MkBinary;
    HuginExecutorBinary            huginExecutorBinary;

    PanoWizard*                    wizard;

private:

    KConfig                        config;
    KConfigGroup                   group;
};

QPointer<PanoManager> PanoManager::internalPtr = QPointer<PanoManager>();

PanoManager::PanoManager(QObject* const parent)
    : QObject(parent),
      d(new Private)
{
    d->thread = new PanoActionThread(this);
}

PanoManager::~PanoManager()
{
    delete d->thread;
    delete d->wizard;
    delete d;
}

PanoManager* PanoManager::instance()
{
    if (PanoManager::internalPtr.isNull())
    {
        PanoManager::internalPtr = new PanoManager();
    }

    return PanoManager::internalPtr;
}

bool PanoManager::isCreated()
{
    return (!internalPtr.isNull());
}

void PanoManager::checkForHugin2015()
{
    if (d->autoOptimiserBinary.recheckDirectories())
    {
        d->hugin2015 = d->autoOptimiserBinary.versionIsRight(2015.0);
    }

    qCDebug(DIGIKAM_GENERAL_LOG) << "Hugin >= 2015.0 : " << d->hugin2015;
}

bool PanoManager::checkBinaries()
{
    bool result = d->autoOptimiserBinary.recheckDirectories() &&
                  d->cpCleanBinary.recheckDirectories()       &&
                  d->cpFindBinary.recheckDirectories()        &&
                  d->enblendBinary.recheckDirectories()       &&
                  d->makeBinary.recheckDirectories()          &&
                  d->nonaBinary.recheckDirectories();

    if (result)
    {
        if (d->hugin2015)
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "Check for Hugin Executor";
            result = d->huginExecutorBinary.recheckDirectories();
        }
        else
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "Check for Hugin Pto2Mk";
            result = d->pto2MkBinary.recheckDirectories();
        }
    }

    return result;
}

bool PanoManager::hugin2015() const
{
    return d->hugin2015;
}

void PanoManager::setGPano(bool gPano)
{
    d->gPano = gPano;
}

bool PanoManager::gPano() const
{
    return d->gPano;
}

// void PanoManager::setHDR(bool hdr)
// {
//     d->hdr = hdr;
// }
//
// bool PanoManager::hdr() const
// {
//     return d->hdr;
// }

void PanoManager::setFileFormatJPEG()
{
    d->fileType = JPEG;
}

void PanoManager::setFileFormatTIFF()
{
    d->fileType = TIFF;
}

void PanoManager::setFileFormatHDR()
{
    d->fileType = HDR;
}

PanoramaFileType PanoManager::format() const
{
    return d->fileType;
}

AutoOptimiserBinary& PanoManager::autoOptimiserBinary() const
{
    return d->autoOptimiserBinary;
}

CPCleanBinary& PanoManager::cpCleanBinary() const
{
    return d->cpCleanBinary;
}

CPFindBinary& PanoManager::cpFindBinary() const
{
    return d->cpFindBinary;
}

EnblendBinary& PanoManager::enblendBinary() const
{
    return d->enblendBinary;
}

MakeBinary& PanoManager::makeBinary() const
{
    return d->makeBinary;
}

NonaBinary& PanoManager::nonaBinary() const
{
    return d->nonaBinary;
}

PanoModifyBinary& PanoManager::panoModifyBinary() const
{
    return d->panoModifyBinary;
}

Pto2MkBinary& PanoManager::pto2MkBinary() const
{
    return d->pto2MkBinary;
}

HuginExecutorBinary& PanoManager::huginExecutorBinary() const
{
    return d->huginExecutorBinary;
}

QUrl& PanoManager::basePtoUrl() const
{
    return d->basePtoUrl;
}

QSharedPointer<PTOType> PanoManager::basePtoData()
{
    if (d->basePtoData.isNull())
    {
        PTOFile file(cpFindBinary().version());
        file.openFile(d->basePtoUrl.toLocalFile());
        d->basePtoData = QSharedPointer<PTOType>(file.getPTO());

        if (d->basePtoData.isNull())
            d->basePtoData = QSharedPointer<PTOType>(new PTOType(cpFindBinary().version()));
    }

    return d->basePtoData;
}

void PanoManager::resetBasePto()
{
    d->basePtoData.clear();

    QFile pto(d->basePtoUrl.toLocalFile());

    if (pto.exists())
    {
        pto.remove();
    }

    d->basePtoUrl.clear();
}

QUrl& PanoManager::cpFindPtoUrl() const
{
    return d->cpFindPtoUrl;
}

QSharedPointer<PTOType> PanoManager::cpFindPtoData()
{
    if (d->cpFindPtoData.isNull())
    {
        PTOFile file(cpFindBinary().version());
        file.openFile(d->cpFindPtoUrl.toLocalFile());
        d->cpFindPtoData = QSharedPointer<PTOType>(file.getPTO());

        if (d->cpFindPtoData.isNull())
            d->cpFindPtoData = QSharedPointer<PTOType>(new PTOType(cpFindBinary().version()));
    }

    return d->cpFindPtoData;
}

void PanoManager::resetCpFindPto()
{
    d->cpFindPtoData.clear();

    QFile pto(d->cpFindPtoUrl.toLocalFile());

    if (pto.exists())
    {
        pto.remove();
    }

    d->cpFindPtoUrl.clear();
}

QUrl& PanoManager::cpCleanPtoUrl() const
{
    return d->cpCleanPtoUrl;
}

QSharedPointer<PTOType> PanoManager::cpCleanPtoData()
{
    if (d->cpCleanPtoData.isNull())
    {
        PTOFile file(cpFindBinary().version());
        file.openFile(d->cpCleanPtoUrl.toLocalFile());
        d->cpCleanPtoData = QSharedPointer<PTOType>(file.getPTO());

        if (d->cpCleanPtoData.isNull())
            d->cpCleanPtoData = QSharedPointer<PTOType>(new PTOType(cpFindBinary().version()));
    }

    return d->cpCleanPtoData;
}

void PanoManager::resetCpCleanPto()
{
    d->cpCleanPtoData.clear();

    QFile pto(d->cpCleanPtoUrl.toLocalFile());

    if (pto.exists())
    {
        pto.remove();
    }

    d->cpCleanPtoUrl.clear();
}

QUrl& PanoManager::autoOptimisePtoUrl() const
{
    return d->autoOptimisePtoUrl;
}

QSharedPointer<PTOType> PanoManager::autoOptimisePtoData()
{
    if (d->autoOptimisePtoData.isNull())
    {
        PTOFile file(cpFindBinary().version());
        file.openFile(d->autoOptimisePtoUrl.toLocalFile());
        d->autoOptimisePtoData = QSharedPointer<PTOType>(file.getPTO());

        if (d->autoOptimisePtoData.isNull())
            d->autoOptimisePtoData = QSharedPointer<PTOType>(new PTOType(cpFindBinary().version()));
    }

    return d->autoOptimisePtoData;
}

void PanoManager::resetAutoOptimisePto()
{
    d->autoOptimisePtoData.clear();

    QFile pto(d->autoOptimisePtoUrl.toLocalFile());

    if (pto.exists())
    {
        pto.remove();
    }

    d->autoOptimisePtoUrl.clear();
}

QUrl& PanoManager::viewAndCropOptimisePtoUrl() const
{
    return d->viewAndCropOptimisePtoUrl;
}

QSharedPointer<PTOType> PanoManager::viewAndCropOptimisePtoData()
{
    if (d->viewAndCropOptimisePtoData.isNull())
    {
        PTOFile file(cpFindBinary().version());
        file.openFile(d->viewAndCropOptimisePtoUrl.toLocalFile());
        d->viewAndCropOptimisePtoData = QSharedPointer<PTOType>(file.getPTO());

        if (d->viewAndCropOptimisePtoData.isNull())
            d->viewAndCropOptimisePtoData = QSharedPointer<PTOType>(new PTOType(cpFindBinary().version()));
    }

    return d->viewAndCropOptimisePtoData;
}

void PanoManager::resetViewAndCropOptimisePto()
{
    d->viewAndCropOptimisePtoData.clear();

    QFile pto(d->viewAndCropOptimisePtoUrl.toLocalFile());

    if (pto.exists())
    {
        pto.remove();
    }

    d->viewAndCropOptimisePtoUrl.clear();
}

QUrl& PanoManager::previewPtoUrl() const
{
    return d->previewPtoUrl;
}

QSharedPointer<PTOType> PanoManager::previewPtoData()
{
    if (d->previewPtoData.isNull())
    {
        PTOFile file(cpFindBinary().version());
        file.openFile(d->previewPtoUrl.toLocalFile());
        d->previewPtoData = QSharedPointer<PTOType>(file.getPTO());

        if (d->previewPtoData.isNull())
            d->previewPtoData = QSharedPointer<PTOType>(new PTOType(cpFindBinary().version()));
    }

    return d->previewPtoData;
}

void PanoManager::resetPreviewPto()
{
    d->previewPtoData.clear();

    QFile pto(d->previewPtoUrl.toLocalFile());

    if (pto.exists())
    {
        pto.remove();
    }

    d->previewPtoUrl.clear();
}

QUrl& PanoManager::panoPtoUrl() const
{
    return d->panoPtoUrl;
}

QSharedPointer<PTOType> PanoManager::panoPtoData()
{
    if (d->panoPtoData.isNull())
    {
        PTOFile file(cpFindBinary().version());
        file.openFile(d->panoPtoUrl.toLocalFile());
        d->panoPtoData = QSharedPointer<PTOType>(file.getPTO());

        if (d->panoPtoData.isNull())
            d->panoPtoData = QSharedPointer<PTOType>(new PTOType(cpFindBinary().version()));
    }

    return d->panoPtoData;
}

void PanoManager::resetPanoPto()
{
    d->panoPtoData.clear();

    QFile pto(d->panoPtoUrl.toLocalFile());

    if (pto.exists())
    {
        pto.remove();
    }

    d->panoPtoUrl.clear();
}

QUrl& PanoManager::previewMkUrl() const
{
    return d->previewMkUrl;
}

void PanoManager::resetPreviewMkUrl()
{
    QFile pto(d->previewMkUrl.toLocalFile());

    if (pto.exists())
    {
        pto.remove();
    }

    d->previewMkUrl.clear();
}

QUrl& PanoManager::previewUrl() const
{
    return d->previewUrl;
}

void PanoManager::resetPreviewUrl()
{
    QFile pto(d->previewUrl.toLocalFile());

    if (pto.exists())
    {
        pto.remove();
    }

    d->previewUrl.clear();
}

QUrl& PanoManager::mkUrl() const
{
    return d->mkUrl;
}

void PanoManager::resetMkUrl()
{
    QFile pto(d->mkUrl.toLocalFile());

    if (pto.exists())
    {
        pto.remove();
    }

    d->mkUrl.clear();
}

QUrl& PanoManager::panoUrl() const
{
    return d->panoUrl;
}

void PanoManager::resetPanoUrl()
{
    QFile pto(d->panoUrl.toLocalFile());

    if (pto.exists())
    {
        pto.remove();
    }

    d->panoUrl.clear();
}

void PanoManager::setItemsList(const QList<QUrl>& urls)
{
    d->inputUrls = urls;
}

QList<QUrl>& PanoManager::itemsList() const
{
    return d->inputUrls;
}

void PanoManager::setPreProcessedMap(const PanoramaItemUrlsMap& urls)
{
    d->preProcessedUrlsMap = urls;
}

PanoramaItemUrlsMap& PanoManager::preProcessedMap() const
{
    return d->preProcessedUrlsMap;
}

PanoActionThread* PanoManager::thread() const
{
    return d->thread;
}

void PanoManager::run()
{
    startWizard();
}

void PanoManager::startWizard()
{
    if (d->wizard && (d->wizard->isMinimized() || !d->wizard->isHidden()))
    {
        d->wizard->showNormal();
        d->wizard->activateWindow();
        d->wizard->raise();
    }
    else
    {
        delete d->wizard;

        d->wizard = new PanoWizard(this);
        d->wizard->show();
    }
}

} // namespace Digikam
