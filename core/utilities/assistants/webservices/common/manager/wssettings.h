/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-01-24
 * Description : Web Service settings container.
 *
 * Copyright (C) 2017-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2018      by Thanh Trung Dinh <dinhthanhtrung1996 at gmail dot com>
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

#ifndef DIGIKAM_WS_SETTINGS_H
#define DIGIKAM_WS_SETTINGS_H

// Qt includes

#include <QObject>
#include <QtGlobal>
#include <QList>
#include <QString>
#include <QStringList>
#include <QUrl>
#include <QMap>
#include <QSettings>

// Local include

#include "o0settingsstore.h"

class KConfigGroup;

namespace Digikam
{

class WSSettings : public QObject
{
    Q_OBJECT

public:

    // Images selection mode
    enum Selection
    {
        EXPORT = 0,
        IMPORT

    };

    enum WebService
    {
        FLICKR = 0,
        DROPBOX,
        IMGUR,
        FACEBOOK,
        SMUGMUG,
        GDRIVE,
        GPHOTO
    };

    enum ImageFormat
    {
        JPEG = 0,
        PNG
    };

public:

    explicit WSSettings(QObject* const parent=0);
    ~WSSettings();

    // Read and write settings in config file between sessions.
    void  readSettings(KConfigGroup& group);
    void  writeSettings(KConfigGroup& group);

    QString format() const;

    // Helper methods to fill settings from GUI.
    static QMap<WebService,  QString> webServiceNames();
    static QMap<ImageFormat, QString> imageFormatNames();

    // Helper method to list all user accounts (of all web service) that user logged in before.
    QStringList allUserNames(const QString& serviceName);

public:

    Selection           selMode;             // Items selection mode

    QList<QUrl>         inputImages;         // Selected items to upload.

    bool                addFileProperties;
    bool                imagesChangeProp;

    bool                removeMetadata;

    int                 imageCompression;

    qint64              attLimitInMbytes;

    WebService          webService;

    QString             userName;

    QSettings*          oauthSettings;
    O0SettingsStore*    oauthSettingsStore;

    QString             currentAlbumId;     // Selected album to upload to

    int                 imageSize;

    ImageFormat         imageFormat;

    QMap<QUrl, QUrl>    itemsList;          // Map of original item and attached item (can be resized).
};

} // namespace Digikam

#endif // DIGIKAM_WS_SETTINGS_H
