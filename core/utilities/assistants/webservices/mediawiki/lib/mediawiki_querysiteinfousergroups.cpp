/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-03-22
 * Description : a MediaWiki C++ interface
 *
 * Copyright (C) 2011-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2011      by Ludovic Delfau <ludovicdelfau at gmail dot com>
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

#include "mediawiki_querysiteinfousergroups.h"

// Qt includes

#include <QTimer>
#include <QUrl>
#include <QUrlQuery>
#include <QXmlStreamReader>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

// Local includes

#include "mediawiki_mediawiki.h"
#include "mediawiki_job_p.h"

namespace mediawiki
{

class QuerySiteinfoUsergroupsPrivate : public JobPrivate
{
public:

    QuerySiteinfoUsergroupsPrivate(MediaWiki& mediawiki, QNetworkAccessManager* const manager, bool includeNumber)
            : JobPrivate(mediawiki),
              manager(manager),
              includeNumber(includeNumber)
    {
    }

    QNetworkAccessManager* const manager;
    bool                         includeNumber;
};

QuerySiteinfoUsergroups::QuerySiteinfoUsergroups(MediaWiki& mediawiki, QObject* const parent)
    : Job(*new QuerySiteinfoUsergroupsPrivate(mediawiki, new QNetworkAccessManager(), false), parent)
{
}

QuerySiteinfoUsergroups::~QuerySiteinfoUsergroups()
{
}

void QuerySiteinfoUsergroups::setIncludeNumber(bool includeNumber)
{
    Q_D(QuerySiteinfoUsergroups);
    d->includeNumber = includeNumber;
}

void QuerySiteinfoUsergroups::start()
{
    QTimer::singleShot(0, this, SLOT(doWorkSendRequest()));
}

void QuerySiteinfoUsergroups::doWorkSendRequest()
{
    Q_D(QuerySiteinfoUsergroups);

    // Set the url
    QUrl url = d->mediawiki.url();
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("format"), QStringLiteral("xml"));
    query.addQueryItem(QStringLiteral("action"), QStringLiteral("query"));
    query.addQueryItem(QStringLiteral("meta"),   QStringLiteral("siteinfo"));
    query.addQueryItem(QStringLiteral("siprop"), QStringLiteral("usergroups"));
    if (d->includeNumber)
    {
        query.addQueryItem(QStringLiteral("sinumberingroup"), QString());
    }
    url.setQuery(query);

    // Set the request
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", d->mediawiki.userAgent().toUtf8());

    // Send the request
    d->reply = d->manager->get(request);
    connectReply();

    connect(d->reply, SIGNAL(finished()),
            this, SLOT(doWorkProcessReply()));
}

void QuerySiteinfoUsergroups::doWorkProcessReply()
{
    Q_D(QuerySiteinfoUsergroups);

    disconnect(d->reply, SIGNAL(finished()),
               this, SLOT(doWorkProcessReply()));

    if (d->reply->error() == QNetworkReply::NoError)
    {
        QList<UserGroup> results;
        QString          name;
        QList<QString>   rights;
        unsigned int     number = 0;
        QXmlStreamReader reader(d->reply);

        while (!reader.atEnd() && !reader.hasError())
        {
            QXmlStreamReader::TokenType token = reader.readNext();

            if (token == QXmlStreamReader::StartElement)
            {
                if (reader.name() == QLatin1String("group"))
                {
                    name = reader.attributes().value(QStringLiteral("name")).toString();
                    if (d->includeNumber)
                    {
                        number = reader.attributes().value(QStringLiteral("number")).toString().toUInt();
                    }
                }
                else if (reader.name() == QLatin1String("rights"))
                {
                    rights.clear();
                }
                else if (reader.name() == QLatin1String("permission"))
                {
                    reader.readNext();
                    rights.push_back(reader.text().toString());
                }
            }
            else if (token == QXmlStreamReader::EndElement)
            {
                if (reader.name() == QLatin1String("group"))
                {
                    UserGroup usergroup;
                    usergroup.setName(name);
                    usergroup.setRights(rights);

                    if(d->includeNumber)
                    {
                        usergroup.setNumber(number);
                    }

                    results.push_back(usergroup);
                }
            }
        }

        if (!reader.hasError())
        {
            setError(Job::NoError);
            emit usergroups(results);
        }
        else
        {
            setError(QuerySiteinfoUsergroups::XmlError);
        }
    }
    else
    {
        setError(QuerySiteinfoUsergroups::NetworkError);
    }

    emitResult();
}

} // namespace mediawiki
