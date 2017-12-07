/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2014-10-17
 * Description : test for implementation of dimagelist api
 *
 * Copyright (C) 2011-2012 by A Janardhan Reddy <annapareddyjanardhanreddy at gmail dot com>
 * Copyright (C) 2011-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIMAGES_LIST_TEST_H
#define DIMAGES_LIST_TEST_H

// Qt includes

#include <QUrl>
#include <QDialog>

// Local includes

#include "actionthreadbase.h"

using namespace Digikam;

class ActionThread : public ActionThreadBase
{
    Q_OBJECT

public:

    explicit ActionThread(QObject* const parent);
    ~ActionThread();

    void rotate(const QList<QUrl>& list);

Q_SIGNALS:

    void starting(const QUrl& url);
    void finished(const QUrl& url);
    void failed(const QUrl& url, const QString& err);
    void progress(const QUrl& url, int percent);

private Q_SLOTS:

    void slotJobDone();
    void slotJobProgress(int);
    void slotJobStarted();
};

// -----------------------------------------------------

class DImagesListTest : public QDialog
{
    Q_OBJECT

public:

    explicit DImagesListTest(QObject* const parent);
    ~DImagesListTest();

private Q_SLOTS:

    void slotStart();
    void slotStarting(const QUrl&);
    void slotFinished(const QUrl&);
    void slotFailed(const QUrl&, const QString&);

private:

    class Private;
    Private* const d;
};

#endif // DIMAGES_LIST_TEST_H
