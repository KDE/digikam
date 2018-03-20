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

#include "copyfilestask.h"

// Qt includes

#include <QFileInfo>
#include <QDateTime>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "drawdecoder.h"



namespace Digikam
{

CopyFilesTask::CopyFilesTask(const QString& workDirPath, const QUrl& panoUrl, const QUrl& finalPanoUrl,
                             const QUrl& ptoUrl, const PanoramaItemUrlsMap& urls, bool sPTO, bool GPlusMetadata)
    : PanoTask(PANO_COPY, workDirPath),
      panoUrl(panoUrl),
      finalPanoUrl(finalPanoUrl),
      ptoUrl(ptoUrl),
      urlList(&urls),
      savePTO(sPTO),
      addGPlusMetadata(GPlusMetadata)
{
}

CopyFilesTask::~CopyFilesTask()
{
}

void CopyFilesTask::run(ThreadWeaver::JobPointer, ThreadWeaver::Thread*)
{
    QFile     panoFile(panoUrl.toLocalFile());
    QFile     finalPanoFile(finalPanoUrl.toLocalFile());

    QFileInfo fi(finalPanoUrl.toLocalFile());
    QUrl      finalPTOUrl = finalPanoUrl.adjusted(QUrl::RemoveFilename);
    finalPTOUrl.setPath(finalPTOUrl.path() + fi.completeBaseName() + QLatin1String(".pto"));

    QFile     ptoFile(ptoUrl.toLocalFile());
    QFile     finalPTOFile(finalPTOUrl.toLocalFile());

    if (!panoFile.exists())
    {
        errString = i18n("Temporary panorama file does not exists.");
        qCDebug(DIGIKAM_GENERAL_LOG) << "Temporary panorama file does not exists: " << panoUrl;
        successFlag = false;
        return;
    }

    if (finalPanoFile.exists())
    {
        errString = i18n("A panorama file named <filename>%1</filename> already exists.", finalPanoUrl.fileName());
        qCDebug(DIGIKAM_GENERAL_LOG) << "Final panorama file already exists: " << finalPanoUrl;
        successFlag = false;
        return;
    }

    if (savePTO && !ptoFile.exists())
    {
        errString = i18n("Temporary project file does not exist.");
        qCDebug(DIGIKAM_GENERAL_LOG) << "Temporary project file does not exists: " << ptoUrl;
        successFlag = false;
        return;
    }

    if (savePTO && finalPTOFile.exists())
    {
        errString = i18n("A project file named <filename>%1</filename> already exists.", finalPTOUrl.fileName());
        qCDebug(DIGIKAM_GENERAL_LOG) << "Final project file already exists: " << finalPTOUrl;
        successFlag = false;
        return;
    }

    qCDebug(DIGIKAM_GENERAL_LOG) << "Copying GPS info...";

    // Find first src image which contain geolocation and save it to target pano file.

    double lat, lng, alt;

    for (PanoramaItemUrlsMap::const_iterator i = urlList->constBegin(); i != urlList->constEnd(); ++i)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << i.key();

        m_meta.load(i.key().toLocalFile());

        if (m_meta.getGPSInfo(alt, lat, lng))
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "GPS info found and saved in " << panoUrl;
            m_meta.load(panoUrl.toLocalFile());
            m_meta.setGPSInfo(alt, lat, lng);
            m_meta.applyChanges();
            break;
        }
    }

    // Restore usual and common metadata from first shot.

    m_meta.load(urlList->constBegin().key().toLocalFile());
    QByteArray iptc = m_meta.getIptc();
    QByteArray xmp  = m_meta.getXmp();
    QString make    = m_meta.getExifTagString("Exif.Image.Make");
    QString model   = m_meta.getExifTagString("Exif.Image.Model");
    QDateTime dt    = m_meta.getImageDateTime();

    m_meta.load(panoUrl.toLocalFile());
    m_meta.setIptc(iptc);
    m_meta.setXmp(xmp);
    m_meta.setXmpTagString("Xmp.tiff.Make",   make);
    m_meta.setXmpTagString("Xmp.tiff.Model", model);
    m_meta.setImageDateTime(dt);

    QString filesList;

    for (PanoramaItemUrlsMap::const_iterator i = urlList->constBegin(); i != urlList->constEnd(); ++i)
        filesList.append(i.key().fileName() + QLatin1String(" ; "));

    filesList.truncate(filesList.length()-3);

    m_meta.setXmpTagString("Xmp.digiKam.PanoramaInputFiles", filesList);

    // NOTE : See https://developers.google.com/photo-sphere/metadata/ for details
    if (addGPlusMetadata)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Adding PhotoSphere metadata...";
        m_meta.registerXmpNameSpace(QLatin1String("http://ns.google.com/photos/1.0/panorama/"), QLatin1String("GPano"));
        m_meta.setXmpTagString("Xmp.GPano.UsePanoramaViewer", QLatin1String("True"));
        m_meta.setXmpTagString("Xmp.GPano.StitchingSoftware", QLatin1String("Panorama digiKam tool with Hugin"));
        m_meta.setXmpTagString("Xmp.GPano.ProjectionType",    QLatin1String("equirectangular"));
    }

    m_meta.applyChanges();

    qCDebug(DIGIKAM_GENERAL_LOG) << "Copying panorama file...";

    if (!panoFile.copy(finalPanoUrl.toLocalFile()) || !panoFile.remove())
    {
        errString = i18n("Cannot move panorama from <filename>%1</filename> to <filename>%2</filename>.",
                         panoUrl.toLocalFile(),
                         finalPanoUrl.toLocalFile());
        qCDebug(DIGIKAM_GENERAL_LOG) << "Cannot move panorama: QFile error = " << panoFile.error();
        successFlag = false;
        return;
    }

    if (savePTO)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Copying project file...";

        if (!ptoFile.copy(finalPTOUrl.toLocalFile()))
        {
            errString = i18n("Cannot move project file from <filename>%1</filename> to <filename>%2</filename>.",
                             panoUrl.toLocalFile(),
                             finalPanoUrl.toLocalFile());
            successFlag = false;
            return;
        }

        qCDebug(DIGIKAM_GENERAL_LOG) << "Copying converted RAW files...";

        for (PanoramaItemUrlsMap::const_iterator i = urlList->constBegin(); i != urlList->constEnd(); ++i)
        {
            if (DRawDecoder::isRawFile(i.key()))
            {
                QUrl finalImgUrl = finalPanoUrl.adjusted(QUrl::RemoveFilename);
                finalImgUrl.setPath(finalImgUrl.path() + i->preprocessedUrl.fileName());
                QFile finalImgFile(finalImgUrl.toString(QUrl::PreferLocalFile));
                QFile imgFile(i->preprocessedUrl.toLocalFile());

                if (finalImgFile.exists())
                {
                    continue;
                }

                if (!imgFile.copy(finalImgUrl.toLocalFile()))
                {
                    errString = i18n("Cannot copy converted image file from <filename>%1</filename> to <filename>%2</filename>.",
                                     i->preprocessedUrl.toLocalFile(),
                                     finalImgUrl.toLocalFile());
                    successFlag = false;
                    return;
                }
            }
        }
    }

    successFlag = true;
    return;
}

}  // namespace Digikam
