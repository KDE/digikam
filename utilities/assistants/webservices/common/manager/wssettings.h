/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-01-24
 * Description : Web Service settings container.
 *
 * Copyright (C) 2017-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef WS_SETTING_H
#define WS_SETTING_H

// Qt includes

#include <QtGlobal>
#include <QList>
#include <QString>
#include <QStringList>
#include <QUrl>
#include <QMap>

class KConfigGroup;

namespace Digikam
{

class WSSettings
{

public:

    // Images selection mode
    enum Selection
    {
        IMAGES = 0,
        ALBUMS
    };

    enum WebService
    {
        FLICKR = 0,
        DROPBOX,
        IMGUR
    };

    enum ImageFormat
    {
        JPEG = 0,
        PNG
    };

public:

    explicit WSSettings();
    ~WSSettings();

    // Read and write settings in config file between sessions.
    void  readSettings(KConfigGroup& group);
    void  writeSettings(KConfigGroup& group);

    QString format() const;

    void setMailUrl(const QUrl& orgUrl, const QUrl& emailUrl);
    QUrl mailUrl(const QUrl& orgUrl) const;

    // Helper methods to fill settings from GUI.
    static QMap<WebService,  QString> webServiceNames();
    static QMap<ImageFormat, QString> imageFormatNames();

public:

    Selection                 selMode;             // Items selection mode

    QList<QUrl>               inputImages;         // Selected items to send.

    bool                      addFileProperties;
    bool                      imagesChangeProp;

    bool                      removeMetadata;

    int                       imageCompression;

    qint64                    attLimitInMbytes;

    QString                   tempPath;

    WebService                webService;

    int                       imageSize;

    ImageFormat               imageFormat;

    QMap<QUrl, QUrl>          itemsList; // Map of original item and attached item (can be resized).
};

} // namespace Digikam

#endif // WS_SETTING_H
