/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-04-12
 * Description : A tool to export items to Rajce web service
 *
 * Copyright (C) 2011      by Lukas Krejci <krejci.l at centrum dot cz>
 * Copyright (C) 2011-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "rajcetalker.h"

// Qt includes

#include <QWidget>
#include <QMutex>
#include <QQueue>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QCryptographicHash>
#include <QXmlResultItems>
#include <QXmlQuery>
#include <QFileInfo>
#include <QUrl>

// Local includes

#include "digikam_debug.h"
#include "rajcempform.h"
#include "rajcecommand.h"
#include "digikam_version.h"
#include "dmetadata.h"
#include "wstoolutils.h"
#include "previewloadthread.h"

namespace Digikam
{

const QUrl RAJCE_URL(QString::fromLatin1("http://www.rajce.idnes.cz/liveAPI/index.php"));

class RajceTalker::Private
{
public:

    explicit Private()
      : queueAccess(QMutex::Recursive),
        netMngr(0),
        reply(0)
    {
    }

    QQueue<RajceCommand*>  commandQueue;
    QMutex                 queueAccess;
    QString                tmpDir;

    QNetworkAccessManager* netMngr;
    QNetworkReply*         reply;

    RajceSession           session;
};

RajceTalker::RajceTalker(QWidget* const parent)
    : QObject(parent),
      d(new Private)
{
    d->tmpDir  = WSToolUtils::makeTemporaryDir("rajce").absolutePath() + QLatin1Char('/');
    d->netMngr = new QNetworkAccessManager(this);

    connect(d->netMngr, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(slotFinished(QNetworkReply*)));
}

RajceTalker::~RajceTalker()
{
    delete d;
}

const RajceSession& RajceTalker::session() const
{
    return d->session;
}

void RajceTalker::startCommand(RajceCommand* const command)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Sending command:\n" << command->getXml();

    QByteArray data = command->encode();

    QNetworkRequest netRequest(RAJCE_URL);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, command->contentType());

    d->reply = d->netMngr->post(netRequest, data);

    connect(d->reply, SIGNAL(uploadProgress(qint64,qint64)),
            SLOT(slotUploadProgress(qint64,qint64)));

    emit signalBusyStarted(command->commandType());
}

void RajceTalker::login(const QString& username, const QString& password)
{
    LoginCommand* const command = new LoginCommand(username, password);
    enqueueCommand(command);
}

void RajceTalker::loadAlbums()
{
    AlbumListCommand* const command = new AlbumListCommand(d->session);
    enqueueCommand(command);
}

void RajceTalker::createAlbum(const QString& name, const QString& description, bool visible)
{
    CreateAlbumCommand* const command = new CreateAlbumCommand(name, description, visible, d->session);
    enqueueCommand(command);
}

void RajceTalker::slotFinished(QNetworkReply* reply)
{
    if (reply != d->reply)
    {
        return;
    }

    QString response      = QString::fromUtf8(reply->readAll());

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << response;

    d->queueAccess.lock();

    RajceCommand* const c = d->commandQueue.head();
    d->reply              = 0;

    c->processResponse(response, d->session);

    RajceCommandType type = c->commandType();

    delete c;

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "State after command: " << d->session;

    // Let the users react on the command before we
    // let the next queued command in.
    // This enables the connected slots to read in
    // reliable values from the state and/or
    // clear the error state once it's handled.
    emit signalBusyFinished(type);

    reply->deleteLater();

    // Only dequeue the command after the above signal has been
    // emitted so that the users can queue other commands
    // without them being started straight away in the enqueue
    // method which would happen if the command was dequed
    // before the signal and the signal was emitted in the same
    // thread (which is the case (always?)).
    d->commandQueue.dequeue();

    // see if there's something to continue with
    if (d->commandQueue.size() > 0)
    {
        startCommand(d->commandQueue.head());
    }

    d->queueAccess.unlock();
}

void RajceTalker::logout()
{
    //TODO
}

void RajceTalker::openAlbum(const RajceAlbum& album)
{
    OpenAlbumCommand* const command = new OpenAlbumCommand(album.id, d->session);
    enqueueCommand(command);
}

void RajceTalker::closeAlbum()
{
    if (!d->session.openAlbumToken().isEmpty())
    {
        CloseAlbumCommand* const command = new CloseAlbumCommand(d->session);
        enqueueCommand(command);
    }
    else
    {
        emit signalBusyFinished(CloseAlbum);
    }
}

void RajceTalker::uploadPhoto(const QString& path, unsigned dimension, int jpgQuality)
{
    AddPhotoCommand* const command = new AddPhotoCommand(d->tmpDir, path, dimension, jpgQuality, d->session);
    enqueueCommand(command);
}

void RajceTalker::clearLastError()
{
    d->session.lastErrorCode()    = 0;
    d->session.lastErrorMessage() = QString::fromLatin1("");
}

void RajceTalker::slotUploadProgress(qint64 bytesSent, qint64 bytesTotal)
{
    if (bytesTotal <= 0)
    {
        return;
    }

    unsigned percent = (unsigned)((float)bytesSent / bytesTotal * 100);

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Percent signalled: " << percent;

    emit signalBusyProgress(d->commandQueue.head()->commandType(), percent);
}

void RajceTalker::enqueueCommand(RajceCommand* const command)
{
    if (d->session.lastErrorCode() != 0)
    {
        return;
    }

    d->queueAccess.lock();
    d->commandQueue.enqueue(command);

    if (d->commandQueue.size() == 1)
    {
        startCommand(command);
    }

    d->queueAccess.unlock();
}

void RajceTalker::cancelCurrentCommand()
{
    if (d->reply != 0)
    {
        slotFinished(d->reply);
        d->reply->abort();
        d->reply = 0;
    }
}

void RajceTalker::init(const Digikam::RajceSession& initialState)
{
    d->session = initialState;
}

} // namespace Digikam
