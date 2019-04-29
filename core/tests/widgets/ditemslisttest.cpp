/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2014-10-17
 * Description : test for implementation of ditemslist api
 *
 * Copyright (C) 2011-2012 by A Janardhan Reddy <annapareddyjanardhanreddy at gmail dot com>
 * Copyright (C) 2011-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "ditemslisttest.h"

// Qt includes

#include <QGridLayout>
#include <QProgressBar>
#include <QDebug>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QApplication>
#include <QImage>
#include <QTransform>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "ditemslist.h"

using namespace Digikam;

class Q_DECL_HIDDEN Task : public ActionJob
{
public:

    Task()
        : ActionJob()
    {
    }

    QString errString;
    QUrl    fileUrl;

protected:

    void run()
    {
        emit signalStarted();

        QImage src;
        QImage dst;

        if (m_cancel)
            return;

        emit signalProgress(20);

        if (!src.load(fileUrl.toLocalFile()))
        {
            errString = QLatin1String("Loading image failed. Aborted...");
            return;
        }

        if (m_cancel)
            return;

        emit signalProgress(40);

        QTransform transform;
        transform.rotate(90);

        if (m_cancel)
            return;

        emit signalProgress(60);

        dst = src.transformed(transform);

        if (m_cancel)
            return;

        emit signalProgress(80);

        if (!dst.save(fileUrl.toLocalFile()))
        {
            errString = QLatin1String("Saving image failed. Aborted...");
            return;
        }

        emit signalDone();
    }
};

// ----------------------------------------------------------------------------------------------------

ActionThread::ActionThread(QObject* const parent)
    : ActionThreadBase(parent)
{
    setObjectName(QLatin1String("ActionThread"));
}

ActionThread::~ActionThread()
{
}

void ActionThread::rotate(const QList<QUrl>& list)
{
    ActionJobCollection collection;

    foreach (const QUrl& url, list)
    {
        Task* const job = new Task();
        job->fileUrl    = url;

        connect(job, &Task::signalStarted,
                this, &ActionThread::slotJobStarted);

        connect(job, &Task::signalProgress,
                this, &ActionThread::slotJobProgress);

        connect(job, &Task::signalDone,
                this, &ActionThread::slotJobDone);

        collection.insert(job, 0);

        qDebug() << "Appending file to process " << url;
    }

    appendJobs(collection);
}

void ActionThread::slotJobDone()
{
    Task* const task = dynamic_cast<Task*>(sender());
    if (!task) return;

    if (task->errString.isEmpty())
    {
        emit finished(task->fileUrl);
    }
    else
    {
        emit failed(task->fileUrl, task->errString);
    }
}

void ActionThread::slotJobProgress(int p)
{
    Task* const task = dynamic_cast<Task*>(sender());
    if (!task) return;

    emit progress(task->fileUrl, p);
}

void ActionThread::slotJobStarted()
{
    Task* const task = dynamic_cast<Task*>(sender());
    if (!task) return;

    emit starting(task->fileUrl);
}

// ----------------------------------------------------------

class Q_DECL_HIDDEN DItemsListTest::Private
{
public:

    explicit Private()
    {
        page        = nullptr;
        buttons     = nullptr;
        progressBar = nullptr;
        applyBtn    = nullptr;
        listView    = nullptr;
        thread      = nullptr;
    }

    QWidget*          page;

    QDialogButtonBox* buttons;
    QProgressBar*     progressBar;
    QPushButton*      applyBtn;

    DItemsList*      listView;

    ActionThread*     thread;
};

DItemsListTest::DItemsListTest(QObject* const /*parent*/)
    : QDialog(),
      d(new Private)
{
    setWindowTitle(QString::fromUtf8("Rotate Images to 180 Degrees"));

    d->buttons               = new QDialogButtonBox(QDialogButtonBox::Apply |
                                                    QDialogButtonBox::Close, this);

    d->page                  = new QWidget(this);
    QVBoxLayout* const vbx   = new QVBoxLayout(this);
    vbx->addWidget(d->page);
    vbx->addWidget(d->buttons);
    setLayout(vbx);
    setModal(false);

    // -------------------

    QGridLayout* const mainLayout = new QGridLayout(d->page);

    d->listView                   = new DItemsList(d->page);
    d->listView->setControlButtonsPlacement(DItemsList::ControlButtonsRight);

    d->progressBar                = new QProgressBar(d->page);
    d->progressBar->setMaximumHeight(fontMetrics().height()+2);

    mainLayout->addWidget(d->listView,    0, 0, 1, 1);
    mainLayout->addWidget(d->progressBar, 1, 0, 1, 1);
    mainLayout->setRowStretch(0, 10);

    d->thread = new ActionThread(this);

    d->applyBtn = d->buttons->button(QDialogButtonBox::Apply);
    d->applyBtn->setText(i18n("Rotate Items"));

    connect(d->applyBtn, &QPushButton::clicked,
            this, &DItemsListTest::slotStart);

    connect(d->buttons->button(QDialogButtonBox::Close), &QPushButton::clicked,
            this, &DItemsListTest::close);

    connect(d->thread, &ActionThread::starting,
            this, &DItemsListTest::slotStarting);

    connect(d->thread, &ActionThread::finished,
            this, &DItemsListTest::slotFinished);

    connect(d->thread, &ActionThread::failed,
            this, &DItemsListTest::slotFailed);
}

DItemsListTest::~DItemsListTest()
{
    delete d;
}

void DItemsListTest::slotStart()
{
    QList<QUrl> selectedImages = d->listView->imageUrls();

    if (selectedImages.isEmpty())
        return;

    qDebug() << selectedImages;
    d->progressBar->setMaximum(selectedImages.count());
    d->progressBar->setValue(0);
    d->applyBtn->setEnabled(false);

    // Rotate the selected images by 180 degrees
    // It can be converted to gray scale also, just change the function here
    d->thread->rotate(selectedImages);
    d->thread->start();
}

void DItemsListTest::slotStarting(const QUrl& url)
{
    d->listView->processing(url);
}

void DItemsListTest::slotFinished(const QUrl& url)
{
    d->listView->processed(url, true);
    d->progressBar->setValue(d->progressBar->value()+1);
    d->listView->updateThumbnail(url);
}

void DItemsListTest::slotFailed(const QUrl& url, const QString&)
{
    d->listView->processed(url, false);
    d->progressBar->setValue(d->progressBar->value()+1);
}

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    DItemsListTest* const view = new DItemsListTest(&app);
    view->show();
    app.exec();
    return 0;
}
