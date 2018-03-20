/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2016-05-27
 * Description : Implementation of v3 of the Imgur API
 *
 * Copyright (C) 2016      by Fabian Vogt <fabian at ritter dash vogt dot de>
 * Copyright (C) 2016-2018 by Caulier Gilles <caulier dot gilles at gmail dot com>
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

#ifndef IMGUR_TALKER_H
#define IMGUR_TALKER_H

// C++ includes

#include <atomic>
#include <queue>

// Qt includes

#include <QString>
#include <QFile>
#include <QUrl>

// Local includes

#include "o2.h"

namespace Digikam
{

enum class ImgurTalkerActionType
{
    ACCT_INFO,       // Action: account Result : account
    IMG_UPLOAD,      // Action: upload Result  : image
    ANON_IMG_UPLOAD, // Action: upload Result  : image
};

struct ImgurTalkerAction
{
    ImgurTalkerActionType type;

    struct
    {
        QString imgpath;
        QString title;
        QString description;
    } upload;

    struct
    {
        QString username;
    } account;
};

struct ImgurTalkerResult
{
    const ImgurTalkerAction* action;

    struct ImgurImage
    {
        QString    name;
        QString    title;
        QString    hash;
        QString    deletehash;
        QString    url;
        QString    description;
        qulonglong datetime;
        QString    type;
        bool       animated;
        uint       width;
        uint       height;
        uint       size;
        uint       views;
        qulonglong bandwidth;
    } image;

    struct ImgurAccount
    {
        QString username;
    } account;
};

// ----------------------------------------------------------------

/*
 * Main class, handles the client side of the Imgur API v3.
 */
class ImgurTalker : public QObject
{
Q_OBJECT

public:

    explicit ImgurTalker(const QString& client_id,
                         const QString& client_secret,
                         QObject* const parent = nullptr);
    ~ImgurTalker();

public:

    /* Use this method to read/write the access and refresh tokens.
     */
    O2& getAuth();

    unsigned int workQueueLength();
    void         queueWork(const ImgurTalkerAction& action);
    void         cancelAllWork();

    static QUrl  urlForDeletehash(const QString& deletehash);

Q_SIGNALS:

    /* Called if authentication state changes.
     */
    void signalAuthorized(bool success, const QString& username);
    void signalAuthError(const QString& msg);

    /* Open url in a browser and let the user copy the pin.
     * Call setPin(pin) to authorize.
     */
    void signalRequestPin(const QUrl& url);

    /* Emitted on progress changes.
     */
    void signalProgress(unsigned int percent, const ImgurTalkerAction& action);
    void signalSuccess(const ImgurTalkerResult& result);
    void signalError(const QString& msg, const ImgurTalkerAction& action);

    /* Emitted when the status changes.
     */
    void signalBusy(bool b);

public Q_SLOTS:

    /* Connected to O2 linkedChanged().
     */
    void slotOauthAuthorized();

    /* Connected to O2 openBrowser(QUrl).
     */
    void slotOauthRequestPin(const QUrl& url);

    /* Connected to O2 linkingFailed().
     */
    void slotOauthFailed();

    /* Connected to the current QNetworkReply.
     */
    void slotUploadProgress(qint64 sent, qint64 total);
    void slotReplyFinished();

protected:

    void timerEvent(QTimerEvent* event) override;

private:

    /* Starts timer if queue not empty.
     */
    void startWorkTimer();

    /* Stops timer if running.
     */
    void stopWorkTimer();

    /* Adds the user authorization info to the request.
     */
    void addAuthToken(QNetworkRequest* request);

    /* Adds the client authorization info to the request.
     */
    void addAnonToken(QNetworkRequest* request);

    /* Start working on the first item of m_work_queue
     * by sending a request.
     */
    void doWork();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // IMGUR_TALKER_H
