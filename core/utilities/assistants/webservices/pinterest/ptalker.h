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

#ifndef P_TALKER_H
#define P_TALKER_H

// Qt includes

#include <QList>
#include <QPair>
#include <QString>
#include <QSettings>
#include <QNetworkReply>
#include <QNetworkAccessManager>

// Local includes

#include "pitem.h"
#include "dmetadata.h"

namespace Digikam
{

class PTalker : public QObject
{
    Q_OBJECT

public:

    explicit PTalker(QWidget* const parent);
    ~PTalker();

public:

    void link();
    void unLink();
    void getUserName();
    bool authenticated();
    void cancel();
    bool addPin(const QString& imgPath, const QString& uploadFolder, bool rescale, int maxDim, int imageQuality);
    void listBoards(const QString& path = QString());
    void createBoard(QString& boardName);
    void setAccessToken(const QString& token);
    QMap<QString,QString> ParseUrlParameters(const QString& url);

Q_SIGNALS:

    void signalBusy(bool val);
    void signalLinkingSucceeded();
    void signalLinkingFailed();
    void signalSetUserName(const QString& msg);
    void signalListBoardsFailed(const QString& msg);
    void signalListBoardsDone(const QList<QPair<QString, QString> >& list);
    void signalCreateBoardFailed(const QString& msg);
    void signalCreateBoardSucceeded();
    void signalAddPinFailed(const QString& msg);
    void signalAddPinSucceeded();
    void pinterestLinkingSucceeded();
    void pinterestLinkingFailed();

private Q_SLOTS:

    void slotLinkingFailed();
    void slotLinkingSucceeded();
    void slotCatchUrl(const QUrl& url);
    void slotOpenBrowser(const QUrl& url);
    void slotFinished(QNetworkReply* reply);

private:

    void parseResponseUserName(const QByteArray& data);
    void parseResponseListBoards(const QByteArray& data);
    void parseResponseCreateBoard(const QByteArray& data);
    void parseResponseAddPin(const QByteArray& data);
    void parseResponseAccessToken(const QByteArray& data);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // P_TALKER_H
