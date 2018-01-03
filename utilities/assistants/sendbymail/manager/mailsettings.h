/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-11-07
 * Description : mail settings container.
 *
 * Copyright (C) 2007-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2010      by Andi Clemens <andi dot clemens at googlemail dot com>
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

#ifndef MAIL_SETTING_H
#define MAIL_SETTING_H

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

class MailSettings
{

public:

    // Images selection mode
    enum Selection
    {
        IMAGES = 0,
        ALBUMS
    };

    enum MailClient
    {
        BALSA = 0,
        CLAWSMAIL,
        EVOLUTION,
        KMAIL,
        NETSCAPE,       // Messenger (https://en.wikipedia.org/wiki/Netscape_Messenger_9)
        SYLPHEED,
        THUNDERBIRD
    };

    enum ImageFormat
    {
        JPEG = 0,
        PNG
    };

public:

    explicit MailSettings();
    ~MailSettings();

    // Read and write settings in config file between sessions.
    void  readSettings(KConfigGroup& group);
    void  writeSettings(KConfigGroup& group);

    QString format()           const;

    /** Return the attachement limit in bytes
     */
    qint64  attachementLimit() const;

    void setMailUrl(const QUrl& orgUrl, const QUrl& emailUrl);
    QUrl mailUrl(const QUrl& orgUrl) const;

    // Helper methods to fill combobox from GUI.
    static QMap<MailClient,  QString> mailClientNames();
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

    MailClient                mailProgram;

    int                       imageSize;

    ImageFormat               imageFormat;

    QMap<QUrl, QUrl>          itemsList; // Map of original item and attached item (can be resized).

    QMap<MailClient, QString> binPaths;  // Map of paths for all mail clients.
};

} // namespace Digikam

#endif // MAIL_SETTING_H
