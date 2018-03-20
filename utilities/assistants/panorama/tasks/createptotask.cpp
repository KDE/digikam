/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-03-15
 * Description : a tool to create panorama by fusion of several images.
 *
 * Copyright (C) 2012-2016 by Benjamin Girault <benjamin dot girault at gmail dot com>
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

#include "createptotask.h"
#include "ptotype.h"

// Qt includes

#include <QFile>

// KDE includes

#include <klocalizedstring.h>

namespace Digikam
{

CreatePtoTask::CreatePtoTask(const QString& workDirPath, PanoramaFileType fileType,
                             QUrl& ptoUrl, const QList<QUrl>& inputFiles, const PanoramaItemUrlsMap& preProcessedMap,
                             bool addGPlusMetadata, const QString& huginVersion)
    : PanoTask(PANO_CREATEPTO,
      workDirPath),
      ptoUrl(ptoUrl),
      preProcessedMap(&preProcessedMap),
      fileType(addGPlusMetadata ? JPEG : fileType),
      inputFiles(inputFiles),
      addGPlusMetadata(addGPlusMetadata),
      huginVersion(huginVersion)
{
}

CreatePtoTask::~CreatePtoTask()
{
}

void CreatePtoTask::run(ThreadWeaver::JobPointer, ThreadWeaver::Thread*)
{
    ptoUrl = tmpDir;
    ptoUrl.setPath(ptoUrl.path() + QLatin1String("pano_base.pto"));

    QFile pto(ptoUrl.toLocalFile());

    if (pto.exists())
    {
        errString = i18n("PTO file already created in the temporary directory.");
        successFlag = false;
        return;
    }

    if (!pto.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
    {
        errString   = i18n("PTO file cannot be created in the temporary directory.");
        successFlag = false;
        return;
    }

    // 1. Project parameters
    PTOType panoBase(huginVersion);

    if (addGPlusMetadata)
    {
        panoBase.project.projection = PTOType::Project::EQUIRECTANGULAR;
    }
    else
    {
        panoBase.project.projection = PTOType::Project::CYLINDRICAL;
    }

    panoBase.project.fieldOfView = 0;
    panoBase.project.hdr         = false;

    switch (fileType)
    {
        case JPEG:
            panoBase.project.fileFormat.fileType = PTOType::Project::FileFormat::JPEG;
            panoBase.project.fileFormat.quality = 90;
            break;
        case TIFF:
            panoBase.project.fileFormat.fileType = PTOType::Project::FileFormat::TIFF_m;
            panoBase.project.fileFormat.compressionMethod = PTOType::Project::FileFormat::LZW;
            panoBase.project.fileFormat.savePositions = false;
            panoBase.project.fileFormat.cropped = false;
            break;
        case HDR:
            panoBase.project.hdr = true;
            // TODO HDR
            break;
    }

//  panoBase.project.bitDepth = PTOType::Project::FLOAT;
//  panoBase.project.crop.setLeft(X_left);
//  panoBase.project.crop.setRight(X_right);
//  panoBase.project.crop.setTop(X_top);
//  panoBase.project.crop.setBottom(X_bottom);
    panoBase.project.photometricReferenceId = 0;

    // 2. Images
    panoBase.images.reserve(inputFiles.size());
    panoBase.images.resize(inputFiles.size());
    int i = 0;

    for (i = 0; i < inputFiles.size(); ++i)
    {
        QUrl inputFile(inputFiles.at(i));
        QUrl preprocessedUrl(preProcessedMap->value(inputFile).preprocessedUrl);
        m_meta.load(preprocessedUrl.toLocalFile());
        QSize size = m_meta.getPixelSize();

        panoBase.images[i]                = PTOType::Image();
        panoBase.images[i].lensProjection = PTOType::Image::RECTILINEAR;
        panoBase.images[i].size           = size;

        if (i > 0)
        {
            // We suppose that the pictures are all taken with the same camera and lens
            panoBase.images[i].lensBarrelCoefficientA.referenceId = 0;
            panoBase.images[i].lensBarrelCoefficientB.referenceId = 0;
            panoBase.images[i].lensBarrelCoefficientC.referenceId = 0;
            panoBase.images[i].lensCenterOffsetX.referenceId = 0;
            panoBase.images[i].lensCenterOffsetY.referenceId = 0;
            panoBase.images[i].lensShearX.referenceId = 0;
            panoBase.images[i].lensShearY.referenceId = 0;
            panoBase.images[i].vignettingCorrectionI.referenceId = 0;
            panoBase.images[i].vignettingCorrectionJ.referenceId = 0;
            panoBase.images[i].vignettingCorrectionK.referenceId = 0;
            panoBase.images[i].vignettingCorrectionL.referenceId = 0;
            panoBase.images[i].vignettingOffsetX.referenceId = 0;
            panoBase.images[i].vignettingOffsetY.referenceId = 0;
        }
        else
        {
            panoBase.images[i].optimisationParameters.push_back(PTOType::Optimisation());
            panoBase.images[i].optimisationParameters.last().parameter = PTOType::Optimisation::LENSA;
            panoBase.images[i].optimisationParameters.push_back(PTOType::Optimisation());
            panoBase.images[i].optimisationParameters.last().parameter = PTOType::Optimisation::LENSB;
            panoBase.images[i].optimisationParameters.push_back(PTOType::Optimisation());
            panoBase.images[i].optimisationParameters.last().parameter = PTOType::Optimisation::LENSC;
            panoBase.images[i].optimisationParameters.push_back(PTOType::Optimisation());
            panoBase.images[i].optimisationParameters.last().parameter = PTOType::Optimisation::LENSD;
            panoBase.images[i].optimisationParameters.push_back(PTOType::Optimisation());
            panoBase.images[i].optimisationParameters.last().parameter = PTOType::Optimisation::LENSE;
            panoBase.images[i].optimisationParameters.push_back(PTOType::Optimisation());
            panoBase.images[i].optimisationParameters.last().parameter = PTOType::Optimisation::VA;
            panoBase.images[i].optimisationParameters.push_back(PTOType::Optimisation());
            panoBase.images[i].optimisationParameters.last().parameter = PTOType::Optimisation::VB;
            panoBase.images[i].optimisationParameters.push_back(PTOType::Optimisation());
            panoBase.images[i].optimisationParameters.last().parameter = PTOType::Optimisation::VC;
            panoBase.images[i].optimisationParameters.push_back(PTOType::Optimisation());
            panoBase.images[i].optimisationParameters.last().parameter = PTOType::Optimisation::VD;
            panoBase.images[i].optimisationParameters.push_back(PTOType::Optimisation());
            panoBase.images[i].optimisationParameters.last().parameter = PTOType::Optimisation::VX;
            panoBase.images[i].optimisationParameters.push_back(PTOType::Optimisation());
            panoBase.images[i].optimisationParameters.last().parameter = PTOType::Optimisation::VY;
        }

        panoBase.images[i].optimisationParameters.push_back(PTOType::Optimisation());
        panoBase.images[i].optimisationParameters.last().parameter =  PTOType::Optimisation::RA;
        panoBase.images[i].optimisationParameters.push_back(PTOType::Optimisation());
        panoBase.images[i].optimisationParameters.last().parameter =  PTOType::Optimisation::RB;
        panoBase.images[i].optimisationParameters.push_back(PTOType::Optimisation());
        panoBase.images[i].optimisationParameters.last().parameter =  PTOType::Optimisation::RC;
        panoBase.images[i].optimisationParameters.push_back(PTOType::Optimisation());
        panoBase.images[i].optimisationParameters.last().parameter =  PTOType::Optimisation::RD;
        panoBase.images[i].optimisationParameters.push_back(PTOType::Optimisation());
        panoBase.images[i].optimisationParameters.last().parameter =  PTOType::Optimisation::RE;
        panoBase.images[i].optimisationParameters.push_back(PTOType::Optimisation());
        panoBase.images[i].optimisationParameters.last().parameter =  PTOType::Optimisation::EXPOSURE;
        panoBase.images[i].optimisationParameters.push_back(PTOType::Optimisation());
        panoBase.images[i].optimisationParameters.last().parameter =  PTOType::Optimisation::WBR;
        panoBase.images[i].optimisationParameters.push_back(PTOType::Optimisation());
        panoBase.images[i].optimisationParameters.last().parameter =  PTOType::Optimisation::WBB;
        panoBase.images[i].optimisationParameters.push_back(PTOType::Optimisation());
        panoBase.images[i].optimisationParameters.last().parameter =  PTOType::Optimisation::LENSYAW;
        panoBase.images[i].optimisationParameters.push_back(PTOType::Optimisation());
        panoBase.images[i].optimisationParameters.last().parameter =  PTOType::Optimisation::LENSPITCH;
        panoBase.images[i].optimisationParameters.push_back(PTOType::Optimisation());
        panoBase.images[i].optimisationParameters.last().parameter =  PTOType::Optimisation::LENSROLL;
        panoBase.images[i].fileName = preprocessedUrl.toLocalFile();
    }

    switch (fileType)
    {
        case TIFF:
            panoBase.lastComments << QLatin1String("#hugin_outputImageType tif");
            panoBase.lastComments << QLatin1String("#hugin_outputImageTypeCompression LZW");
            break;
        case JPEG:
            panoBase.lastComments << QLatin1String("#hugin_outputImageType jpg");
            panoBase.lastComments << QLatin1String("#hugin_outputJPEGQuality 90");
            break;
        case HDR:
            // TODO: HDR
            break;
    }

    panoBase.createFile(ptoUrl.toLocalFile());

    successFlag = true;
    return;
}

}  // namespace Digikam
