/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-02-12
 * Description : a tool to export images to IPFS web service
 *
 * Copyright (C) 2018 by Amar Lakshya <amar dot lakshya at xaviers dot edu dot in>
 * Copyright (C) 2018 by Caulier Gilles <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_IPFS_TALKER_H
#define DIGIKAM_IPFS_TALKER_H

// C++ includes

#include <atomic>
#include <queue>

// Qt includes

#include <QNetworkAccessManager>
#include <QString>
#include <QFile>
#include <QUrl>

// Local includes

#include "o2.h"

namespace DigikamGenericIpfsPlugin
{

enum class IpfsTalkerActionType
{
    IMG_UPLOAD, // Action: upload Result: image
};

struct IpfsTalkerAction
{
    IpfsTalkerActionType type;

    struct
    {
        QString imgpath;
        QString title;
        QString description;
    } upload;
};

struct IpfsTalkerResult
{
    const IpfsTalkerAction* action;

    struct IPFSImage
    {
        QString    name;
        QString    url;
        uint       size;
    } image;
};

/* Main class, handles the client side of the globalupload.io API
 */
class IpfsTalker : public QObject
{
Q_OBJECT

public:

    explicit IpfsTalker(QObject* const parent = nullptr);
    ~IpfsTalker();

    unsigned int workQueueLength();
    void queueWork(const IpfsTalkerAction& action);
    void cancelAllWork();

Q_SIGNALS:

    /* Emitted on progress changes.
     */
    void progress(unsigned int percent, const IpfsTalkerAction& action);
    void success(const IpfsTalkerResult& result);
    void error(const QString& msg, const IpfsTalkerAction& action);

    /* Emitted when the status changes.
     */
    void busy(bool b);

public Q_SLOTS:

    /* Connected to the current QNetworkReply.
     */
    void uploadProgress(qint64 sent, qint64 total);
    void replyFinished();

protected:

    void timerEvent(QTimerEvent* event) override;

private:

    /* Starts m_work_timer if m_work_queue not empty.
     */
    void startWorkTimer();

    /* Stops m_work_timer if running.
     */
    void stopWorkTimer();

    /* Start working on the first item of m_work_queue
     * by sending a request.
     */
    void doWork();

private:

    class Private;
    Private* const d;
};

} // namespace DigikamGenericIpfsPlugin

#endif // DIGIKAM_IPFS_TALKER_H
