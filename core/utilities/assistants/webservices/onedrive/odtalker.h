/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-05-20
 * Description : a tool to export images to Onedrive web service
 *
 * Copyright (C) 2018      by Tarek Talaat <tarektalaat93 at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef DIGIKAM_OD_TALKER_H
#define DIGIKAM_OD_TALKER_H

// Qt includes

#include <QList>
#include <QPair>
#include <QString>
#include <QSettings>
#include <QNetworkReply>
#include <QNetworkAccessManager>

// Local includes

#include "oditem.h"
#include "dmetadata.h"

namespace Digikam
{

class ODTalker : public QObject
{
    Q_OBJECT

public:

    explicit ODTalker(QWidget* const parent);
    ~ODTalker();

public:

    void link();
    void unLink();
    void getUserName();
    bool authenticated();
    void cancel();
    bool addPhoto(const QString& imgPath, const QString& uploadFolder, bool rescale, int maxDim, int imageQuality);
    void listFolders();
    void createFolder(QString& path);
    void setAccessToken(const QString& token);
    QMap<QString, QString> ParseUrlParameters(const QString& url);
    void readSettings();
    void writeSettings();

Q_SIGNALS:

    void signalBusy(bool val);
    void signalLinkingSucceeded();
    void signalLinkingFailed();
    void signalSetUserName(const QString& msg);
    void signalListAlbumsFailed(const QString& msg);
    void signalListAlbumsDone(const QList<QPair<QString, QString> >& list);
    void signalCreateFolderFailed(const QString& msg);
    void signalCreateFolderSucceeded();
    void signalAddPhotoFailed(const QString& msg);
    void signalAddPhotoSucceeded();
    void oneDriveLinkingSucceeded();
    void oneDriveLinkingFailed();

private Q_SLOTS:

    void slotLinkingFailed();
    void slotLinkingSucceeded();
    void slotCatchUrl(const QUrl& url);
    void slotFinished(QNetworkReply* reply);

private:

    void parseResponseUserName(const QByteArray& data);
    void parseResponseListFolders(const QByteArray& data);
    void parseResponseCreateFolder(const QByteArray& data);
    void parseResponseAddPhoto(const QByteArray& data);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_OD_TALKER_H
