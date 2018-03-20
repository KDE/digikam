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

#ifndef RAJCE_TALKER_H
#define RAJCE_TALKER_H

// Qt includes

#include <QObject>
#include <QString>

// Local includes

#include "rajcesession.h"

class QWidget;
class QNetworkReply;

namespace Digikam
{

class RajceCommand;

class RajceTalker : public QObject
{
    Q_OBJECT

public:

    explicit RajceTalker(QWidget* const parent);
    ~RajceTalker();

public:

    void  init(const RajceSession& initialState);
    const RajceSession& session() const;
    void  login(const QString& username, const QString& password);
    void  logout();

    void  loadAlbums();
    void  createAlbum(const QString& name, const QString& description, bool visible);
    void  openAlbum(const RajceAlbum& album);
    void  closeAlbum();

    void  uploadPhoto(const QString& path, unsigned dimension, int jpgQuality);

    void  clearLastError();
    void  cancelCurrentCommand();

Q_SIGNALS:

    void signalBusyStarted(unsigned);
    void signalBusyFinished(unsigned);
    void signalBusyProgress(unsigned, unsigned percent);

private Q_SLOTS:

    void slotFinished(QNetworkReply* reply);
    void slotUploadProgress(qint64 bytesSent, qint64 bytesTotal);

private:

    void startCommand(RajceCommand* const);
    void enqueueCommand(RajceCommand* const);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // RAJCE_TALKER_H
