/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-05-20
 * Description : a tool to export images to Onedrive web service
 *
 * Copyright (C) 2013      by Pankaj Kumar <me at panks dot me>
 * Copyright (C) 2013-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef OD_TALKER_H
#define OD_TALKER_H

// Qt includes

#include <QList>
#include <QPair>
#include <QString>
#include <QSettings>
#include <QNetworkReply>
#include <QNetworkAccessManager>

// Local includes

#include "oditem.h"
#include "o2.h"
#include "o0globals.h"
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
    void listFolders(const QString& path = QString());
    void createFolder(const QString& path);

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

private Q_SLOTS:

    void slotLinkingFailed();
    void slotLinkingSucceeded();
    void slotOpenBrowser(const QUrl& url);
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

#endif // OD_TALKER_H
