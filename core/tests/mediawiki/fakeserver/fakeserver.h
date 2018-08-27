/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-03-22
 * Description : a MediaWiki C++ interface
 *
 * Copyright (C) 2011-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2011      by Alexandre Mendes <alex dot mendes1988 at gmail dot com>
 * Copyright (C) 2011      by Hormiere Guillaume <hormiere dot guillaume at gmail dot com>
 * Copyright (C) 2011      by Manuel Campomanes <campomanes dot manuel at gmail dot com>
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

#ifndef MEDIAWIKI_FAKE_SERVER_H
#define MEDIAWIKI_FAKE_SERVER_H

// Qt includes

#include <QList>
#include <QMutex>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QThread>
#include <QTcpServer>
#include <QTcpSocket>

class FakeServer : QThread
{
    Q_OBJECT

public:

    struct Request
    {
        Request()
        {
        }

        Request(const QString& t, const QString& a, const QString& v)
        {
            type  = t;
            agent = a;
            value = v;
        }

        QString type;
        QString agent;
        QString value;

        bool operator==(const FakeServer::Request &other) const
        {
            return this->type  == other.type  &&
                   this->agent == other.agent &&
                   this->value == other.value;
        }
    };

    FakeServer(QObject* const parent = 0);
    ~FakeServer();

    void startAndWait();
    void run() Q_DECL_OVERRIDE;

    void setScenario( const QString& scenario, const QString& cookie = QStringLiteral("empty"));
    void addScenario( const QString& scenario, const QString& cookie = QStringLiteral("empty"));
    void addScenarioFromFile( const QString& fileName, const QString& cookie = QStringLiteral("empty"));

    bool isScenarioDone( int scenarioNumber ) const;
    bool isAllScenarioDone() const;

    const QList<FakeServer::Request>& getRequest()
    {
        return m_request;
    }

    FakeServer::Request takeLastRequest()
    {
        return m_request.takeLast();
    }

    FakeServer::Request takeFirstRequest()
    {
        return m_request.takeFirst();
    }

    void clearRequest()
    {
        return m_request.clear();
    }

private Q_SLOTS:

    void newConnection();
    void dataAvailable();
    void started();

private:

    QStringList                m_scenarios;
    QStringList                m_cookie;
    QList<FakeServer::Request> m_request;
    QTcpServer*                m_tcpServer;
    mutable QMutex             m_mutex;
    QTcpSocket*                m_clientSocket;
};

#endif // MEDIAWIKI_FAKE_SERVER_H
