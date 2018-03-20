/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-12-21
 * Description : abstract camera interface class
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DKCAMERA_H
#define DKCAMERA_H

// Qt includes

#include <QString>
#include <QByteArray>

// Local includes

#include "camiteminfo.h"

class QStringList;
class QImage;

namespace Digikam
{

class DMetadata;

class DKCamera : public QObject
{
    Q_OBJECT

public:

    enum CameraDriverType
    {
        GPhotoDriver = 0,
        UMSDriver
    };

public:

    DKCamera(const QString& title, const QString& model, const QString& port, const QString& path);
    virtual ~DKCamera();

public:

    virtual bool doConnect() = 0;
    virtual void cancel() = 0;

    virtual bool getFolders(const QString& folder) = 0;

    /// If getImageDimensions is false, the camera shall set width and height to -1
    /// if the values are not immediately available
    virtual bool getItemsInfoList(const QString& folder, bool useMetadata, CamItemInfoList& infoList) = 0;
    virtual void getItemInfo(const QString& folder, const QString& itemName, CamItemInfo& info, bool useMetadata) = 0;
    virtual bool getThumbnail(const QString& folder, const QString& itemName, QImage& thumbnail) = 0;
    virtual bool getMetadata(const QString& folder, const QString& itemName, DMetadata& meta) = 0;
    virtual bool getFreeSpace(unsigned long& kBSize, unsigned long& kBAvail) = 0;
    virtual bool getPreview(QImage& preview) = 0;

    virtual bool capture(CamItemInfo& itemInfo) = 0;

    virtual bool downloadItem(const QString& folder, const QString& itemName, const QString& saveFile) = 0;
    virtual bool deleteItem(const QString& folder, const QString& itemName) = 0;
    virtual bool uploadItem(const QString& folder, const QString& itemName, const QString& localFile, CamItemInfo& itemInfo) = 0;
    virtual bool cameraSummary(QString& summary) = 0;
    virtual bool cameraManual(QString& manual) = 0;
    virtual bool cameraAbout(QString& about) = 0;

    virtual bool setLockItem(const QString& folder, const QString& itemName, bool lock) = 0;

    virtual DKCamera::CameraDriverType cameraDriverType() = 0;
    virtual QByteArray                 cameraMD5ID() = 0;

public:

    QString title() const;
    QString model() const;
    QString port()  const;
    QString path()  const;
    QString uuid()  const;

    bool    thumbnailSupport()           const;
    bool    deleteSupport()              const;
    bool    uploadSupport()              const;
    bool    mkDirSupport()               const;
    bool    delDirSupport()              const;
    bool    captureImageSupport()        const;
    bool    captureImagePreviewSupport() const;

    QString mimeType(const QString& fileext) const;

    void printSupportedFeatures();

protected:

    void    fillItemInfoFromMetadata(CamItemInfo& item, const DMetadata& meta) const;

protected:

    bool    m_thumbnailSupport;
    bool    m_deleteSupport;
    bool    m_uploadSupport;
    bool    m_mkDirSupport;
    bool    m_delDirSupport;
    bool    m_captureImageSupport;
    bool    m_captureImagePreviewSupport;

    QString m_imageFilter;
    QString m_movieFilter;
    QString m_audioFilter;
    QString m_rawFilter;

    QString m_model;
    QString m_port;
    QString m_path;
    QString m_title;
    QString m_uuid;

Q_SIGNALS:

    void signalFolderList(const QStringList&);
};

}  // namespace Digikam

#endif /* DKCAMERA_H */
