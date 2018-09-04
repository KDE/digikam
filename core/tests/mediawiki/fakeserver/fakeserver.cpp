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

#include "fakeserver.h"

// C++ includes

#include <iostream>

// Qt includes

#include <QFile>

FakeServer::FakeServer(QObject* const parent)
    :  QThread(parent)
{
    m_clientSocket = 0;
    m_tcpServer    = 0;

    moveToThread(this);
}

FakeServer::~FakeServer()
{
    quit();
    wait();
}

void FakeServer::startAndWait()
{
    start();
    // this will block until the event queue starts
    QMetaObject::invokeMethod(this, "started", Qt::BlockingQueuedConnection);
}

void FakeServer::newConnection()
{
    QMutexLocker locker(&m_mutex);
    m_clientSocket = m_tcpServer->nextPendingConnection();

    connect(m_clientSocket, SIGNAL(readyRead()),
            this, SLOT(dataAvailable()));
}

void FakeServer::dataAvailable()
{
    QMutexLocker locker(&m_mutex);

    if (m_clientSocket->canReadLine())
    {
        QStringList token = QString::fromUtf8(m_clientSocket->readAll()).split(QRegExp(QStringLiteral("[ \r\n][ \r\n]*")));

        if (!token.empty())
        {
            FakeServer::Request request;
            request.type  = token[0];
            request.agent = token[4];
            request.value = token[1];

            // It might happen that the same request cames through more than once, so you need to check that you are
            // counting each different request only once.
            //
            // For more information, see: http://qt-project.org/forums/viewthread/25521
            //
            if (!m_request.contains(request))
            {
                m_request << request;

                QString retour   = m_scenarios.isEmpty() ? QStringLiteral("empty") : m_scenarios.takeFirst();
                QString cookie   = m_cookie.isEmpty()    ? QStringLiteral("empty") : m_cookie.takeFirst();
                QString scenario = QStringLiteral("HTTP/1.0 200 Ok\r\nContent-Type: text/html; charset=\"utf-8\"\r\nSet-Cookie:") + cookie + QStringLiteral("\r\n\r\n") + retour;
                m_clientSocket->write(scenario.toLocal8Bit());
            }
        }
    }

    m_clientSocket->close();
}

void FakeServer::run()
{
    m_tcpServer = new QTcpServer();

    if (!m_tcpServer->listen(QHostAddress(QHostAddress::LocalHost), 12566))
    {
        // TODO
    }

    connect(m_tcpServer, SIGNAL(newConnection()),
            this, SLOT(newConnection()));

    exec();

    delete m_clientSocket;
    delete m_tcpServer;
}

void FakeServer::started()
{
  // do nothing: this is a dummy slot used by startAndWait()
}

void FakeServer::setScenario(const QString& scenario, const QString& cookie)
{
    QMutexLocker locker(&m_mutex);

    m_scenarios.clear();
    m_scenarios << scenario;
    m_cookie.clear();
    m_cookie    << cookie;
    m_request.clear();
}

void FakeServer::addScenario(const QString& scenario, const QString& cookie )
{
    QMutexLocker locker(&m_mutex);

    m_scenarios << scenario;
    m_cookie    << cookie;
}

void FakeServer::addScenarioFromFile(const QString& fileName, const QString& cookie )
{
    QFile file(fileName);

    if (!file.open(QFile::ReadOnly))
    {
        return;
    }

    QTextStream in(&file);
    QString scenario;

    // When loading from files we never have the authentication phase
    // force jumping directly to authenticated state.

    while (!in.atEnd())
    {
        scenario.append(in.readLine());
    }

    file.close();

    addScenario(scenario, cookie);
}

bool FakeServer::isScenarioDone( int scenarioNumber ) const
{
    QMutexLocker locker(&m_mutex);

    if (scenarioNumber < m_scenarios.size())
    {
        return m_scenarios[scenarioNumber].isEmpty();
    }
    else
    {
        return true; // Non existent hence empty, right?
    }
}

bool FakeServer::isAllScenarioDone() const
{
    QMutexLocker locker(&m_mutex);

    foreach (const QString& scenario, m_scenarios)
    {
        if (!scenario.isEmpty())
        {
            return false;
        }
    }

    return true;
}

const QList<FakeServer::Request>& FakeServer::getRequest()
{
    return m_request;
}

FakeServer::Request FakeServer::takeLastRequest()
{
    return m_request.takeLast();
}

FakeServer::Request FakeServer::takeFirstRequest()
{
    return m_request.takeFirst();
}

void FakeServer::clearRequest()
{
    return m_request.clear();
}
