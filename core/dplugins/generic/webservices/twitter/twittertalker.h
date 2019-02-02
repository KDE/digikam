/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-06-29
 * Description : a tool to export images to Twitter social network
 *
 * Copyright (C) 2018 by Tarek Talaat <tarektalaat93 at gmail dot com>
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

#ifndef DIGIKAM_TWITTER_TALKER_H
#define DIGIKAM_TWITTER_TALKER_H

// Qt includes

#include <QList>
#include <QPair>
#include <QString>
#include <QNetworkReply>

// Local includes

#include "twitteritem.h"
#include "o2.h"
#include "o0globals.h"
#include "dmetadata.h"
#include "o1twitter.h"

using namespace Digikam;

namespace DigikamGenericTwitterPlugin
{

class TwTalker : public QObject
{
    Q_OBJECT

public:

    explicit TwTalker(QWidget* const parent);
    ~TwTalker();

public:

    void link();
    void unLink();
    void getUserName();
    bool authenticated();
    void cancel();
    bool addPhoto(const QString& imgPath, const QString& uploadFolder, bool rescale, int maxDim, int imageQuality);
    void listFolders(const QString& path = QString());
    void createFolder(QString& path);
    void setAccessToken(const QString& token);
    QMap<QString, QString> ParseUrlParameters(const QString& url);
    void createTweet(const QString& mediaId);

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
    void twitterLinkingSucceeded();
    void twitterLinkingFailed();

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
    void parseResponseCreateTweet(const QByteArray& data);

private:

    class Private;
    Private* const d;
};

} // namespace DigikamGenericTwitterPlugin

#endif // DIGIKAM_TWITTER_TALKER_H
