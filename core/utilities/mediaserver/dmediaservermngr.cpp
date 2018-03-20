/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-05-28
 * Description : Media server manager
 *
 * Copyright (C) 2012      by Smit Mehta <smit dot meh at gmail dot com>
 * Copyright (C) 2012-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "dmediaservermngr.h"

// Qt includes

#include <QList>
#include <QMap>
#include <QApplication>
#include <QStringList>
#include <QUrl>
#include <QFile>
#include <QDomDocument>
#include <QDomElement>
#include <QTextStream>
#include <QTextCodec>
#include <QStandardPaths>

// KDE includes

#include <klocalizedstring.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

// Local includes

#include "digikam_debug.h"
#include "dnotificationwrapper.h"

namespace Digikam
{

class DMediaServerMngrCreator
{
public:

    DMediaServerMngr object;
};

Q_GLOBAL_STATIC(DMediaServerMngrCreator, creator)

// ---------------------------------------------------------------------------------------------

class DMediaServerMngr::Private
{
public:

    Private()
    {
        server = 0;
    }

    // Configuration XML file to store albums map to share in case of restoring between sessions.
    QString              mapsConf;

    // Server instance pointer.
    DMediaServer*        server;

    // The current albums collection to share.
    MediaServerMap       collectionMap;

    static const QString configGroupName;
    static const QString configStartServerOnStartupEntry;
};

const QString DMediaServerMngr::Private::configGroupName(QLatin1String("DLNA Settings"));
const QString DMediaServerMngr::Private::configStartServerOnStartupEntry(QLatin1String("Start MediaServer At Startup"));

DMediaServerMngr* DMediaServerMngr::instance()
{
    return &creator->object;
}

DMediaServerMngr::DMediaServerMngr()
    : d(new Private)
{
    d->mapsConf = QStandardPaths::writableLocation(QStandardPaths::DataLocation) +
                  QLatin1String("/mediaserver.xml");
}

DMediaServerMngr::~DMediaServerMngr()
{
    delete d;
}

QString DMediaServerMngr::configGroupName() const
{
    return d->configGroupName;
}

QString DMediaServerMngr::configStartServerOnStartupEntry() const
{
    return d->configStartServerOnStartupEntry;
}

void DMediaServerMngr::cleanUp()
{
    delete d->server;
    d->server = 0;
}

bool DMediaServerMngr::loadAtStartup()
{
    KSharedConfig::Ptr config    = KSharedConfig::openConfig();
    KConfigGroup dlnaConfigGroup = config->group(configGroupName());
    bool startServerOnStartup    = dlnaConfigGroup.readEntry(configStartServerOnStartupEntry(), false);
    bool result                  = true;

    if (startServerOnStartup)
    {
        // Restore the old sharing configuration and start the server.
        result &= load();
        result &= startMediaServer();

        mediaServerNotification(result);

        return result;
    }

    return false;
}

void DMediaServerMngr::saveAtShutdown()
{
    KSharedConfig::Ptr config    = KSharedConfig::openConfig();
    KConfigGroup dlnaConfigGroup = config->group(configGroupName());
    bool startServerOnStartup    = dlnaConfigGroup.readEntry(configStartServerOnStartupEntry(), false);

    if (startServerOnStartup)
    {
        // Save the current sharing configuration for the next session.
        save();
    }

    cleanUp();
}

void DMediaServerMngr::mediaServerNotification(bool started)
{
    DNotificationWrapper(QLatin1String("mediaserverloadstartup"),
                         started ? i18n("Media Server have been started")
                                 : i18n("Media Server cannot be started!"),
                         qApp->activeWindow(), qApp->applicationName());
}

void DMediaServerMngr::setItemsList(const QString& aname, const QList<QUrl>& urls)
{
    d->collectionMap.clear();
    d->collectionMap.insert(aname, urls);
}

QList<QUrl> DMediaServerMngr::itemsList() const
{
    QList<QUrl> ret;

    if (!d->collectionMap.isEmpty())
    {
        QList<QList<QUrl> > ulst = d->collectionMap.values();

        foreach(QList<QUrl> urls, ulst)
        {
            ret << urls;
        }
    }

    return ret;
}

void DMediaServerMngr::setCollectionMap(const MediaServerMap& map)
{
    d->collectionMap = map;
}

MediaServerMap DMediaServerMngr::collectionMap() const
{
    return d->collectionMap;
}

bool DMediaServerMngr::startMediaServer()
{
    if (!d->server)
    {
        d->server = new DMediaServer();

        if (!d->server->init())
        {
            cleanUp();
            return false;
        }
    }

    if (d->collectionMap.isEmpty())
    {
        cleanUp();
        return false;
    }

    d->server->addAlbumsOnServer(d->collectionMap);
 
    return true;
}

bool DMediaServerMngr::isRunning() const
{
    return d->server ? true : false;
}

int DMediaServerMngr::albumsShared() const
{
    if (d->collectionMap.isEmpty())
        return 0;

    return d->collectionMap.uniqueKeys().count();
}

int DMediaServerMngr::itemsShared() const
{
    return itemsList().count();
}

bool DMediaServerMngr::save()
{
    QDomDocument doc(QString::fromLatin1("mediaserverlist"));
    doc.setContent(QString::fromUtf8("<!DOCTYPE XMLQueueList><mediaserverlist version=\"1.0\" client=\"digikam\" encoding=\"UTF-8\"/>"));
    QDomElement docElem = doc.documentElement();

    auto end = d->collectionMap.cend();

    for (auto it = d->collectionMap.cbegin() ; it != end ; ++it)
    {
        QDomElement elm = doc.createElement(QString::fromLatin1("album"));
        elm.setAttribute(QString::fromLatin1("title"), it.key());

        // ----------------------

        QDomElement data;

        foreach(const QUrl& url, it.value())
        {
            data = doc.createElement(QString::fromLatin1("path"));
            data.setAttribute(QString::fromLatin1("value"), url.toLocalFile());
            elm.appendChild(data);
        }

        docElem.appendChild(elm);
    }

    QFile file(d->mapsConf);

    if (!file.open(QIODevice::WriteOnly))
    {
        qCDebug(DIGIKAM_MEDIASRV_LOG) << "Cannot open XML file to store MediaServer list";
        qCDebug(DIGIKAM_MEDIASRV_LOG) << file.fileName();
        return false;
    }

    QTextStream stream(&file);
    stream.setCodec(QTextCodec::codecForName("UTF-8"));
    stream.setAutoDetectUnicode(true);
    stream << doc.toString(4);
    file.close();

    return true;
}

bool DMediaServerMngr::load()
{
    QFile file(d->mapsConf);

    if (file.exists())
    {
        if (!file.open(QIODevice::ReadOnly))
        {
            qCDebug(DIGIKAM_MEDIASRV_LOG) << "Cannot open XML file to load MediaServer list";
            return false;
        }

        QDomDocument doc(QString::fromLatin1("mediaserverlist"));

        if (!doc.setContent(&file))
        {
            qCDebug(DIGIKAM_MEDIASRV_LOG) << "Cannot load MediaServer list XML file";
            return false;
        }

        QDomElement    docElem = doc.documentElement();
        MediaServerMap map;
        QList<QUrl>    urls;
        QString        album;

        for (QDomNode n = docElem.firstChild() ; !n.isNull() ; n = n.nextSibling())
        {
            QDomElement e = n.toElement();

            if (e.isNull())
            {
                continue;
            }

            if (e.tagName() != QString::fromLatin1("album"))
            {
                continue;
            }

            album = e.attribute(QString::fromLatin1("title"));
            urls.clear();

            for (QDomNode n2 = e.firstChild() ; !n2.isNull() ; n2 = n2.nextSibling())
            {
                QDomElement e2 = n2.toElement();

                if (e2.isNull())
                {
                    continue;
                }

                QString name2 = e2.tagName();
                QString val2  = e2.attribute(QString::fromLatin1("value"));

                if (name2 == QString::fromLatin1("path"))
                {
                    urls << QUrl::fromLocalFile(val2);
                }
            }

            map.insert(album, urls);
        }

        setCollectionMap(map);

        return true;
    }
    else
    {
        return false;
    }
}

} // namespace Digikam
