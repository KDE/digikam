/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-12-26
 * Description : a tool to export items to Facebook web service
 *
 * Copyright (C) 2008-2009 by Luka Renko <lure at kubuntu dot org>
 * Copyright (C) 2008-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2018      by Thanh Trung Dinh <dinhthanhtrung1996 at gmail dot com>
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

#ifndef DIGIKAM_FB_TALKER_H
#define DIGIKAM_FB_TALKER_H

// Qt includes

#include <QList>
#include <QString>
#include <QTime>
#include <QObject>
#include <QUrl>
#include <QNetworkReply>
#include <QMap>
#include <QSettings>

// Local includes

#include "fbitem.h"
#include "wsnewalbumdialog.h"
#include "wstalker.h"
#include "wsitem.h"

// O2 include

#include "o2.h"
#include "o0globals.h"

class QDomElement;

namespace Digikam
{

class FbTalker : public WSTalker
{
    Q_OBJECT

public:

    explicit FbTalker(QWidget* const parent, WSNewAlbumDialog* albumDlg=0);
    ~FbTalker();

    void    link();
    void    unlink();
    bool    linked() const;

    void    resetTalker(const QString& expire, const QString& accessToken, const QString& refreshToken);

    FbUser  getUser() const;

    void    authenticate();
    void    logout();

    void    listAlbums(long long userID = 0);

    void    createNewAlbum();
    void    createAlbum(const FbAlbum& album);

    void    addPhoto(const QString& imgPath, const QString& albumID,
                     const QString& caption);

Q_SIGNALS:

    void    signalLoginProgress(int step, int maxStep = 0, const QString& label = QString());
    void    signalLoginDone(int errCode, const QString& errMsg);

private:

    //QString getApiSig(const QMap<QString, QString>& args);
    void    authenticationDone(int errCode, const QString& errMsg);
    void    getLoggedInUser();

    QString errorToText(int errCode, const QString& errMsg);
    int     parseErrorResponse(const QDomElement& e, QString& errMsg);
    void    parseResponseGetLoggedInUser(const QByteArray& data);
    void    parseResponseAddPhoto(const QByteArray& data);
    void    parseResponseCreateAlbum(const QByteArray& data);
    void    parseResponseListAlbums(const QByteArray& data);

private Q_SLOTS:

    void    slotResponseTokenReceived(const QMap<QString, QString>& rep);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_FB_TALKER_H
