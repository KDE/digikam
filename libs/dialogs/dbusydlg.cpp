/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-03
 * Description : a busy dialog for digiKam
 *
 * Copyright (C) 2009-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "dbusydlg.moc"

// Qt includes

#include <QPushButton>
#include <QProgressBar>

// KDE includes

#include <kdebug.h>

namespace Digikam
{

DBusyThread::DBusyThread(QObject* const parent)
    : QThread(parent)
{
}

DBusyThread::~DBusyThread()
{
    wait();
}

// ----------------------------------------------------------------------------------

class DBusyDlg::Private
{
public:

    Private() :
        thread(0)
    {
    }

    DBusyThread* thread;
};

DBusyDlg::DBusyDlg(const QString& txt, QWidget* const parent)
    : KProgressDialog(parent, QString(), txt, Qt::FramelessWindowHint),
      d(new Private)
{
    setAllowCancel(false);
    setMinimumDuration(0);
    setModal(true);
    setAutoClose(false);

    progressBar()->setMaximum(0);
    progressBar()->setMinimum(0);
    progressBar()->setValue(0);
}

DBusyDlg::~DBusyDlg()
{
    delete d;
}

void DBusyDlg::setBusyThread(DBusyThread* const thread)
{
    d->thread = thread;

    if (d->thread)
    {
        connect(d->thread, SIGNAL(signalComplete()),
                this, SLOT(slotComplete()));

        kDebug() << "Thread is started";
        d->thread->start();
    }
}

void DBusyDlg::slotComplete()
{
    kDebug() << "Thread is complete";
    accept();
}

}  // namespace Digikam
