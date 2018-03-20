/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-02-02
 * Description : a tool to export items to ImageShack web service
 *
 * Copyright (C) 2012      by Dodon Victor <dodonvictor at gmail dot com>
 * Copyright (C) 2013-2018 by Caulier Gilles <caulier dot gilles at gmail dot com>
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

#ifndef IMAGESHACK_TALKER_H
#define IMAGESHACK_TALKER_H

// Qt includes

#include <QObject>
#include <QString>
#include <QMap>
#include <QNetworkReply>
#include <QNetworkAccessManager>

class QDomElement;
class QByteArray;

namespace Digikam
{

class ImageShackSession;

class ImageShackTalker : public QObject
{
    Q_OBJECT

public:

    explicit ImageShackTalker(ImageShackSession* const session);
    ~ImageShackTalker();

public:

    void authenticate();
    void cancelLogIn();
    void cancel();
    void getGalleries();

    void uploadItem(const QString& path, const QMap<QString, QString>& opts);
    void uploadItemToGallery(const QString& path,
                             const QString& gallery,
                             const QMap<QString, QString>& opts);

Q_SIGNALS:

    void signalBusy(bool busy);
    void signalJobInProgress(int step, int maxStep = 0, const QString& label = QString());
    void signalLoginDone(int errCode,  const QString &errMsg);
    void signalGetGalleriesDone(int errCode, const QString &errMsg);

    void signalAddPhotoDone(int errCode, const QString& errMsg);
    void signalUpdateGalleries(const QStringList& gTexts, const QStringList& gNames);

private Q_SLOTS:

    void slotFinished(QNetworkReply* reply);

private:

    QString getCallString(QMap<QString, QString>& args) const;
    void    checkRegistrationCodeDone(int errCode, const QString& errMsg);
    void    parseAccessToken(const QByteArray& data);
    void    parseGetGalleries(const QByteArray& data);
    void    authenticationDone(int errCode, const QString& errMsg);

    void    logOut();

    int     parseErrorResponse(QDomElement elem, QString& errMsg) const;

    void    parseUploadPhotoDone(QByteArray data);
    void    parseAddPhotoToGalleryDone(QByteArray data);

    QString mimeType(const QString& path) const;

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // IMAGESHACK_TALKER_H
